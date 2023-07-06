/*
 * crc32.h
 *
 *  Created on: 14/03/2023
 *      Author: luisferreira
 */

#ifndef INC_CRC32_H_
#define INC_CRC32_H_
#include <stdint.h>

uint32_t xcrc32 (const unsigned char *buf, int len, uint32_t init);

#endif /* INC_CRC32_H_ */
