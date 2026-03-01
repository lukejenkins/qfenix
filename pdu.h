/* SPDX-License-Identifier: Apache-2.0 */
/*
 * SMS PDU encoding/decoding library
 *
 * Ported from sms_tool pdu_lib by iamromulan
 * Original authors:
 *   2017-2021 Cezary Jackiewicz <cezary@eko.one.pl>
 *   2014 lovewilliam <ztong@vt.edu>
 *   2011 The Avalon Project Authors
 */
#ifndef __PDU_H__
#define __PDU_H__

#include <time.h>

#define SMS_MAX_PDU_LENGTH	256
#define SMS_MAX_7BIT_TEXT_LEN	160

/*
 * Encode an SMS message into PDU format.
 * Returns encoded length or negative on error.
 */
int pdu_encode(const char *smsc, const char *phone, const char *text,
	       unsigned char *pdu, int pdu_size);

/*
 * Decode a PDU message into human-readable components.
 * Returns decoded text length or negative on error.
 */
int pdu_decode(const unsigned char *pdu, int pdu_len,
	       time_t *sms_time,
	       char *phone, int phone_size,
	       char *text, int text_size,
	       int *tp_dcs,
	       int *ref_number,
	       int *total_parts,
	       int *part_number,
	       int *skip_bytes);

/*
 * Encode 7-bit ASCII text into packed 8-bit PDU octets.
 * Returns number of octets written or negative on error.
 */
int pdu_encode_7bit(const char *text, int text_len,
		    unsigned char *out, int out_size);

/*
 * Decode packed 8-bit PDU octets into 7-bit ASCII text.
 * Returns number of characters decoded.
 */
int pdu_decode_7bit(const unsigned char *buf, int buf_len,
		    char *out, int out_len);

/*
 * Convert a UCS-2 code point to UTF-8.
 * Returns number of bytes written or negative on error.
 */
int ucs2_to_utf8(int ucs2, unsigned char *utf8);

#endif /* __PDU_H__ */
