#ifndef H_GSM
#define H_GSM
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdint.h>

// MACROS
#define CMD_OK(gsm, cmd) xstrcmp(gsm_cmd(gsm, cmd), "OK", 1) == 0

#define TRUE   1
#define FALSE  0

#define UNREAD 0
#define READ   1
#define UNSENT 2
#define ALL    3

#define MSG_INDEX    0
#define MSG_STAT     1
#define MSG_ORIGADDR 2
#define MSG_ORIGNAME 3
#define MSG_TIMESTMP 4

#define MAX_SMS_LEN  160
#define MAX_SMS_BYTES 140
#define MAX_SMS_LEN_HEX MAX_SMS_LEN*2

typedef struct gsm
{
  const char *devnode;
  int fd;
} gsm_t;

typedef struct gsm_msg
{
  int index;
  int state;
  char *orig;
  char *origname;
  char *msg;
  struct tm datetime;
} gsmmsg_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Helper Utils */
int xfind(char *, const char *);
int xstrcmp(char *, const char *, int);
char *xstrtok(char *, const char *, char **);
char *xstrdup(char *);
char *xstrndup(char *, int);
int septlen(int);
int pdu_pack(char *, const char *, int);
int pdu_npack(char *, const char *, int, int);
char * str2nhex(char *, char *, int);
char * str2hex(char *, char *);
void printbytes(char *, int);

/* Read into buffer until CR and returns characters read */
static int gsm_read(gsm_t *, char *, int);

gsm_t * gsm_open(const char *, int);
char *  gsm_cmd(gsm_t *, const char *);
static int gsm_sendmsg(gsm_t *, const char *, const char *, int);
int     gsm_sendmsgpdu(gsm_t *, char *, char *);
int     gsm_sendmsgtext(gsm_t *, char *, char *);
gsmmsg_t * gsm_readmsg(gsm_t *, int, int);
gsmmsg_t * gsm_readlastunread(gsm_t *);
void    gsm_freemsgs(gsmmsg_t *, int);
void    gsm_freemsg(gsmmsg_t *);
void    gsm_close(gsm_t *);

#ifdef __cplusplus
}
#endif

#endif
