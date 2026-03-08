/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * qcseriald — macOS USB-to-serial bridge daemon for Qualcomm modems
 *
 * Ported from qcseriald-darwin by iamromulan
 * https://github.com/iamromulan/qcseriald-darwin
 */
#ifndef __QCSERIALD_H__
#define __QCSERIALD_H__

#ifdef HAVE_QCSERIALD

/*
 * Main dispatcher for "qfenix qcseriald <subcommand>".
 * Handles start, stop, restart, status, log subcommands.
 */
int qdl_qcseriald(int argc, char **argv);

/*
 * Detect the DIAG port path via qcseriald status file.
 * Returns 1 if found (path written to buf), 0 otherwise.
 */
int qcseriald_detect_diag_port(char *buf, size_t size);

/*
 * Detect an AT port path via qcseriald status file.
 * index selects which AT port (0 = at0, 1 = at1, etc).
 * Returns 1 if found (path written to buf), 0 otherwise.
 */
int qcseriald_detect_at_port(char *buf, size_t size, int index);

/*
 * Check if qcseriald daemon is currently running.
 * Returns 1 if running, 0 if not.
 */
int qcseriald_is_running(void);

/*
 * Ensure qcseriald is running. If not, start the daemon and wait
 * up to 35s for port probing to complete (covers the 30s RDY
 * timeout on cold boot).
 * Returns 0 on success, -1 on failure.
 */
int qcseriald_ensure_running(void);

/*
 * Wait for a specific port to become available via qcseriald.
 * Starts daemon if needed, polls status file with user-friendly output.
 * port_type: "diag", "at0", "at1", etc.
 * Returns 1 if found (path written to buf), 0 on failure.
 */
int qcseriald_wait_for_port(const char *port_type, char *buf, size_t size);

/*
 * List all ports known to qcseriald for the list command.
 * Reads daemon status file, prints DIAG/AT/NMEA ports.
 * Returns number of ports found.
 */
int qcseriald_list_ports(FILE *out);

/* Print help text for qcseriald subcommand */
void print_qcseriald_help(FILE *out);

#endif /* HAVE_QCSERIALD */
#endif /* __QCSERIALD_H__ */
