/*
   Copyright (c) 2019 Boot&Work Corp., S.L. All rights reserved

   This library is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FTP.h"

#define BUFF_SIZE 1024 //64

static char buff[BUFF_SIZE];
static size_t buffLen = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////
FTP::FTP(Client &cClient, Client &dClient) : cClient(cClient), dClient(dClient) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::connect(IPAddress ip, uint16_t port, const char *user, const char *password) {
    Serial.println("###>>0<<");
	return connect(ip, port) && auth(user, password);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::connect(const char *host, uint16_t port, const char *user, const char *password) {
	Serial.println("###>>1<<");
	return connect(host, port) && auth(user, password);
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::connect(IPAddress ip, uint16_t port) {
	Serial.println("###>>2<<");
	return cClient.connect(ip, port) && cClient.connected();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::connect(const char *host, uint16_t port) {
	Serial.println("###>>3<<");
	return cClient.connect(host, port) && cClient.connected();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::auth(const char *user, const char *password) {
	Serial.println("###iniciando auth");
	if (!user) {
        Serial.println("###No se ha introducido usuario");
		return 0;
	}
Serial.println("###esperando respuesta");
	if (waitServerCode() != 220) {
	//	return 0;
	}
Serial.println("###-0-");
	switch (waitServerCode(F("USER"), user)) {
		case 331:
			if (!password) {
				Serial.println("###No se ha introducido contraseña");
				return 0;
			}

			if (waitServerCode(F("PASS"), password) != 230) {
				return 0;
			}
			break;

		case 230:
			// Logged in
			Serial.println("###LOGGED IN");
			break;

		default:
			return 0;
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::beginTransaction() {
	if (!cClient.connected()) {
		Serial.println("### Servidor no conectado");
		return 0;
	}

	if (!waitServerCode(F("TYPE I"), nullptr, 200)) {
		Serial.println("### Err0");
		return 0;
	}

	cClient.println(F("PASV"));
#if DEBUG
	Serial.print("> ");
	Serial.println(F("PASV"));
#endif

	char pasvResponse[70];
	if (waitServerCode(pasvResponse) != 227) {
		Serial.println("### Err1");
		return 0;
	}

	uint8_t params[6];
	char *ptr = strtok(pasvResponse, "(");
	for (int i = 0; i < 6; ++i) {
		ptr = strtok(nullptr, ",");
		if (!ptr) {
			Serial.println("### Err2");
			return 0;
		}
		params[i] = atoi(ptr);
		Serial.println("parametros >>>>>>>>>>>>>>>>>>>>>>>>>>"+String(params[i]));
	}

	IPAddress dataAddress(params[0], params[1], params[2], params[3]);
	uint16_t dataPort = (uint16_t(params[4]) << 8) | (params[5] & 0xff);
Serial.println("###>>>>>>>>>>>>>>>>>>>>>>>>>>Conectando ");
	dClient.connect(dataAddress, dataPort);
	delay(500);

	
	if (!dClient.connected()) {
		Serial.println("### Cliente desconectado");
		return 0;
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
size_t FTP::retrieve(const char *fileName, Stream &stream) {
	if (!beginTransaction()) {
		Serial.println("### Err4");
		return 0;
	}

	bool alreadyClosed = false;
	int code = waitServerCode(F("RETR"), fileName);
	switch (code) {
		case 125: // Data connection already open
		case 150: // Data connection opened
			break;
		case 226: // Data connection already closed
			alreadyClosed = true;
			break;
		default:
			quit();
			Serial.println("### Err5");
			return 0;
	}

	uint32_t startTime = millis();
	uint8_t chunk[_FTP_CHUNK_SIZE];
	size_t size = 0;
	do {
		size_t len = dClient.available();
		if (len > 0) {
			if (len > _FTP_CHUNK_SIZE) {
				len = _FTP_CHUNK_SIZE;
			}

			startTime = millis();
			len = dClient.read(chunk, len);
			if (stream.write(chunk, len) == 0) {
				break;
			}
			size += len;
		} else if (!dClient.connected()) {
			break;
		} else {
			delay(1);
		}
	} while (millis() - startTime < _FTP_TIMEOUT);

	dClient.stop();

	if (alreadyClosed) {
		if (waitServerCode() != 226) {
			Serial.println("### Err6");
			return 0;
		}
	}

	return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
size_t FTP::store(const char *fileName, Stream &stream) {
	
	Serial.println(">>>3<<<");
	if (!beginTransaction()) {
		Serial.println(">>>4<<<");
		return 0;
	}
Serial.println(">>>5<<<");

	int code = waitServerCode(F("STOR"), fileName);
	switch (code) {
		case 125: // Data connection already open
		case 150: // Data connection opened
			break;
		default:
		Serial.println("### Conexion de datos cerrada");
			return 0;
	}

	uint8_t chunk[_FTP_CHUNK_SIZE];
	size_t size = 0;
	do {
		size_t len = stream.available();
		if (len == 0) {
			Serial.println("### Stream no dispo");
			break;
		}
		if (len > _FTP_CHUNK_SIZE) {
			len = _FTP_CHUNK_SIZE;
		}
		len = stream.readBytes(chunk, len);
		
		if (dClient.write(chunk, len) == 0) {
			Serial.println("### Finalizando envio");
			break;
		}
		size += len;
	} while (true);

	dClient.stop();

	if (waitServerCode() != 226) {
		Serial.println("### Err8");
		return 0;
	}

	return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int FTP::waitServerCode(const __FlashStringHelper *cmd, const char *arg, uint16_t response) {
	return waitServerCode(cmd, arg, nullptr) == response;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t FTP::waitServerCode(const __FlashStringHelper *cmd, const char *arg, char *desc) {
	cClient.print(cmd);
#if DEBUG
	Serial.print("> ");
	Serial.print(cmd);
#endif

	if (arg) {
		cClient.print(' ');
		cClient.print(arg);
#if DEBUG
		Serial.print(' ');
		Serial.print(arg);
#endif
	}

	cClient.println();
#if DEBUG
	Serial.println();
#endif

	return waitServerCode(desc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t FTP::waitServerCode(char *desc) {
	uint32_t startTime = millis();
	size_t len;
	do {
		len = cClient.available();
		delay(1);
	} while ((len < 4) && (millis() - startTime <= _FTP_TIMEOUT));

	if (len == 0) {
		Serial.println("### Err9");
		return 0;
	}

#if DEBUG
	Serial.print("< ");
#endif


	uint16_t code = 0;
	bool codeDone = false;
	bool descDone = false;
	do {
		len = cClient.available();
		if (len > 0) {
			if (len + buffLen > BUFF_SIZE) {
				len = BUFF_SIZE - buffLen;
			}
			// virtual int read(uint8_t *buf, size_t size) = 0;  
			//static char buff[BUFF_SIZE];  //static size_t buffLen = 0;
			/////////uint8_t* suma_ =(uint8_t*) buff + buffLen;
			/////////buffLen += cClient.read(suma_, len);  //MODIF!!! original: buffLen += cClient.read(buff + buffLen, len);  
		
			buffLen += cClient.read((uint8_t*)buff + buffLen, len);  
            Serial.println("bufflen: "+String(buffLen));
			  
			char *ptr = buff;
			char c;
			Serial.print("###>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Recibido: ");
			while (buffLen > 0) {
				c = *ptr++;
				--buffLen;

				if (c == '\r') {
					
#if DEBUG
					Serial.print("<CR>");
#endif
					// Nothing to do
				} else if (c == '\n') {
					
#if DEBUG
					Serial.println("<NL>");
#endif
					descDone = true;
					break;
				} else {
//#if DEBUG
					Serial.print(c);
//#endif
					if (!codeDone) {
						if (c == ' ') {
							codeDone = true;
						} else {
							code *= 10;
							code += c - '0';
						}
					} else if (desc) {
						*desc++ = c;
					}
				}
			}
			
			if (buffLen == BUFF_SIZE) {
				buffLen = 0;
			} else if (buffLen > 0) {
				memcpy(buff, ptr, buffLen);
			}
		}
	} while ((millis() - startTime <= _FTP_TIMEOUT) && !descDone);

	if (desc) {
		*desc = '\0';
	}
	
	Serial.println("###"+String(code));

	return code;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void FTP::stop() {
	if (cClient.connected()) {
		quit();
		cClient.stop();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void FTP::quit() {
	waitServerCode(F("QUIT"));
	if (dClient.connected()) {
		dClient.stop();
	}
}
