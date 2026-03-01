// SPDX-License-Identifier: Apache-2.0
/*
 * SMS PDU encoding/decoding library
 *
 * Ported from sms_tool pdu_lib by iamromulan
 * Original authors:
 *   2017-2021 Cezary Jackiewicz <cezary@eko.one.pl>
 *   2014 lovewilliam <ztong@vt.edu>
 *   2011 The Avalon Project Authors
 *
 * SMS encoding examples from: http://www.dreamfabric.com/sms/
 */

#include "pdu.h"

#ifdef _WIN32
#define timegm _mkgmtime
#endif

#include <string.h>
#include <time.h>

enum {
	BITMASK_7BITS		= 0x7F,
	BITMASK_8BITS		= 0xFF,
	BITMASK_HIGH_4BITS	= 0xF0,
	BITMASK_LOW_4BITS	= 0x0F,

	TYPE_OF_ADDR_UNKNOWN	   = 0x81,
	TYPE_OF_ADDR_INTERNATIONAL = 0x91,
	TYPE_OF_ADDR_NATIONAL	   = 0xC8,
	TYPE_OF_ADDR_ALPHANUMERIC  = 0xD0,

	SMS_DELIVER_ONE_MESSAGE	= 0x04,
	SMS_SUBMIT		= 0x11,
};

#define GSM_7BITS_ESCAPE	0x1b
#define NPC			'?'

/* Swap decimal nibbles: e.g. 0x12 -> 21 */
static unsigned char swap_decimal_nibble(unsigned char x)
{
	return (x / 16) + ((x % 16) * 10);
}

/* Encode 7-bit text into packed 8-bit PDU octets */
int pdu_encode_7bit(const char *text, int text_len,
		    unsigned char *out, int out_size)
{
	int out_len = 0;
	int carry = 1;
	int i = 0;

	if ((text_len * 7 + 7) / 8 > out_size)
		return -1;

	for (; i < text_len - 1; ++i) {
		out[out_len++] =
			((text[i] & BITMASK_7BITS) >> (carry - 1)) |
			((text[i + 1] & BITMASK_7BITS) << (8 - carry));
		carry++;
		if (carry == 8) {
			carry = 1;
			++i;
		}
	}

	if (i <= text_len)
		out[out_len++] = (text[i] & BITMASK_7BITS) >> (carry - 1);

	return out_len;
}

/* Decode packed 8-bit PDU octets into 7-bit ASCII text */
int pdu_decode_7bit(const unsigned char *buf, int buf_len,
		    char *out, int out_len)
{
	int text_len = 0;
	int carry = 1;
	int i = 1;

	if (buf_len > 0)
		out[text_len++] = BITMASK_7BITS & buf[0];

	if (out_len > 1) {
		for (; i < buf_len; ++i) {
			out[text_len++] = BITMASK_7BITS &
				((buf[i] << carry) | (buf[i - 1] >> (8 - carry)));

			if (text_len == out_len)
				break;

			carry++;
			if (carry == 8) {
				carry = 1;
				out[text_len++] = buf[i] & BITMASK_7BITS;
				if (text_len == out_len)
					break;
			}
		}
		if (text_len < out_len)
			out[text_len++] = buf[i - 1] >> (8 - carry);
	}

	return text_len;
}

/* GSM 7-bit default alphabet to Latin-1 */
static const unsigned char gsm7_to_latin1[128] = {
	'@', 0xa3,  '$', 0xa5, 0xe8, 0xe9, 0xf9, 0xec,
	0xf2, 0xc7, '\n', 0xd8, 0xf8, '\r', 0xc5, 0xe5,
	   0,  '_',    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0, 0xc6, 0xe6, 0xdf, 0xc9,
	 ' ',  '!',  '"',  '#', 0xa4,  '%',  '&', '\'',
	 '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
	 '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
	 '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
	0xa1,  'A',  'B',  'C',  'D',  'E',  'F',  'G',
	 'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
	 'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
	 'X',  'Y',  'Z', 0xc4, 0xd6, 0xd1, 0xdc, 0xa7,
	0xbf,  'a',  'b',  'c',  'd',  'e',  'f',  'g',
	 'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	 'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
	 'x',  'y',  'z', 0xe4, 0xf6, 0xf1, 0xfc, 0xe0,
};

/* GSM 7-bit extension table (after escape char 0x1b) */
static const unsigned char gsm7_ext_to_latin1[128] = {
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0, '\f',    0,    0,    0,    0,    0,
	   0,    0,    0,    0,  '^',    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	 '{',  '}',    0,    0,    0,    0,    0, '\\',
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,  '[',  '~',  ']',    0,
	 '|',    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
};

/* Convert GSM 7-bit decoded buffer to Latin-1/ASCII in place */
static int gsm7_to_ascii(char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if ((unsigned char)buf[i] < 128) {
			if (buf[i] == GSM_7BITS_ESCAPE) {
				buf[i] = gsm7_ext_to_latin1[(unsigned char)buf[i + 1]];
				memmove(&buf[i + 1], &buf[i + 2], len - i - 1);
				len--;
			} else {
				buf[i] = gsm7_to_latin1[(unsigned char)buf[i]];
			}
		}
	}
	return len;
}

/* Latin-1/ASCII to GSM 7-bit lookup table */
static const int latin1_to_gsm7[256] = {
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	NPC,  NPC, 0x0a,  NPC,-0x0a, 0x0d,  NPC,  NPC,
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	0x20, 0x21, 0x22, 0x23, 0x02, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a,-0x3c,-0x2f,-0x3e,-0x14, 0x11,
	NPC,  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a,-0x28,-0x40,-0x29,-0x3d,  NPC,
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	NPC,  0x40,  NPC, 0x01, 0x24, 0x03,  NPC, 0x5f,
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	NPC,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC, 0x60,
	NPC,  NPC,  NPC,  NPC, 0x5b, 0x0e, 0x1c, 0x09,
	NPC,  0x1f,  NPC,  NPC,  NPC,  NPC,  NPC,  NPC,
	NPC,  0x5d,  NPC,  NPC,  NPC,  NPC, 0x5c,  NPC,
	0x0b, NPC,  NPC,  NPC, 0x5e,  NPC,  NPC, 0x1e,
	0x7f, NPC,  NPC,  NPC, 0x7b, 0x0f, 0x1d,  NPC,
	0x04, 0x05,  NPC,  NPC, 0x07,  NPC,  NPC,  NPC,
	NPC,  0x7d, 0x08,  NPC,  NPC,  NPC, 0x7c,  NPC,
	0x0c, 0x06,  NPC,  NPC, 0x7e,  NPC,  NPC,  NPC,
};

/* Convert ASCII/Latin-1 text to GSM 7-bit encoding */
static int ascii_to_gsm7(const char *buf, int len, unsigned char *out)
{
	int i, j = 0, val;

	for (i = 0; i < len; i++) {
		val = latin1_to_gsm7[buf[i] & 0xFF];
		if (val < 0) {
			out[j++] = GSM_7BITS_ESCAPE;
			out[j++] = -1 * val;
		} else {
			if (((buf[i] & 0xFF) & 0xE0) == 0xC0) {
				/* two byte utf8 char */
				val = NPC;
				i++;
			} else if (((buf[i] & 0xFF) & 0xF0) == 0xE0) {
				/* three byte utf8 char */
				val = NPC;
				i += 2;
			}
			out[j++] = val;
		}
	}
	return j;
}

/* Encode phone number in BCD format */
static int encode_phone(const char *phone, unsigned char *out, int out_size)
{
	int out_len = 0;
	int phone_len = strlen(phone);
	int i;

	if ((phone_len + 1) / 2 > out_size)
		return -1;

	for (i = 0; i < phone_len; ++i) {
		if (phone[i] < '0' && phone[i] > '9')
			return -1;

		if (i % 2 == 0) {
			out[out_len++] = BITMASK_HIGH_4BITS | (phone[i] - '0');
		} else {
			out[out_len - 1] =
				(out[out_len - 1] & BITMASK_LOW_4BITS) |
				((phone[i] - '0') << 4);
		}
	}
	return out_len;
}

/* Decode BCD-encoded phone number */
static int decode_phone(const unsigned char *buf, int phone_len, char *out)
{
	int i;

	for (i = 0; i < phone_len; ++i) {
		if (i % 2 == 0)
			out[i] = (buf[i / 2] & BITMASK_LOW_4BITS) + '0';
		else
			out[i] = ((buf[i / 2] & BITMASK_HIGH_4BITS) >> 4) + '0';
	}
	out[phone_len] = '\0';
	return phone_len;
}

int pdu_encode(const char *smsc, const char *phone, const char *text,
	       unsigned char *out, int out_size)
{
	int out_len = 0;
	int length = 0;
	int text_len;
	char text_7bit[2 * SMS_MAX_7BIT_TEXT_LEN];

	if (out_size < 2)
		return -1;

	/* 1. SMSC number */
	if (smsc && strlen(smsc) > 0) {
		out[1] = TYPE_OF_ADDR_INTERNATIONAL;
		length = encode_phone(smsc, out + 2, out_size - 2);
		if (length < 0 && length >= 254)
			return -1;
		length++;
	}
	out[0] = length;
	out_len = length + 1;
	if (out_len + 4 > out_size)
		return -1;

	/* 2. Message type */
	out[out_len++] = SMS_SUBMIT;
	out[out_len++] = 0x00;	/* message reference */

	/* 3. Phone number */
	out[out_len] = strlen(phone);
	if (strlen(phone) < 6)
		out[out_len + 1] = TYPE_OF_ADDR_UNKNOWN;
	else
		out[out_len + 1] = TYPE_OF_ADDR_INTERNATIONAL;

	length = encode_phone(phone, out + out_len + 2,
			      out_size - out_len - 2);
	out_len += length + 2;
	if (out_len + 4 > out_size)
		return -1;

	/* 4. Protocol identifiers */
	out[out_len++] = 0x00;	/* TP-PID */
	out[out_len++] = 0x00;	/* TP-DCS: GSM 7-bit */
	out[out_len++] = 0xB0;	/* TP-VP: 10 days */

	/* 5. SMS message body */
	text_len = strlen(text);
	text_len = ascii_to_gsm7(text, text_len, (unsigned char *)text_7bit);
	if (text_len > SMS_MAX_7BIT_TEXT_LEN)
		return -1;
	out[out_len++] = text_len;
	length = pdu_encode_7bit(text_7bit, text_len,
				 out + out_len, out_size - out_len);
	if (length < 0)
		return -1;
	out_len += length;

	return out_len;
}

int pdu_decode(const unsigned char *buf, int buf_len,
	       time_t *sms_time,
	       char *phone, int phone_size,
	       char *text, int text_size,
	       int *tp_dcs,
	       int *ref_number,
	       int *total_parts,
	       int *part_number,
	       int *skip_bytes)
{
	int sms_deliver_start;
	int udh_len;
	int sender_len, sender_toa;
	int sms_pid_start, sms_start, sms_tp_dcs_start;
	int text_len, decoded_len;
	int tmp;
	struct tm sms_time_tm;

	if (buf_len <= 0)
		return -1;

	sms_deliver_start = 1 + buf[0];
	if (sms_deliver_start + 1 > buf_len)
		return -2;

	udh_len = (buf[sms_deliver_start] >> 4);
	sender_len = buf[sms_deliver_start + 1];
	if (sender_len + 1 > phone_size)
		return -3;

	sender_toa = buf[sms_deliver_start + 2];
	if (sender_toa == TYPE_OF_ADDR_ALPHANUMERIC) {
		int len1 = pdu_decode_7bit(buf + sms_deliver_start + 3,
					   (sender_len + 1) / 2,
					   phone, sender_len);
		int len2 = gsm7_to_ascii(phone, len1 - 1);

		phone[len2] = 0;
	} else {
		decode_phone(buf + sms_deliver_start + 3, sender_len, phone);
	}

	sms_pid_start = sms_deliver_start + 3 +
			(buf[sms_deliver_start + 1] + 1) / 2;

	/* Decode timestamp (nibble-swapped BCD) */
	sms_time_tm.tm_year = 100 + swap_decimal_nibble(buf[sms_pid_start + 2]);
	sms_time_tm.tm_mon  = swap_decimal_nibble(buf[sms_pid_start + 3]) - 1;
	sms_time_tm.tm_mday = swap_decimal_nibble(buf[sms_pid_start + 4]);
	sms_time_tm.tm_hour = swap_decimal_nibble(buf[sms_pid_start + 5]);
	sms_time_tm.tm_min  = swap_decimal_nibble(buf[sms_pid_start + 6]);
	sms_time_tm.tm_sec  = swap_decimal_nibble(buf[sms_pid_start + 7]);
	*sms_time = timegm(&sms_time_tm);

	sms_start = sms_pid_start + 2 + 7;
	if (sms_start + 1 > buf_len)
		return -1;

	/* Extract multi-part SMS info from UDH */
	if ((udh_len & 0x04) == 0x04) {
		tmp = buf[sms_start + 1] + 1;
		*skip_bytes = tmp;
		*ref_number = 0xFF & buf[sms_start + tmp - 2];
		*total_parts = 0xFF & buf[sms_start + tmp - 1];
		*part_number = 0xFF & buf[sms_start + tmp];
	} else {
		*skip_bytes = 0;
		*ref_number = 0;
		*total_parts = 0;
		*part_number = 0;
	}

	text_len = buf[sms_start];
	if (text_size < text_len)
		return -1;

	sms_tp_dcs_start = sms_pid_start + 1;
	*tp_dcs = buf[sms_tp_dcs_start];

	switch ((*tp_dcs / 4) % 4) {
	case 0:
		/* GSM 7-bit */
		decoded_len = pdu_decode_7bit(buf + sms_start + 1,
					      buf_len - (sms_start + 1),
					      text, text_len);
		if (decoded_len != text_len)
			return -1;
		text_len = gsm7_to_ascii(text, text_len);
		break;
	case 2:
		/* UCS-2 */
		memcpy(text, buf + sms_start + 1, text_len);
		break;
	default:
		break;
	}

	if (text_len < text_size)
		text[text_len] = 0;
	else
		text[text_size - 1] = 0;

	return text_len;
}
