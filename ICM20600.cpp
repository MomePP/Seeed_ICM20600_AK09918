#include "ICM20600.h"

ICM20600::ICM20600(bool AD0)
    : I2CDevice(AD0 ? ICM20600_I2C_ADDR2 : ICM20600_I2C_ADDR1) {}

uint8_t ICM20600::getDeviceID()
{
    return read8(ICM20600_WHO_AM_I);
}

void ICM20600::initialize()
{
    // configuration
    write_register(ICM20600_CONFIG, 0x00);
    // disable fifo
    write_register(ICM20600_FIFO_EN, 0x00);

    // set default power mode
    ICM20600::setPowerMode(ICM_6AXIS_LOW_POWER);

    // gyro config
    ICM20600::setGyroScaleRange(RANGE_2K_DPS);
    ICM20600::setGyroOutputDataRate(GYRO_RATE_1K_BW_176);
    ICM20600::setGyroAverageSample(GYRO_AVERAGE_1);

    // accel config
    ICM20600::setAccScaleRange(RANGE_16G);
    ICM20600::setAccOutputDataRate(ACC_RATE_1K_BW_420);
    ICM20600::setAccAverageSample(ACC_AVERAGE_4);
}

void ICM20600::setPowerMode(icm20600_power_type_t mode)
{
    uint8_t data_pwr1;
    uint8_t data_pwr2 = 0x00;
    uint8_t data_gyro_lp;
    data_pwr1 = read8(ICM20600_PWR_MGMT_1);
    data_pwr1 &= 0x8f; // 0b10001111
    data_gyro_lp = read8(ICM20600_GYRO_LP_MODE_CFG);
    // When set to ‘1’ low-power gyroscope mode is enabled. Default setting is 0
    data_gyro_lp &= 0x7f; // 0b01111111
    switch (mode)
    {
    case ICM_SLEEP_MODE:
        data_pwr1 |= 0x40; // set 0b01000000
        break;

    case ICM_STANDYBY_MODE:
        data_pwr1 |= 0x10; // set 0b00010000
        data_pwr2 = 0x38;  // 0x00111000 disable acc
        break;

    case ICM_ACC_LOW_POWER:
        data_pwr1 |= 0x20; // set bit5 0b00100000
        data_pwr2 = 0x07;  // 0x00000111 disable gyro
        break;

    case ICM_ACC_LOW_NOISE:
        data_pwr1 |= 0x00;
        data_pwr2 = 0x07; // 0x00000111 disable gyro
        break;

    case ICM_GYRO_LOW_POWER:
        data_pwr1 |= 0x00; // dont set bit5 0b00000000
        data_pwr2 = 0x38;  // 0x00111000 disable acc
        data_gyro_lp |= 0x80;
        break;

    case ICM_GYRO_LOW_NOISE:
        data_pwr1 |= 0x00;
        data_pwr2 = 0x38; // 0x00111000 disable acc
        break;

    case ICM_6AXIS_LOW_POWER:
        data_pwr1 |= 0x00; // dont set bit5 0b00100000
        data_gyro_lp |= 0x80;
        break;

    case ICM_6AXIS_LOW_NOISE:
        data_pwr1 |= 0x00;
        break;

    default:
        break;
    }
    write_register(ICM20600_PWR_MGMT_1, data_pwr1);
    write_register(ICM20600_PWR_MGMT_2, data_pwr2);
    write_register(ICM20600_GYRO_LP_MODE_CFG, data_gyro_lp);
}

// SAMPLE_RATE = 1KHz / (1 + div)
// work for low-power gyroscope and low-power accelerometer and low-noise accelerometer
void ICM20600::setSampleRateDivier(uint8_t div)
{
    write_register(ICM20600_SMPLRT_DIV, div);
}

void ICM20600::setAccScaleRange(acc_scale_type_t range)
{
    uint8_t data;
    data = read8(ICM20600_ACCEL_CONFIG);
    data &= 0xe7; // 0b 1110 0111

    switch (range)
    {
    case RANGE_2G:
        data |= 0x00; // 0bxxx00xxx
        _acc_scale = 4000;
        break;

    case RANGE_4G:
        data |= 0x08; // 0bxxx01xxx
        _acc_scale = 8000;
        break;

    case RANGE_8G:
        data |= 0x10; // 0bxxx10xxx
        _acc_scale = 16000;
        break;

    case RANGE_16G:
        data |= 0x18; // 0bxxx11xxx
        _acc_scale = 32000;
        break;

    default:
        break;
    }
    write_register(ICM20600_ACCEL_CONFIG, data);
}

// for low power mode only
void ICM20600::setAccAverageSample(acc_averaging_sample_type_t sample)
{
    uint8_t data = 0;
    data = read8(ICM20600_ACCEL_CONFIG2);

    data &= 0xcf; // & 0b11001111
    switch (sample)
    {
    case ACC_AVERAGE_4:
        data |= 0x00; // 0bxx00xxxx
        break;

    case ACC_AVERAGE_8:
        data |= 0x10; // 0bxx01xxxx
        break;

    case ACC_AVERAGE_16:
        data |= 0x20; // 0bxx10xxxx
        break;

    case ACC_AVERAGE_32:
        data |= 0x30; // 0bxx11xxxx
        break;

    default:
        break;
    }
    write_register(ICM20600_ACCEL_CONFIG2, data);
}

void ICM20600::setAccOutputDataRate(acc_lownoise_odr_type_t odr)
{
    uint8_t data;
    data = read8(ICM20600_ACCEL_CONFIG2);
    data &= 0xf0; // 0b11110000

    switch (odr)
    {
    case ACC_RATE_4K_BW_1046:
        data |= 0x08;
        break;

    case ACC_RATE_1K_BW_420:
        data |= 0x07;
        break;

    case ACC_RATE_1K_BW_218:
        data |= 0x01;
        break;

    case ACC_RATE_1K_BW_99:
        data |= 0x02;
        break;

    case ACC_RATE_1K_BW_44:
        data |= 0x03;
        break;

    case ACC_RATE_1K_BW_21:
        data |= 0x04;
        break;

    case ACC_RATE_1K_BW_10:
        data |= 0x05;
        break;

    case ACC_RATE_1K_BW_5:
        data |= 0x06;
        break;

    default:
        break;
    }
    write_register(ICM20600_ACCEL_CONFIG2, data);
}

void ICM20600::setGyroScaleRange(gyro_scale_type_t range)
{
    uint8_t data = 0;
    data = read8(ICM20600_GYRO_CONFIG);
    data &= 0xe7; // 0b11100111

    switch (range)
    {
    case RANGE_250_DPS:
        data |= 0x00; // 0bxxx00xxx
        _gyro_scale = 500;
        break;

    case RANGE_500_DPS:
        data |= 0x08; // 0bxxx00xxx
        _gyro_scale = 1000;
        break;

    case RANGE_1K_DPS:
        data |= 0x10; // 0bxxx10xxx
        _gyro_scale = 2000;
        break;

    case RANGE_2K_DPS:
        data |= 0x18; // 0bxxx11xxx
        _gyro_scale = 4000;
        break;

    default:
        break;
    }
    write_register(ICM20600_GYRO_CONFIG, data);
}

// for low power mode only
void ICM20600::setGyroAverageSample(gyro_averaging_sample_type_t sample)
{
    uint8_t data = 0;
    data = read8(ICM20600_GYRO_LP_MODE_CFG);

    data &= 0x8f; // 0b10001111
    switch (sample)
    {
    case GYRO_AVERAGE_1:
        data |= 0x00; // 0bx000xxxx
        break;

    case GYRO_AVERAGE_2:
        data |= 0x10; // 0bx001xxxx
        break;

    case GYRO_AVERAGE_4:
        data |= 0x20; // 0bx010xxxx
        break;

    case GYRO_AVERAGE_8:
        data |= 0x30; // 0bx011xxxx
        break;

    case GYRO_AVERAGE_16:
        data |= 0x40; // 0bx100xxxx
        break;

    case GYRO_AVERAGE_32:
        data |= 0x50; // 0bx101xxxx
        break;

    case GYRO_AVERAGE_64:
        data |= 0x60;
        break;

    case GYRO_AVERAGE_128:
        data |= 0x70;
        break;

    default:
        break;
    }
    write_register(ICM20600_GYRO_LP_MODE_CFG, data);
}

void ICM20600::setGyroOutputDataRate(gyro_lownoise_odr_type_t odr)
{
    uint8_t data;
    data = read8(ICM20600_CONFIG);
    data &= 0xf8; // DLPF_CFG[2:0] 0b11111000

    switch (odr)
    {
    case GYRO_RATE_8K_BW_3281:
        data |= 0x07;
        break;
    case GYRO_RATE_8K_BW_250:
        data |= 0x00;
        break;
    case GYRO_RATE_1K_BW_176:
        data |= 0x01;
        break;
    case GYRO_RATE_1K_BW_92:
        data |= 0x02;
        break;
    case GYRO_RATE_1K_BW_41:
        data |= 0x03;
        break;
    case GYRO_RATE_1K_BW_20:
        data |= 0x04;
        break;
    case GYRO_RATE_1K_BW_10:
        data |= 0x05;
        break;
    case GYRO_RATE_1K_BW_5:
        data |= 0x06;
        break;
    }
    write_register(ICM20600_CONFIG, data);
}

void ICM20600::getAcceleration(int16_t *x, int16_t *y, int16_t *z)
{
    *x = ICM20600::getAccelerationX();
    *y = ICM20600::getAccelerationY();
    *z = ICM20600::getAccelerationZ();
}

int16_t ICM20600::getAccelerationX(void)
{
    int32_t raw_data = ICM20600::getRawAccelerationX();
    raw_data = (raw_data * _acc_scale) >> 16;
    return (int16_t)raw_data;
}
int16_t ICM20600::getAccelerationY(void)
{
    int32_t raw_data = ICM20600::getRawAccelerationY();
    raw_data = (raw_data * _acc_scale) >> 16;
    return (int16_t)raw_data;
}
int16_t ICM20600::getAccelerationZ(void)
{
    int32_t raw_data = ICM20600::getRawAccelerationZ();
    raw_data = (raw_data * _acc_scale) >> 16;
    return (int16_t)raw_data;
}

int16_t ICM20600::getRawAccelerationX(void)
{
    return read16(ICM20600_ACCEL_XOUT_H);
}

int16_t ICM20600::getRawAccelerationY(void)
{
    return read16(ICM20600_ACCEL_YOUT_H);
}

int16_t ICM20600::getRawAccelerationZ(void)
{
    return read16(ICM20600_ACCEL_ZOUT_H);
}

void ICM20600::getGyroscope(int16_t *x, int16_t *y, int16_t *z)
{
    *x = ICM20600::getGyroscopeX();
    *y = ICM20600::getGyroscopeY();
    *z = ICM20600::getGyroscopeZ();
}

int16_t ICM20600::getGyroscopeX(void)
{
    int32_t raw_data = ICM20600::getRawGyroscopeX();
    raw_data = (raw_data * _gyro_scale) >> 16;
    return (int16_t)raw_data;
}

int16_t ICM20600::getGyroscopeY(void)
{
    int32_t raw_data = ICM20600::getRawGyroscopeY();
    raw_data = (raw_data * _gyro_scale) >> 16;
    return (int16_t)raw_data;
}

int16_t ICM20600::getGyroscopeZ(void)
{
    int32_t raw_data = ICM20600::getRawGyroscopeZ();
    raw_data = (raw_data * _gyro_scale) >> 16;
    return (int16_t)raw_data;
}

int16_t ICM20600::getRawGyroscopeX(void)
{
    return read16(ICM20600_GYRO_XOUT_H);
}

int16_t ICM20600::getRawGyroscopeY(void)
{
    return read16(ICM20600_GYRO_YOUT_H);
}

int16_t ICM20600::getRawGyroscopeZ(void)
{
    return read16(ICM20600_GYRO_ZOUT_H);
}

int16_t ICM20600::getTemperature(void)
{
    uint16_t rawdata = read16(ICM20600_TEMP_OUT_H);
    return (int16_t)(rawdata / 327 + 25);
}

void ICM20600::reset()
{
    uint8_t data = read8(ICM20600_USER_CTRL);
    data &= 0xfe; // ICM20600_USER_CTRL[0] 0b11111110
    data |= ICM20600_RESET_BIT;
    write_register(ICM20600_USER_CTRL, data);
}
