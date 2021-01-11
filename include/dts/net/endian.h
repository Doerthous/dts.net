/**
  ******************************************************************************
  * \brief      endian util
  * \file       endian.h
  * \author     doerthous
  * \date       2019-08-27
  * \details    
  ******************************************************************************
  */

#ifndef ENDIAN_H_
#define ENDIAN_H_

#include <stdint.h>

static inline uint16_t lr16(uint8_t *data)
{
    return (data[1] << 8) | data[0];
}

static inline uint8_t * lw16(uint16_t data, uint8_t *buff)
{
    buff[0] = data & 0xFF;
    buff[1] = (data >> 8) & 0xFF;
    return buff+2;
}

static inline uint32_t lr32(uint8_t *data)
{
    return (data[3] << 24) | (data[2] << 16) |
        (data[1] << 8) | data[0];
}

static inline uint8_t * lw32(uint32_t data, uint8_t *buff)
{
    buff[0] = data & 0xFF;
    buff[1] = (data >> 8) & 0xFF;
    buff[2] = (data >> 16) & 0xFF;
    buff[3] = (data >> 24) & 0xFF;
    return buff+4;
}

static inline uint16_t br16(uint8_t *data)
{
    return (data[0] << 8) | data[1];
}

static inline uint8_t * bw16(uint16_t data, uint8_t *buff)
{
    buff[1] = data & 0xFF;
    buff[0] = (data >> 8) & 0xFF;
    return buff+2;
}

static inline uint32_t br32(uint8_t *data)
{
    return (data[0] << 24) | (data[1] << 16) |
        (data[2] << 8) | data[3];
}

static inline uint8_t * bw32(uint32_t data, uint8_t *buff)
{
    buff[3] = data & 0xFF;
    buff[2] = (data >> 8) & 0xFF;
    buff[1] = (data >> 16) & 0xFF;
    buff[0] = (data >> 24) & 0xFF;
    return buff+4;
}

#endif /* ENDIAN_H_ */

/****************************** Copy right 2019 *******************************/
