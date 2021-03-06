/******************************************************************************
** This file is part of the jpegant project.
**
** Copyright (C) 2009-2013 Vladimir Antonenko
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License,
** or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.
******************************************************************************/

#ifndef __JPEG_H__
#define __JPEG_H__

#include "arch.h"

#ifdef __cplusplus
extern "C" {
#endif

void quantization_lum(conv data[64]);
void quantization_chrom(conv data[64]);
void iquantization_lum(conv data[64]);
void iquantization_chrom(conv data[64]);

#ifdef __cplusplus
}
#endif

#endif//__JPEG_H__
