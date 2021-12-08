#ifndef __UNPACK_H__
#define __UNPACK_H__

#include "rpi.h"
#include "color.h"

unsigned unpack(Color* col, unsigned *joy, uint32_t data);

#endif