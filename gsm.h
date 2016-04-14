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

/* Helper Utils */
int xfind(char *, const char *);
char *xstrtok(char *, const char *, char **);
char *xstrdup(char *);
char *xstrndup(char *, int);
void printbytes(char *, int);

/* Read into buffer until CR and returns characters read */
static int gsm_read(gsm_t *, char *, int);

gsm_t * gsm_open(const char *, int);
char *  gsm_cmd(gsm_t *, const char *);
int     gsm_sendmsg(gsm_t *, const char *, const char *);
gsmmsg_t * gsm_readmsg(gsm_t *, int, int);
gsmmsg_t * gsm_readlastunread(gsm_t *);
void    gsm_freemsgs(gsmmsg_t *, int);
void    gsm_freemsg(gsmmsg_t *);
void    gsm_close(gsm_t *);

#endif
