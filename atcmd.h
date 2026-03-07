/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef __ATCMD_H__
#define __ATCMD_H__

#include <stdio.h>

struct at_session {
	int  fd;
	char port[256];
	int  timeout_ms;
	int  debug;
};

/* Open an AT command session on the given serial port */
struct at_session *at_open(const char *port, int timeout_ms);

/* Close and free an AT session */
void at_close(struct at_session *sess);

/* Read a line from the modem, returns bytes read or -1 */
int at_read_line(struct at_session *sess, char *buf, size_t size);

/* Send an AT command and collect response until OK/ERROR */
int at_send_cmd(struct at_session *sess, const char *cmd,
		char *resp, size_t resp_size);

/* Top-level subcommand dispatchers */
int qdl_smssend(int argc, char **argv);
int qdl_smsread(int argc, char **argv);
int qdl_smsrm(int argc, char **argv);
int qdl_smsstatus(int argc, char **argv);
int qdl_ussd(int argc, char **argv);
int qdl_atcmd(int argc, char **argv);
int qdl_atconsole(int argc, char **argv);

/* Per-command help */
void print_smssend_help(FILE *out);
void print_smsread_help(FILE *out);
void print_smsrm_help(FILE *out);
void print_smsstatus_help(FILE *out);
void print_ussd_help(FILE *out);
void print_atcmd_help(FILE *out);
void print_atconsole_help(FILE *out);

#endif /* __ATCMD_H__ */
