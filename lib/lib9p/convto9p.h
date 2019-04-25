/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#define CHAR(x, p)	(p = tobytes(p, x, 1))
#define SHORT(x, p)	(p = tobytes(p, x, 2))
#define LONG(x, p)	(p = tobytes(p, x, 4))
#define LLONG(x, p)	(p = tobytes(p, x, 8))
#define STRING(s, p)	(p = tostring(p, s))

unsigned char *tobytes(unsigned char *p, unsigned long long v, int nbytes);
unsigned char *tostring(unsigned char *p, char *s);
