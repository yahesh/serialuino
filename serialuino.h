/*
 * File:    serialuino.h
 * Author:  Yahe <hello@yahe.sh>
 * Version: 0.1.0
 *
 * Created on 15. December 2013
 *
 * Release 0.1.0 on 16. December 2013
 * initial implementation
 */

/*
 * serialuino.h contains an implementation of a serial communition
 * protocol. It implements a custom subset of the ASN.1 standard.
 * This way several different commands can be encoded.
 *
 * Copyright (C) 2013-2018 Yahe <hello@yahe.sh>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SERIALUINO_H__
#define	__SERIALUINO_H__

// as taken from https://github.com/madsci1016/Arduino-EasyTransfer/blob/master/EasyTransfer/EasyTransfer.h
#if ARDUINO > 22
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "HardwareSerial.h"

#include "chunkuino.h"

#define SERIALUINO_ERROR  0x0F
#define SERIALUINO_HEADER 0xC0
#define SERIALUINO_SLAVE  0x10

#define SERIALUINO_MIN_LENGTH 0x04
#define SERIALUINO_MAX_DATA   0x32

#define SERIALUINO_FALSE 0x00
#define SERIALUINO_TRUE  0xFF

#define SERIALUINO_TIMEOUT 0x01F4

class Serialuino {
public:
	void begin(Stream* port);
	void end();
	
	uint8_t calculateChecksum(CHUNK data);
	uint8_t calculateRequest(bool isSlave, uint8_t action);
	
	uint8_t receiveChunk(CHUNK buffer);
	
	bool sendChunk(uint8_t request, CHUNK data);
	bool sendSingle(uint8_t request, CHUNK_TYPE data);
	
	uint8_t sendChunkAndReceive(uint8_t request, CHUNK data, CHUNK buffer);
	uint8_t sendSingleAndReceive(uint8_t request, CHUNK_TYPE data, CHUNK buffer);
protected:
	Stream* __port;
};

#endif	/* __SERIALUINO_H__ */
