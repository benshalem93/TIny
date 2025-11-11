#include "ti_msp_dl_config.h"
#include "bma530_driver.h"


/* Maximum buffer size defined for this example */
#define MAX_BUFFER_SIZE (20)

/* Dummy data sent when receiving data from SPI Peripheral */
#define DUMMY_DATA (0x00)


typedef enum SPI_ControllerModeEnum {
    IDLE_MODE,
    TX_REG_ADDRESS_WRITE_MODE,
    TX_REG_ADDRESS_READ_MODE,
    WRITE_DATA_MODE,
    READ_DATA_MODE,
    TIMEOUT_MODE
} SPI_Controller_Mode;



/* Buffer used to receive data in the ISR */
uint8_t gRxBuffer[MAX_BUFFER_SIZE] = {0};
/* Buffer used to transmit data in the ISR */
uint8_t gTxBuffer[MAX_BUFFER_SIZE] = {0};

/* Used to track the state of the software state machine */
volatile SPI_Controller_Mode gControllerMode = IDLE_MODE;
/* Number of bytes left to receive */
volatile uint8_t gRxByteCount = 0;
/* Index of the next byte to be received in gRxBuffer */
volatile uint8_t gRxIndex = 0;
/* Number of bytes left to transfer */
volatile uint8_t gTxByteCount = 0;
/* Index of the next byte to be transmitted in gTxBuffer */
volatile uint8_t gTxIndex = 0;


/*
 * Copies an array from source to destination
 *
 *  source   Pointer to source array
 *  dest     Pointer to destination array
 *  count    Number of bytes to copy
 *
 */
static void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count)
{
    uint8_t copyIndex = 0;
    for (copyIndex = 0; copyIndex < count; copyIndex++) {
        dest[copyIndex] = source[copyIndex];
    }
}


void SPI_0_INST_IRQHandler(void)
{
    switch (DL_SPI_getPendingInterrupt(SPI_0_INST)) {
        case DL_SPI_IIDX_TX:
            switch (gControllerMode) {
                case IDLE_MODE:
                case TIMEOUT_MODE:
                    break;
                case TX_REG_ADDRESS_READ_MODE:
                case READ_DATA_MODE:
                    if (gTxByteCount) {
                        /*  Send dummy data to get read more bytes */
                        DL_SPI_transmitData8(SPI_0_INST, DUMMY_DATA);
                        gTxByteCount--;
                    }
                    break;
                case TX_REG_ADDRESS_WRITE_MODE:
                    gControllerMode = WRITE_DATA_MODE;
                case WRITE_DATA_MODE:
                    if (gTxByteCount) {
                        /* Transmit data until all expected data is sent */
                        DL_SPI_transmitData8(
                            SPI_0_INST, gTxBuffer[gTxIndex++]);
                        gTxByteCount--;
                    } else {
                        /* Transmission is done, reset state machine */
                        gControllerMode = IDLE_MODE;
                    }
                    break;
            }
            break;
        case DL_SPI_IIDX_RX:
            switch (gControllerMode) {
                case IDLE_MODE:
                case TIMEOUT_MODE:
                    break;
                case TX_REG_ADDRESS_READ_MODE:
                    /* Ignore data and change state machine to read data */
                    DL_SPI_receiveData8(SPI_0_INST);
                    gControllerMode = READ_DATA_MODE;
                    break;
                case READ_DATA_MODE:
                    if (gRxByteCount) {
                        /* Receive data until all expected data is read */
                        gRxBuffer[gRxIndex++] =
                            DL_SPI_receiveData8(SPI_0_INST);
                        gRxByteCount--;
                    }
                    if (gRxByteCount == 0) {
                        /* All data is received, reset state machine */
                        gControllerMode = IDLE_MODE;
                    }
                    break;
                case TX_REG_ADDRESS_WRITE_MODE:
                case WRITE_DATA_MODE:
                    /* Ignore the data while transmitting */
                    DL_SPI_receiveData8(SPI_0_INST);
                    break;
            }
            break;
        default:
            break;
    }
}

/*
 *  Controller sends a command to the Peripheral device to write data sent
 *  from the Controller.
 *  After sending the command, the Controller mode will be in
 *  TX_REG_ADDRESS_WRITE_MODE. If there are remaining bytes to transmit, the
 *  Controller will move to TX_DATA_MODE and it will continue to transmit
 *  the remaining bytes. Data sent by the Peripheral will be ignored during the
 *  transmission of the data. After all data is transmitted, the Controller
 *  will move back to IDLE_MODE.
 *
 *  writeCmd  The write command/register address to send to the Peripheral.
 *            Example: CMD_WRITE_TYPE_0
 *  data      The buffer containing the data to send to the Peripheral to write.
 *            Example: gCmdWriteType2Buffer
 *  count     The length of data to read. Example: TYPE_0_LENGTH
 */
static void SPI_Controller_writeReg(
    uint8_t writeCmd, uint8_t *data, uint8_t count)
{
    gControllerMode = TX_REG_ADDRESS_WRITE_MODE;

    /* Copy data to gTxBuffer */
    CopyArray(data, gTxBuffer, count);

    gTxByteCount = count;
    gRxByteCount = count;
    gRxIndex     = 0;
    gTxIndex     = 0;
    writeCmd = writeCmd & 0x7F;

    DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_2);

    /*
     * TX interrupts are disabled and RX interrupts are enabled by default.
     * TX interrupts will be enabled after sending the command, and they will
     * trigger after the FIFO has more space to send all subsequent bytes.
     */
    DL_SPI_clearInterruptStatus(SPI_0_INST, DL_SPI_INTERRUPT_TX);
    DL_SPI_transmitData8(SPI_0_INST, writeCmd);
    DL_SPI_enableInterrupt(SPI_0_INST, DL_SPI_INTERRUPT_TX);

    /* Go to sleep until all data is transmitted */
    while (gControllerMode != IDLE_MODE) {
        __WFI();
    }

    /* Disable TX interrupts after the command is complete */
    DL_SPI_disableInterrupt(SPI_0_INST, DL_SPI_INTERRUPT_TX);
    DL_SPI_clearInterruptStatus(SPI_0_INST, DL_SPI_INTERRUPT_TX);

    DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_2);
}

/*
 *  Controller sends a command to the Peripheral device to read data as
 *  specified by the command and send it to the Controller.
 *  After sending the command, the Controller will be in
 *  TX_REG_ADDRESS_READ_MODE. After receiving the first byte from the
 *  Peripheral, if there are more bytes to receive, then the Controller will
 *  move to RX_DATA_MODE. The Controller will transmit DUMMY_DATA to
 *  receive 'count' number of bytes from the Peripheral.
 *  After 'count' number of bytes have been received, the Controller will move
 *  back to IDLE_MODE. The received data will be available in gRxBuffer.
 *
 *  readCmd   The read command/register address to send to the Peripheral.
 *            Example: CMD_WRITE_TYPE_0
 *  count     The length of data to read. Example: TYPE_0_LENGTH
 */
static void SPI_Controller_readReg(uint8_t readCmd, uint8_t count)
{
    gControllerMode = TX_REG_ADDRESS_READ_MODE;
    gRxByteCount    = count;
    gTxByteCount    = count;
    gRxIndex        = 0;
    gTxIndex        = 0;
    
    readCmd = readCmd | 0x80;

    DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_2);

    /*
     * TX interrupts are disabled and RX interrupts are enabled by default.
     * TX interrupts will be enabled after sending the command, and they will
     * trigger after the FIFO has more space to send all subsequent bytes.
     */
    DL_SPI_clearInterruptStatus(SPI_0_INST, DL_SPI_INTERRUPT_TX);
    DL_SPI_transmitData8(SPI_0_INST, readCmd);
    DL_SPI_enableInterrupt(SPI_0_INST, DL_SPI_INTERRUPT_TX);

    /* Go to sleep until all data is received */
    while (gControllerMode != IDLE_MODE) {
        __WFI();
    }

    /* Disable TX interrupts after the command is complete */
    DL_SPI_disableInterrupt(SPI_0_INST, DL_SPI_INTERRUPT_TX);
    DL_SPI_clearInterruptStatus(SPI_0_INST, DL_SPI_INTERRUPT_TX);
    DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_2);
}


float convert2mpss(int16_t data, BMA530_Range range)
{
    float scale = 2048.0;
    switch (range) {
        case RANGE_2G:
            scale = scale * 8.0;
            break;
        case RANGE_4G:
            scale = scale * 4.0;
            break;
        case RANGE_8G:
            scale = scale * 2.0;
            break;
        default:
            break;
    }
    return data * (9.80665 / scale);
}


void BMA530_ReadAccel(BMA530_AccelXYZ *acc, BMA530_Range range)
{
    uint8_t data[7];
    uint8_t status[2];

    while ((status[1] & 0x01) == 0x01)
    {
        SPI_Controller_readReg(BMA530_REG_SENSOR_STATUS, 1);
        CopyArray(gRxBuffer, status, 1);
    }

    SPI_Controller_readReg(BMA530_REG_ACC_DATA_0, 7);
    CopyArray(gRxBuffer, data, 7);

    acc->x = convert2mpss((int16_t)((data[2] << 8) | data[1]), range);
    acc->y = convert2mpss((int16_t)((data[4] << 8) | data[3]), range);
    acc->z = convert2mpss((int16_t)((data[6] << 8) | data[5]), range);
}


void BMA530_init()
{
    // SPI_Controller_readReg(BMA530_REG_CHIP_ID, TYPE_1_LENGTH);

    // delay_cycles(100000);

    // SPI_Controller_readReg(BMA530_REG_CHIP_ID, TYPE_1_LENGTH);
    // CopyArray(gRxBuffer, gCmdReadType1Buffer, TYPE_1_LENGTH);
    // if (gCmdReadType1Buffer[1] != 0xC2)
    // {
    //     return;
    // }

    // delay_cycles(100000);

    // SPI_Controller_readReg(BMA530_REG_ACCEL_CONF_1, TYPE_1_LENGTH);
    // CopyArray(gRxBuffer, gCmdReadType1Buffer, TYPE_1_LENGTH);
    // uint8_t add_conf_1 = (gCmdReadType1Buffer[1] | 0x80);
    
    // delay_cycles(100000);
    // uint8_t data[] = {add_conf_1};
    // SPI_Controller_writeReg(BMA530_REG_ACCEL_CONF_1, data, 1);

    // delay_cycles(100000);
    // data[0] = 0x00;
    // SPI_Controller_writeReg(0x30, data, 1);

    // delay_cycles(100000);
    // data[0] = 0xA6;
    // SPI_Controller_writeReg(0x31, data, 1);

    // delay_cycles(100000);
    // data[0] = 0x03;
    // SPI_Controller_writeReg(0x32, data, 1);

    // delay_cycles(100000);
    // data[0] = 0x0F;
    // SPI_Controller_writeReg(0x30, data, 1);
}