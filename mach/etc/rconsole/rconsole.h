/*
 ***********************************************************************
 *	File: rconsole.h
 *
 ***********************************************************************
 * HISTORY
 * 22-Sep-86  William Bolosky (bolosky) at Carnegie-Mellon University
 *	Changed CONSOLE_FILE to /usr/mach/lib/rconsoles.
 *
 ***********************************************************************
 */

#define CONSOLE_FILE	"/usr/mach/lib/rconsoles"
#define LOG_DIR		"/usr/rconsole/"
#define RCONSOLE_ID	"rconsole"

#define LOG_LIMIT	250000
#define LOG_MODE	0664
#define ESC_CHAR	'\037'
#define	OOB_CHAR	0xc5
#define		OOB_TERM	0x81
#define		OOB_BRK		0x82
#define		OOB_DEBUG	0x83
#define		OOB_LOG		0x84
#define TIME_LIMIT	10
#define LINE_LENGTH	256
#define NUM_ALIASES	10
#define ALIAS_LENGTH	20

#define max(a, b)	(((a) > (b)) ? (a) : (b))
