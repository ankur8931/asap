/****************************************************************************
 *                          utf8.h
 * This header file defines constants and macros to be used for utf8
 * decoding.
 *
 ***************************************************************************/

#ifndef UTF8_H_INCLUDED
#define UTF8_H_INCLUDED

#define ISUTF8_MB(c) ((unsigned)(c) >= 0xc0 && (unsigned)(c) <= 0xfd)

#define utf8_get_char(in, chr) \
	(*(in) & 0x80 ? __utf8_get_char(in, chr) : *chr=*in, ++in)

extern char *__utf8_get_char(const char *in, int *chr);

#endif /*UTF8_H_INCLUDED*/
