// SPDX-License-Identifier: Apache-2.0
/*
 * UCS-2 to UTF-8 conversion
 *
 * Ported from sms_tool pdu_lib by iamromulan
 * Original: 2014 lovewilliam <ztong@vt.edu>
 * From http://www.lemoda.net/c/ucs2-to-utf8/ucs2-to-utf8.c
 */

#include "pdu.h"

#define UNICODE_SURROGATE_PAIR	-2
#define UNICODE_BAD_INPUT	-1

int ucs2_to_utf8(int ucs2, unsigned char *utf8)
{
	if (ucs2 < 0x80) {
		utf8[0] = ucs2;
		utf8[1] = '\0';
		return 1;
	}
	if (ucs2 >= 0x80 && ucs2 < 0x800) {
		utf8[0] = (ucs2 >> 6) | 0xC0;
		utf8[1] = (ucs2 & 0x3F) | 0x80;
		utf8[2] = '\0';
		return 2;
	}
	if (ucs2 >= 0x800 && ucs2 < 0xFFFF) {
		if (ucs2 >= 0xD800 && ucs2 <= 0xDFFF)
			return UNICODE_SURROGATE_PAIR;
		utf8[0] = ((ucs2 >> 12)) | 0xE0;
		utf8[1] = ((ucs2 >> 6) & 0x3F) | 0x80;
		utf8[2] = ((ucs2) & 0x3F) | 0x80;
		utf8[3] = '\0';
		return 3;
	}
	if (ucs2 >= 0x10000 && ucs2 < 0x10FFFF) {
		utf8[0] = 0xF0 | (ucs2 >> 18);
		utf8[1] = 0x80 | ((ucs2 >> 12) & 0x3F);
		utf8[2] = 0x80 | ((ucs2 >> 6) & 0x3F);
		utf8[3] = 0x80 | ((ucs2 & 0x3F));
		utf8[4] = '\0';
		return 4;
	}
	return UNICODE_BAD_INPUT;
}
