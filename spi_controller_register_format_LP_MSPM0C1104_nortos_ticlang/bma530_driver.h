/*
 * BMA530 Accelerometer Driver Header
 * Bosch Sensortec BMA530 3-Axis Accelerometer
 */ 

#define BMA530_REG_ACC_DATA_0       0x18
#define BMA530_REG_SENSOR_STATUS    0x11
#define BMA530_REG_CHIP_ID          0x00

#define DEAFULT_DELAY_CYCLES        100000

typedef struct {
    float x;
    float y;
    float z;
} BMA530_AccelXYZ;


typedef enum {
    RANGE_2G,
    RANGE_4G,
    RANGE_8G,
    RANGE_16,
} BMA530_Range;


void BMA530_ReadAccel(BMA530_AccelXYZ *acc, BMA530_Range range);


void BMA530_init();