#include "SX1280.h"

uint8_t RXPacketL, periodBase = PERIODBASE_01_MS;
uint8_t rxBuffer[RXBUFFER_SIZE], txBuffer[RXBUFFER_SIZE];
uint8_t rxtxpinmode;
uint8_t waitIrq = false;

uint8_t SX1280_begin() {
    // use SPI0 at the sx1280 maximum speed
    spi_init(SPI_PORT, LTspeedMaximum);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(PIN_MISO, PIN_MOSI, PIN_SCK, GPIO_FUNC_SPI));

    rxtxpinmode = false;

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, true);

    gpio_init(NRESET);
    gpio_set_dir(NRESET, GPIO_OUT);
    gpio_put(NRESET, false);

    gpio_init(RFBUSY);
    gpio_set_dir(RFBUSY, GPIO_IN);

    if (DIO1 >= 0)
    {
        gpio_init(DIO1);
        gpio_set_dir(DIO1, GPIO_IN);
    }

    if ((RX_EN >= 0) && (TX_EN >= 0))
    {
        gpio_init(RX_EN);
        gpio_set_dir(RX_EN, GPIO_OUT);
        // gpio_put(RX_EN,true);

        gpio_init(TX_EN);
        gpio_set_dir(TX_EN, GPIO_OUT);

        rxtxpinmode = true;
    }

    SX1280_resetDevice();

    if (SX1280_checkDevice())
    {
        return true;
    }

    return false;
}

void SX1280_setupLoRa()
{
    SX1280_setMode(MODE_STDBY_RC);
    SX1280_setRegulatorMode(USE_LDO);
    SX1280_setPacketType(PACKET_TYPE_LORA);
    SX1280_setRfFrequency(Frequency, Offset);
    SX1280_setBufferBaseAddress(0, 0x00);
    SX1280_setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
    SX1280_setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
    SX1280_setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
    SX1280_setHighSensitivity();
}

void SX1280_setRegulatorMode(uint8_t mode)
{
    SX1280_writeCommand(RADIO_SET_REGULATORMODE, &mode, 1);
}

void SX1280_setPacketType(uint8_t packettype)
{
    SX1280_writeCommand(RADIO_SET_PACKETTYPE, &packettype, 1);
}

void SX1280_setRfFrequency(uint32_t frequency, int32_t offset)
{
    frequency = frequency + offset;
    uint8_t buffer[3];
    uint32_t freqtemp = 0;
    freqtemp = (uint32_t)((double)frequency / (double)FREQ_STEP);
    buffer[0] = (uint8_t)((freqtemp >> 16) & 0xFF);
    buffer[1] = (uint8_t)((freqtemp >> 8) & 0xFF);
    buffer[2] = (uint8_t)(freqtemp & 0xFF);
    SX1280_writeCommand(RADIO_SET_RFFREQUENCY, buffer, 3);
}

void SX1280_setBufferBaseAddress(uint8_t txBaseAddress, uint8_t rxBaseAddress)
{
    uint8_t buffer[2];

    buffer[0] = txBaseAddress;
    buffer[1] = rxBaseAddress;
    SX1280_writeCommand(RADIO_SET_BUFFERBASEADDRESS, buffer, 2);
}

void SX1280_setModulationParams(uint8_t modParam1, uint8_t modParam2, uint8_t modParam3)
{
    uint8_t buffer[3];

    buffer[0] = modParam1;
    buffer[1] = modParam2;
    buffer[2] = modParam3;

    SX1280_writeCommand(RADIO_SET_MODULATIONPARAMS, buffer, 3);

    // implement data sheet additions, datasheet SX1280-1_V3.2section 14.47

    SX1280_writeRegister(0x93C, 0x1);

    switch (modParam1)
    {
    case LORA_SF5:
        SX1280_writeRegister(0x925, 0x1E);
        break;
    case LORA_SF6:
        SX1280_writeRegister(0x925, 0x1E);
        break;

    case LORA_SF7:
        SX1280_writeRegister(0x925, 0x37);
        break;

    case LORA_SF8:
        SX1280_writeRegister(0x925, 0x37);
        break;

    case LORA_SF9:
        SX1280_writeRegister(0x925, 0x32);
        break;

    case LORA_SF10:
        SX1280_writeRegister(0x925, 0x32);
        break;

    case LORA_SF11:
        SX1280_writeRegister(0x925, 0x32);
        break;

    case LORA_SF12:
        SX1280_writeRegister(0x925, 0x32);
        break;

    default:
        break;
    }
}

void SX1280_setPacketParams(uint8_t packetParam1, uint8_t packetParam2, uint8_t packetParam3, uint8_t packetParam4, uint8_t packetParam5, uint8_t packetParam6, uint8_t packetParam7)
{
    uint8_t buffer[7];
    buffer[0] = packetParam1;
    buffer[1] = packetParam2;
    buffer[2] = packetParam3;
    buffer[3] = packetParam4;
    buffer[4] = packetParam5;
    buffer[5] = packetParam6;
    buffer[6] = packetParam7;
    SX1280_writeCommand(RADIO_SET_PACKETPARAMS, buffer, 7);
}

void SX1280_setDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask)
{
    uint8_t buffer[8];

    buffer[0] = (uint8_t)(irqMask >> 8);
    buffer[1] = (uint8_t)(irqMask & 0xFF);
    buffer[2] = (uint8_t)(dio1Mask >> 8);
    buffer[3] = (uint8_t)(dio1Mask & 0xFF);
    buffer[4] = (uint8_t)(dio2Mask >> 8);
    buffer[5] = (uint8_t)(dio2Mask & 0xFF);
    buffer[6] = (uint8_t)(dio3Mask >> 8);
    buffer[7] = (uint8_t)(dio3Mask & 0xFF);
    SX1280_writeCommand(RADIO_SET_DIOIRQPARAMS, buffer, 8);
}

void SX1280_setHighSensitivity()
{
    SX1280_writeRegister(REG_LNA_REGIME, (SX1280_readRegister(REG_LNA_REGIME) | 0xC0));
}

void SX1280_resetDevice()
{
    if (NRESET >= 0)
    {
        sleep_ms(20);
        gpio_put(NRESET, false);
        sleep_ms(50);
        gpio_put(NRESET, true);
        sleep_ms(20);
    }
}

void SX1280_readRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
    uint16_t index;
    uint8_t addr_l, addr_h;

    addr_h = address >> 8;
    addr_l = address & 0x00FF;
    SX1280_checkBusy();

    gpio_put(PIN_CS, false);

    uint8_t buf[4];
    buf[0] = RADIO_READ_REGISTER;
    buf[1] = addr_h;
    buf[2] = addr_l;
    buf[3] = 0xFF;

    spi_write_blocking(SPI_PORT, buf, 4);
    // sleep_ms(10);
    spi_read_blocking(SPI_PORT, 0, buffer, size);

    gpio_put(PIN_CS, true);
}

uint8_t SX1280_readRegister(uint16_t address)
{
    uint8_t data;

    SX1280_readRegisters(address, &data, 1);
    return data;
}

void SX1280_writeRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
    uint8_t addr_l, addr_h;
    uint8_t i;

    addr_l = address & 0xff;
    addr_h = address >> 8;
    SX1280_checkBusy();

    gpio_put(PIN_CS, false);

    uint8_t buf[size + 3];
    buf[0] = RADIO_WRITE_REGISTER;
    buf[1] = addr_h;
    buf[2] = addr_l;

    for (i = 0; i < size; i++)
    {
        buf[3 + i] = buffer[i];
    }

    spi_write_blocking(SPI_PORT, buf, size + 3);

    gpio_put(PIN_CS, true);
}

void SX1280_writeRegister(uint16_t address, uint8_t value)
{
    SX1280_writeRegisters(address, &value, 1);
}

uint8_t SX1280_checkDevice()
{
    uint8_t Regdata1, Regdata2;
    Regdata1 = SX1280_readRegister(0x0908); // low byte of frequency setting
    SX1280_writeRegister(0x0908, (Regdata1 + 1));
    Regdata2 = SX1280_readRegister(0x0908); // read changed value back
    SX1280_writeRegister(0x0908, Regdata1); // restore register to original value*/

    if (Regdata2 == (Regdata1 + 1))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void SX1280_checkBusy()
{
    uint32_t startmS = to_ms_since_boot(get_absolute_time());

    do
    {
        if (to_ms_since_boot(get_absolute_time()) - startmS > 9)
        {
#ifdef SX_DEBUG
            // std::cout << "busy timeout !" << std::endl;
            printf("busy timeout !\n");
#endif

            SX1280_resetDevice();
            SX1280_setMode(MODE_STDBY_RC);
            SX1280_setupLoRa();
        }
    } while (gpio_get(RFBUSY));
}

void SX1280_setMode(uint8_t modeconfig)
{
    uint8_t Opcode = 0x80;

    SX1280_checkBusy();

    uint8_t buf[] = {Opcode, modeconfig};
    gpio_put(PIN_CS, false);
    spi_write_blocking(SPI_PORT, buf, 2);
    gpio_put(PIN_CS, true);
}

void SX1280_readCommand(uint8_t Opcode, uint8_t *buffer, uint16_t size)
{
    uint8_t i;
    SX1280_checkBusy();

    gpio_put(PIN_CS, false);

    uint8_t buf[2];
    buf[0] = Opcode;
    buf[1] = 0xFF;

    spi_write_blocking(SPI_PORT, buf, 2);

    spi_read_blocking(SPI_PORT, 0, buffer, size);

    gpio_put(PIN_CS, true);
}

void SX1280_writeCommand(uint8_t Opcode, uint8_t *buffer, uint16_t size)
{
    uint8_t index;
    SX1280_checkBusy();

    gpio_put(PIN_CS, false);
    uint8_t buf[size + 1];
    buf[0] = Opcode;

    for (index = 0; index < size; index++)
    {
        buf[index + 1] = buffer[index];
    }

    spi_write_blocking(SPI_PORT, buf, size + 1);

    gpio_put(PIN_CS, true);

    if (Opcode != RADIO_SET_SLEEP)
    {
        SX1280_checkBusy();
    }
}

void SX1280_setTxParams(uint8_t RampTime)
{
    uint8_t buffer[2];
    // power register is set to 0 to 31 which is -18dBm to +12dBm
    buffer[0] = (TXpower + 18);
    buffer[1] = (uint8_t)RampTime;
    SX1280_writeCommand(RADIO_SET_TXPARAMS, buffer, 2);
}

uint8_t SX1280_receiveSXBuffer(uint8_t startaddr, uint16_t timeout)
{
    uint8_t buffer[2];

    /* setMode(MODE_STDBY_RC);
     setBufferBaseAddress(0, startaddr);                //order is TX RX
     setRx(timeout);

     //0x4022   = IRQ_RX_DONE + IRQ_RX_TX_TIMEOUT + IRQ_HEADER_ERROR
     while (!(readIrqStatus() & 0x4022 ));              //wait for IRQs going active


     setMode(MODE_STDBY_RC);                            //ensure to stop further packet reception

     if ( readIrqStatus() & (IRQ_HEADER_ERROR + IRQ_CRC_ERROR + IRQ_RX_TX_TIMEOUT + IRQ_SYNCWORD_ERROR))
     {
       return 0;                                        //no RX done and header valid only, could be CRC error
     }*/

    SX1280_readCommand(RADIO_GET_RXBUFFERSTATUS, buffer, 2);
    RXPacketL = buffer[0];

    return RXPacketL;
}

void SX1280_startReadSXBuffer(uint8_t ptr)
{
    RXPacketL = 0;

    uint8_t buf[3];
    buf[0] = RADIO_READ_BUFFER;
    buf[1] = ptr;
    buf[2] = 0xFF;

    SX1280_checkBusy();

    gpio_put(PIN_CS, false); // start the burst read
    spi_write_blocking(SPI_PORT, buf, 3);
}

uint8_t SX1280_readBufferChar(char *rxbuffer)
{
    uint8_t index = 0;
    uint8_t regdata[16];

    spi_read_blocking(SPI_PORT, 0, regdata, 16);

    for (int i = 0; i < 16; i++)
    {
        if (regdata[i] != 0)
        {
            rxbuffer[i] = regdata[i];
        }
        else
        {
            RXPacketL = RXPacketL + i;
            index = i;
            break;
        }
        // std::cout << i;
        printf("%d", i);
    }
    // std::cout << std::endl;
    printf("\n");

    return index;
}

int16_t SX1280_readPacketRSSI()
{
    int8_t snr;
    int16_t rssi = 0; // so routine returns 0 if packet not LoRa or FLRC
    uint8_t status[5];

    snr = SX1280_readPacketSNR();
    SX1280_readCommand(RADIO_GET_PACKETSTATUS, status, 5);

    rssi = -status[0] / 2;
    if (snr < 0)
    {
        rssi = rssi + snr;
    }

    return rssi;
}

int8_t SX1280_readPacketSNR()
{
    uint8_t status[5];
    int8_t snr;

    SX1280_readCommand(RADIO_GET_PACKETSTATUS, status, 5);

    if (status[1] < 128)
    {
        snr = status[1] / 4;
    }
    else
    {
        snr = ((status[1] - 256) / 4);
    }

    return snr;
}

void SX1280_readSXBuffer(uint8_t size)
{
    spi_read_blocking(SPI_PORT, 0, rxBuffer, size);
}

void SX1280_getSXBuffer(uint8_t *destination, uint8_t size)
{
    for (uint8_t i; i < size; i++)
    {
        destination[i] = rxBuffer[i];
    }
}

uint8_t SX1280_endReadSXBuffer()
{
    gpio_put(PIN_CS, true);
    return RXPacketL;
}

uint8_t SX1280_isTXReady()
{
    return gpio_get(DIO1);
}

void SX1280_startWriteSXBuffer(uint8_t ptr)
{
    SX1280_setMode(MODE_STDBY_RC);
    SX1280_setBufferBaseAddress(ptr, 0); // TX,RX
    SX1280_checkBusy();

    uint8_t buf[2];
    buf[0] = RADIO_WRITE_BUFFER;
    buf[1] = ptr;

    gpio_put(PIN_CS, false);
    spi_write_blocking(SPI_PORT, buf, 2);
}

void SX1280_writeSXBuffer(uint8_t size)
{
    spi_write_blocking(SPI_PORT, txBuffer, size);
}

void SX1280_endWriteSXBuffer()
{
    gpio_put(PIN_CS, true);
}

void SX1280_transmitSXBuffer(uint8_t startaddr, uint8_t length, uint16_t timeout)
{
    SX1280_setBufferBaseAddress(startaddr, 0); // TX, RX
    SX1280_setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, length, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
    SX1280_setTxParams(RAMP_TIME);
    SX1280_setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0); // set for IRQ on TX done and timeout on DIO1
    SX1280_setTx(timeout);                                                          // this starts the TX
}

uint8_t SX1280_checkIrqs()
{
    if (!waitIrq)
    {
        SX1280_setMode(MODE_STDBY_RC);
        SX1280_setBufferBaseAddress(0, 0); // order is TX RX
        SX1280_setRx(0);
        waitIrq = true;
    }

    if (!(SX1280_readIrqStatus() & 0x4022))
    {
        return 0;
    }
    else
    {
        SX1280_setMode(MODE_STDBY_RC); // ensure to stop further packet reception
        if (SX1280_readIrqStatus() & (IRQ_HEADER_ERROR + IRQ_CRC_ERROR + IRQ_RX_TX_TIMEOUT + IRQ_SYNCWORD_ERROR))
        {
            waitIrq = false;
            return 0; // no RX done and header valid only, could be CRC error
        }
        else
        {
            waitIrq = false;
            return 1;
        }
    }
}

void SX1280_writeUint8(uint8_t u8, uint8_t addr)
{
    txBuffer[addr] = u8;
}

uint8_t SX1280_readUint8(uint8_t addr)
{
    return rxBuffer[addr];
}

void SX1280_writeUint16(uint16_t u16, uint8_t startaddr)
{
    txBuffer[startaddr] = u16 & 0xFF;
    txBuffer[startaddr + 1] = u16 >> 8;
}

uint16_t SX1280_readUint16(uint8_t startaddr)
{
    return (rxBuffer[startaddr + 1] << 8) + rxBuffer[startaddr];
}

void SX1280_writeInt8(int8_t i8, uint8_t addr)
{
    txBuffer[addr] = (i8 & 0xFF);
}

int8_t SX1280_readInt8(uint8_t addr)
{
    return (rxBuffer[addr] & 0xFF);
}

void SX1280_writeInt16(int16_t i16, uint8_t startaddr)
{
    txBuffer[startaddr] = i16 & 0xff;
    txBuffer[startaddr + 1] = i16 >> 8;
}

int16_t SX1280_readInt16(uint8_t startaddr)
{
    return (rxBuffer[startaddr + 1] << 8) + rxBuffer[startaddr];
}

void SX1280_writeFloat(float fl, uint8_t startaddr)
{
    union
    {
        float a;
        uint8_t b[4];
    } data;
    data.a = fl;

    for (uint8_t i = 0; i < 4; i++)
    {
        txBuffer[startaddr + i] = data.b[i];
    }
}

float SX1280_readFloat(uint8_t startaddr)
{
    union
    {
        float a;
        uint8_t b[4];
    } data;

    for (uint8_t i = 0; i < 4; i++)
    {
        data.b[i] = rxBuffer[startaddr + i];
    }
    return data.a;
}

void SX1280_writeChar(char c, uint8_t addr)
{
    txBuffer[addr] = c;
}

char SX1280_readChar(uint8_t addr)
{
    return rxBuffer[addr];
}

void SX1280_setTx(uint16_t timeout)
{
    if (rxtxpinmode)
    {
        SX1280_txEnable();
    }

    uint8_t buffer[3];

    SX1280_clearIrqStatus(IRQ_RADIO_ALL); // clear all interrupt flags
    buffer[0] = periodBase;
    buffer[1] = (uint8_t)((timeout >> 8) & 0x00FF);
    buffer[2] = (uint8_t)(timeout & 0x00FF);
    SX1280_writeCommand(RADIO_SET_TX, buffer, 3);
}

void SX1280_txEnable()
{
    gpio_put(RX_EN, false);
    gpio_put(TX_EN, true);
}

void SX1280_setRx(uint16_t timeout)
{
    uint8_t buffer[3];

    if (rxtxpinmode)
    {
        SX1280_rxEnable();
    }

    SX1280_clearIrqStatus(IRQ_RADIO_ALL); // clear all interrupt flags
    buffer[0] = periodBase;        // use pre determined period base setting
    buffer[1] = (uint8_t)((timeout >> 8) & 0x00FF);
    buffer[2] = (uint8_t)(timeout & 0x00FF);
    SX1280_writeCommand(RADIO_SET_RX, buffer, 3);
}

void SX1280_rxEnable()
{
    gpio_put(RX_EN, true);
    gpio_put(TX_EN, false);
}

void SX1280_clearIrqStatus(uint16_t irqMask)
{
    uint8_t buffer[2];

    buffer[0] = (uint8_t)(irqMask >> 8);
    buffer[1] = (uint8_t)(irqMask & 0xFF);
    SX1280_writeCommand(RADIO_CLR_IRQSTATUS, buffer, 2);
}

uint16_t SX1280_readIrqStatus()
{
    uint16_t temp;
    uint8_t buffer[2];

    SX1280_readCommand(RADIO_GET_IRQSTATUS, buffer, 2);
    temp = ((buffer[0] << 8) + buffer[1]);
    return temp;
}

#ifdef DEBUG
uint8_t SX1280_getTXBuffer(uint8_t addr)
{
    return txBuffer[addr];
}
#endif