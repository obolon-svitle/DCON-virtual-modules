#include "smbus.h"

// Bus free time between Stop and Start condition
#define TBUF 30

// Clock low and high period
#define LOWLEV 30
#define HIGHLEV 46

// Data setup time
#define TSET 3
#define ERRCOUNT 255

#define ACK  0
#define NACK 1

#define mSDA_HIGH() {GPIOPinTypeGPIOOutput(SDA_PIN); \
                                            GPIOPinWrite(SDA_PIN,0xff);}
#define mSDA_LOW()  {GPIOPinTypeGPIOOutput(SDA_PIN); \
                                            GPIOPinWrite(SDA_PIN,0x0);}

#define mSCL_HIGH() {GPIOPinTypeGPIOOutput(SCL_PIN); \
                                            GPIOPinWrite(SCL_PIN,0xff);}
#define mSCL_LOW()  {GPIOPinTypeGPIOOutput(SCL_PIN); \
                                            GPIOPinWrite(SCL_PIN,0x0);}

/* forward declarations */
static inline void MCUinit(void);
static inline void START_bit(void);
static inline void STOP_bit(void);
static inline void send_bit(uint8_t bit_out);
static inline uint8_t receive_bit(void);
static inline uint8_t byte_TX(uint8_t Tx_buffer);
static inline uint8_t byte_RX(uint8_t ack_status);
static inline uint8_t calculate_PEC(uint8_t pec[]);

static inline void MCUinit(void) {
    mSDA_HIGH();
    mSCL_HIGH();
}

static inline void START_bit(void) {
    mSDA_HIGH();
    SysCtlDelay(TBUF);

    mSCL_HIGH();
    SysCtlDelay(TBUF);

    mSDA_LOW();
    SysCtlDelay(TBUF);

    mSCL_LOW();
    SysCtlDelay(TBUF);
}

static inline void STOP_bit(void) {
    mSCL_LOW();
    SysCtlDelay(TBUF);

    mSDA_LOW();
    SysCtlDelay(TBUF);

    mSCL_HIGH();
    SysCtlDelay(TBUF);

    mSDA_HIGH();
}

static inline uint8_t byte_TX(uint8_t Tx_buffer) {
    uint8_t Bit_counter;
    uint8_t Ack_bit;
    uint8_t bit_out;

    for (Bit_counter = 8; Bit_counter; Bit_counter--) {
        if (Tx_buffer & 0x80) {
            bit_out = 1;
        } else {
            bit_out = 0;
        }

        send_bit(bit_out);
        Tx_buffer <<= 1;
    }

    Ack_bit = receive_bit();
    return Ack_bit;
}

static inline void send_bit(uint8_t bit_out) {
    if (bit_out == 0)
        mSDA_LOW()
        else {
            mSDA_HIGH();
        }

    SysCtlDelay(TSET);

    mSCL_HIGH();
    SysCtlDelay(HIGHLEV);

    mSCL_LOW();
    SysCtlDelay(LOWLEV);
}

static inline uint8_t byte_RX(uint8_t ack_status) {
    uint8_t RX_buffer;
    uint8_t Bit_Counter;

    for (Bit_Counter = 8; Bit_Counter; Bit_Counter--) {
        if (receive_bit()) {
            RX_buffer <<= 1;
            RX_buffer |= 0x1;
        } else {
            RX_buffer <<= 1;
            RX_buffer &= 0xfe;
        }
    }

    send_bit(ack_status);

    return RX_buffer;
}

static inline uint8_t receive_bit(void) {
    uint8_t ack_bit;

    GPIOPinTypeGPIOInput(SDA_PIN);

    mSCL_HIGH();
    SysCtlDelay(HIGHLEV);

    if (GPIOPinRead(SDA_PIN)) {
        ack_bit = 1;
    } else {
        ack_bit = 0;
    }

    mSCL_LOW();
    SysCtlDelay(LOWLEV);

    return ack_bit;
}

static inline uint8_t calculate_PEC(uint8_t pec[]) {
    uint8_t crc[6];
    uint8_t shift;
    uint8_t i;
    uint8_t j;
    uint8_t temp;
    uint8_t bitPos = 47;

    do {
        crc[5] = 0;             /* Load CRC value 0x00000000017 */
        crc[4] = 0;
        crc[3] = 0;
        crc[2] = 0;
        crc[1] = 0x01;
        crc[0] = 0x07;

        bitPos = 47;        /* Set maximum bit position at 47 */
        shift = 0;

        i = 5;
        j = 0;
        while ((pec[i] & (0x80 >> j)) == 0 && i > 0) {
            bitPos--;
            if (j < 7) {
                j++;
            } else {
                j = 0x00;
                i--;
            }
        }

        shift = bitPos - 8; /*Get shift value for crc value*/

        while (shift) {
            for (i = 5; i < 0xFF; i--) {
                if ((crc[i - 1] & 0x80) && (i > 0)) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                crc[i] <<= 1;
                crc[i] += temp;
            }
            shift--;
        }

        for (i = 0; i <= 5; i++) {
            pec[i] ^= crc[i];
        }
    } while (bitPos > 8);

    return pec[0];
}

uint16_t memRead(uint8_t SlaveAddress, uint8_t command) {
    uint16_t data;
    uint8_t pec;
    uint8_t dataLow;
    uint8_t dataHigh;
    uint8_t arr[6];
    uint8_t pecReg;
    uint8_t errcount;

    MCUinit();

    errcount = ERRCOUNT;

    do {
        while (--errcount != 0) {
            STOP_bit();
            START_bit();

            if (byte_TX(SlaveAddress)) {
                continue;
            }

            if (byte_TX(command)) {
                continue;
            }

            START_bit();

            if (byte_TX(SlaveAddress)) {
                continue;
            }

            dataLow = byte_RX(ACK);
            dataHigh = byte_RX(ACK);
            pec = byte_RX(NACK);

            STOP_bit();

            arr[5] = SlaveAddress;
            arr[4] = command;
            arr[3] = SlaveAddress;
            arr[2] = dataLow;
            arr[1] = dataHigh;
            arr[0] = 0;
            pecReg = calculate_PEC(arr);
            break;
        }

    } while (pecReg != pec);

    *((uint8_t *)(&data)) = dataLow;
    *((uint8_t *)(&data) + 1) = dataHigh;

    return data;
}
