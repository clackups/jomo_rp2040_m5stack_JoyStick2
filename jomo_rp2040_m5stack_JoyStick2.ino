#include <Arduino.h>
#include <Wire.h>
#include <Mouse.h>

#define JOYSTICK2_ADDR                        0x63
#define JOYSTICK2_ADC_VALUE_12BITS_REG        0x00
#define JOYSTICK2_ADC_VALUE_8BITS_REG         0x10
#define JOYSTICK2_BUTTON_REG                  0x20
#define JOYSTICK2_RGB_REG                     0x30
#define JOYSTICK2_ADC_VALUE_CAL_REG           0x40
#define JOYSTICK2_OFFSET_ADC_VALUE_12BITS_REG 0x50
#define JOYSTICK2_OFFSET_ADC_VALUE_8BITS_REG  0x60
#define JOYSTICK2_FIRMWARE_VERSION_REG        0xFE
#define JOYSTICK2_BOOTLOADER_VERSION_REG      0xFC
#define JOYSTICK2_I2C_ADDRESS_REG             0xFF


void read_bytes(uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t length)
{
    uint8_t index = 0;
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, length);
    for (int i = 0; i < length; i++) {
        buffer[index++] = Wire.read();
    }
}

void get_joy_adc_16bits_value_xy(uint16_t *adc_x, uint16_t *adc_y)
{
    uint8_t data[4];
    uint8_t reg = JOYSTICK2_ADC_VALUE_12BITS_REG;
    read_bytes(JOYSTICK2_ADDR, reg, data, 4);
    memcpy((uint8_t *)adc_x, &data[0], 2);
    memcpy((uint8_t *)adc_y, &data[2], 2);
}


// X, Y - Low, High
static uint16_t calibrated_data[2][2];

void setup()
{
    Serial.begin(115200);
    Wire.setSDA(0);
    Wire.setSCL(1);
    Wire.begin();

    uint16_t val[2];
    get_joy_adc_16bits_value_xy(&val[0], &val[1]);
    for(int i=0; i<2; i++) {
        calibrated_data[i][0] = val[i];
        calibrated_data[i][1] = val[i];
    }

    for(int n=0; n < 20; n++) {
        get_joy_adc_16bits_value_xy(&val[0], &val[1]);
        for(int i=0; i<2; i++) {
            if( val[i] < calibrated_data[i][0] ) {
                calibrated_data[i][0] = val[i];
            }
            if( val[i] < calibrated_data[i][1] ) {
                calibrated_data[i][1] = val[i];
            }
        }
        delay(10);
    }

    for(int i=0; i<2; i++) {
        uint16_t margin = 1000;
        calibrated_data[i][0] -= margin;
        calibrated_data[i][1] += margin;
    }

    Mouse.begin();
}

void read_joystick(int *val)
{
    uint16_t adc[2];
    get_joy_adc_16bits_value_xy(&adc[0], &adc[1]);
    for(int i=0; i<2; i++) {
        if( adc[i] < calibrated_data[i][0] ) {
            val[i] = (int)adc[i] - calibrated_data[i][0];
        }
        else if ( adc[i] > calibrated_data[i][1] ) {
            val[i] = (int)adc[i] - calibrated_data[i][1];
        }
        else {
            val[i] = 0;
        }
    }
}


void loop()
{
    int val[2];
    read_joystick(val);
    // Serial.printf("x: %d, y:%d\r\n", val[0], val[1]);
    Mouse.move(val[0]/-500, val[1]/-500, 0);
    delay(30);
}
