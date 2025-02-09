/*
  Copyright 2022 - Juaneda Axel
  Licensed under a MIT license displayed at the bottom of this document.

  This code was adapted from the library of Stuart Robinson https://github.com/StuartsProjects/SX12XX-LoRa
*/

#ifndef SX1280_H // include guard
#define SX1280_H

#include <stdio.h>
// #include <iostream>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"

#include "Settings.h"
#include "SX1280_Def.h"

void SX1280_setupLoRa();
uint8_t SX1280_begin();

void SX1280_setMode(uint8_t modeconfig);
void SX1280_setRegulatorMode(uint8_t mode);
void SX1280_setPacketType(uint8_t packettype);
void SX1280_setRfFrequency(uint32_t frequency, int32_t offset);
void SX1280_setBufferBaseAddress(uint8_t txBaseAddress, uint8_t rxBaseAddress);
void SX1280_setModulationParams(uint8_t modParam1, uint8_t modParam2, uint8_t modParam3);
void SX1280_setPacketParams(uint8_t packetParam1, uint8_t packetParam2, uint8_t packetParam3, uint8_t packetParam4, uint8_t packetParam5, uint8_t packetParam6, uint8_t packetParam7);
void SX1280_setDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask);
void SX1280_setHighSensitivity();
void SX1280_setTxParams(uint8_t RampTime);

void SX1280_resetDevice();

uint8_t SX1280_receiveSXBuffer(uint8_t startaddr, uint16_t timeout);
void SX1280_startReadSXBuffer(uint8_t ptr);
uint8_t SX1280_readBufferChar(char *rxbuffer);

int16_t SX1280_readPacketRSSI();
int8_t SX1280_readPacketSNR();

void SX1280_readSXBuffer(uint8_t size);
void SX1280_getSXBuffer(uint8_t *destination, uint8_t size);

uint8_t SX1280_endReadSXBuffer();

uint8_t SX1280_isTXReady();
void SX1280_startWriteSXBuffer(uint8_t ptr);
void SX1280_writeSXBuffer(uint8_t size);
void SX1280_endWriteSXBuffer();
void SX1280_transmitSXBuffer(uint8_t startaddr, uint8_t length, uint16_t timeout);

uint8_t SX1280_checkIrqs();

void SX1280_writeUint8(uint8_t u8, uint8_t addr);
uint8_t SX1280_readUint8(uint8_t addr);

void SX1280_writeUint16(uint16_t u16, uint8_t startaddr);
uint16_t SX1280_readUint16(uint8_t startaddr);

void SX1280_writeInt8(int8_t i8, uint8_t addr);
int8_t SX1280_readInt8(uint8_t addr);

void SX1280_writeInt16(int16_t i16, uint8_t startaddr);
int16_t SX1280_readInt16(uint8_t startaddr);

void SX1280_writeFloat(float fl, uint8_t startaddr);
float SX1280_readFloat(uint8_t startaddr);

void SX1280_writeChar(char c, uint8_t addr);
char SX1280_readChar(uint8_t addr);

#ifdef DEBUG
    uint8_t SX1280_getTXBuffer(uint8_t addr);
#endif

void SX1280_checkBusy();
uint8_t SX1280_checkDevice();

void SX1280_writeRegister(uint16_t address, uint8_t value);
void SX1280_readRegisters(uint16_t address, uint8_t *buffer, uint16_t size);
uint8_t SX1280_readRegister(uint16_t address);
void SX1280_writeRegisters(uint16_t address, uint8_t *buffer, uint16_t size);

void SX1280_readCommand(uint8_t Opcode, uint8_t *buffer, uint16_t size);
void SX1280_writeCommand(uint8_t Opcode, uint8_t *buffer, uint16_t size);

void SX1280_setTx(uint16_t timeout);
void SX1280_txEnable();

void SX1280_setRx(uint16_t timeout);
void SX1280_rxEnable();

void SX1280_clearIrqStatus(uint16_t irqMask);
uint16_t SX1280_readIrqStatus();

// uint8_t RXPacketL, periodBase = PERIODBASE_01_MS;
// uint8_t rxBuffer[RXBUFFER_SIZE], txBuffer[RXBUFFER_SIZE];
// uint8_t rxtxpinmode;
// uint8_t waitIrq = false;

#endif

/*
  MIT license
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
  documentation files (the "Software"), to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/
