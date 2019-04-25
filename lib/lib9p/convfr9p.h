/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#define CHAR(v, b, n, m)	(v = frombytes(b, &n, m, 1))
#define SHORT(v, b, n, m)	(v = frombytes(b, &n, m, 2))
#define LONG(v, b, n, m)	(v = frombytes(b, &n, m, 4))
#define LLONG(v, b, n, m)	(v = frombytes(b, &n, m, 8))
#define STRING(str, s, b, n, m)	(fromstring(b, &n, m, str, s))

extern long long frombytes(unsigned char *b, int *n, int max, int nbytes);
extern char *fromstring(unsigned char *b, int *n, int max, char *str, int s);
