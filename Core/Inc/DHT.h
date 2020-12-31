/*
 * DHT.h
 *
 *  Created on: Jun 28, 2020
 *      Author: Controllerstech.com
 */

#ifndef DHT_H_
#define DHT_H_



typedef struct
{
	uint8_t error;
	float Temperature;
	float Humidity;
}DHT_DataTypedef;


void DHT_GetData (DHT_DataTypedef *DHT_Data);

#endif /* INC_DHT_H_ */
