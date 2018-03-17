/*
 * File:    serialuino.cpp
 * Author:  Yahe <hello@yahe.sh>
 * Version: 0.1.0
 *
 * Created on 15. December 2013
 *
 * Release 0.1.0 on 16. December 2013
 * initial implementation
 */

/*
 * serialuino.cpp contains an implementation of a serial communition
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

#include "serialuino.h"

void Serialuino::begin(Stream* port) {
	
	__port = port;
	
	__port->flush();
	while (0x00 <= __port->read()) {}
	
}

void Serialuino::end() {
	
	while (0x00 <= __port->read()) {}
	__port->flush();
	
	__port = NULL;
	
}

// simple XOR checksum
uint8_t Serialuino::calculateChecksum(CHUNK data) {
	
	uint8_t result = 0x00;
	
	CHUNK_SIZE dataSize = sizeof_chunk(data);
	if (0x00 < dataSize) {
		CHUNK_SIZE index = 0x00;
		
		for (index = 0x00; index < dataSize; index++) {
			result ^= get_chunk(data, index);
		}
	}
	
	return result;
	
}

// structure of request is:
// * 110 | (slave bit) | xxxx
uint8_t Serialuino::calculateRequest(bool isSlave, uint8_t action) {
	
	uint8_t result = (SERIALUINO_HEADER & 0xE0) | (action & 0x0F);
	
	if (isSlave) {
		result = (result & 0xEF) | (SERIALUINO_SLAVE & 0x10);
	}
	
	return result;
	
}

uint8_t Serialuino::receiveChunk(CHUNK buffer) {
	
	uint8_t result = 0x00;
	
	if (is_alloc_chunk(buffer)) {
		uint8_t    checksum = 0x00;
		CHUNK_SIZE index    = 0x00;
		uint8_t    length   = 0x00;
		uint8_t    response = 0x00;
		CHUNK_TYPE temp     = 0x00;
		
		// wait for minimal input length
		while (SERIALUINO_MIN_LENGTH > __port->available()) {
			delay(25);
		}
		
		// read response
		response = __port->read();
		checksum ^= response;

		// read length
		length   = __port->read();
		checksum ^= length;
		
		// check if response has some length
		// and if length is not too big
		// (SERIALUINO_MAX_DATA + 1 byte for checksum)
		if ((0x00 < length) && (SERIALUINO_MAX_DATA+0x01 >= length)) {
			// wait for length
			while (length > __port->available()) {
				delay(25); 
			}
			
			// copy to result
			if (resize_chunk(buffer, length-0x01)) {
				for (index = 0x00; index < length-0x01; index++) {
					temp = __port->read();
					
					set_chunk(buffer, index, temp);
					checksum ^= temp;
				}
				
				// test if checksum is correct
				if (__port->read() == checksum) {
					result = response;
				}
			}
		}
			
		// something went wrong
		if (0x00 == result) {
			// kill Serial buffer
			while (0x00 <= __port->read()) {}
		}	
	}

	return result;
	
}

// structure of sent package is:
// * request [1 byte]
// * length [1 byte]
// * data [(length-1) bytes]
// * checksum [1 byte]
//
// checksum is XOR of whole package
bool Serialuino::sendChunk(uint8_t request, CHUNK data) {

	bool result = false;
	
	CHUNK_SIZE dataSize = sizeof_chunk(data);
	if ((0x00 < dataSize) && (SERIALUINO_MAX_DATA >= dataSize)) {
		uint8_t    checksum = 0x00;
		CHUNK_SIZE index    = 0x00;
		uint8_t    length   = dataSize+0x01;
		CHUNK_TYPE temp     = 0x00;

		// send request
		__port->write(request);
		checksum ^= request;
		
		// send length
		__port->write(length);
		checksum ^= length;

		// send bytes of data
		for (index = 0x00; index < dataSize; index++) {
			temp = get_chunk(data, index);
		
			// send single byte of data
			__port->write(temp);
			checksum ^= temp;
		}

		// send checksum
		__port->write(checksum);
		
		// wait until sending is finished
		__port->flush();
		
		result = true;
	}

	return result;
		
}

bool Serialuino::sendSingle(uint8_t request, CHUNK_TYPE data) {
	
	bool result = false;
	
	// allocate new chunk and send data via sendChunk()
	CHUNK temp = alloc_chunk(0x01);
	if (is_alloc_chunk(temp)) {
		set_chunk(temp, 0x00, data);
		result = this->sendChunk(request, temp);
		dealloc_chunk(temp);
	}
	
	return result;
	
}

uint8_t Serialuino::sendChunkAndReceive(uint8_t request, CHUNK data, CHUNK buffer) {

	uint8_t result = 0x00;
	
	if (is_alloc_chunk(buffer)) {
		if (this->sendChunk(request, data)) {
			result = this->receiveChunk(buffer);
		}
	}
	
	return result;
	
}

uint8_t Serialuino::sendSingleAndReceive(uint8_t request, uint8_t data, CHUNK buffer) {
	
	uint8_t result = 0x00;
	
	// allocate new chunk and send data via sendChunkAndReceive()
	CHUNK temp = alloc_chunk(0x01);
	if (is_alloc_chunk(temp)) {
		set_chunk(temp, 0x00, data);
		result = this->sendChunkAndReceive(request, temp, buffer);
		dealloc_chunk(temp);
	}
	
	
	return result;
	
}
