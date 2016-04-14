  #include "gsm.h"

char *xstrdup(char *str)
{
  int len = strlen(str);
  return xstrndup(str, len);
}

char *xstrndup(char *str, int size)
{
  int c, len;
  c = 0;
  len = strlen(str);
  char *ret = malloc(size + 1);

  if (ret == NULL)
    return NULL;

  strncpy(ret, str, size + 1);
  ret[size] = '\0';
  // printf("Result: %s\n", ret);

  return ret;
}

void printbytes(char *buf, int len)
{
  for (int i = 0; i < len; i++)
    if (buf[i] == '\r' || buf[i] == '\n')
      printf("\n");
    else
      printf("%d ", buf[i]);
  printf("\n--------------------------------------\n");
}

int xfind(char *str, const char *substr)
{
  // printf("String to look through: %s\n", str);
  int fullstrlen, substrlen, pos, insidequote;
  pos = 0;
  insidequote = 0;
  fullstrlen = strlen(str);
  substrlen = strlen(substr);

  for (int i = 0; i < fullstrlen; i++)
  {
    if (!insidequote && str[i] == '\"')
      insidequote = 1;
    else if (insidequote && str[i] == '\"')
      insidequote = 0;

    if (!insidequote)
    {
      if (strncmp(str + i, substr, substrlen) == 0)
        return i;
    }
  }

  return -1;
}

char *xstrtok(char *str, const char *delim, char **ptr)
{
  char *ret = NULL;
  int len, dlen, i, j, pos;

  // Reset: New Token
  if (str != NULL)
    *ptr = str;

  len = strlen(*ptr);
  dlen = strlen(delim);

  if ((pos = xfind(*ptr, delim)) != -1)
  {
    for (j = 0; j < dlen; j++)
      *(*ptr + pos + j) = '\0';

    if (ptr != NULL)
    {
      ret = *ptr;
      *ptr = *ptr + pos + dlen;
    }
  }
  else if (pos == -1 && len > 0)
  {
    ret = *ptr;
    *ptr = *ptr + len;
  }

  return ret;
}

static int gsm_read(gsm_t *gsm, char *buf, int len)
{
  int nb, pos, resp, pollret;
  char ign[2];
  struct pollfd fds[1];
  nb = 0;
  pos = 0;
  resp = 0;
  bzero(buf, len);

  fds[0].fd = gsm->fd;
  fds[0].events = POLLIN;

    pollret = poll(fds, 1, -1);
    if (pollret > 0)
    {
      if (fds[0].revents & POLLIN)
      {
        do
        {
          nb = read(gsm->fd, buf + pos, len - pos);

          if ((buf[0] == '\n' || buf[0] == '\r') && resp != 4)
          {
            resp++;
            continue;
          }
          else
          {
            resp = 4;
          }

          if (nb > 0)
            pos += nb;

          // DEBUG
          // for (int i = 0; i < pos; i++)
          // {
          //   printf("%d ", buf[i]);
          // }
          // printf("\n");
          // printf("Buf: %s, Ret: %d\n", buf, nb);
          // if (buf[pos - 2] == '\r' && buf[pos - 1] == '\n')
          //   break;
        }
        while(nb > 0);
      }
    }

  // printf("POS: %d\n", pos);

  if (buf[pos - 2] == '\r' && buf[pos - 1] == '\n')
    buf[pos - 2] = '\0';

  // Length
  return pos - 1;
}

gsm_t * gsm_open(const char *devnode, int baudrate)
{
  int fd;
  struct termios tio;
  gsm_t *gsm;
  fd = open(devnode, O_RDWR | O_NOCTTY | O_NDELAY);

  // Zero out term settings
  bzero(&tio, sizeof(struct termios));

  if (fd < 0) return NULL;

  fcntl(fd, F_SETFL, O_NONBLOCK);

  tio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
  tio.c_iflag = IGNPAR;
  tio.c_oflag = 0;
  tio.c_lflag = ICANON;

  tio.c_cc[VINTR]    = 0;     /* Ctrl-c */
  tio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
  tio.c_cc[VERASE]   = 0;     /* del */
  tio.c_cc[VKILL]    = 0;     /* @ */
  tio.c_cc[VEOF]     = 4;     /* Ctrl-d */
  tio.c_cc[VTIME]    = 5;     /* inter-character timer unused */
  tio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
  tio.c_cc[VSWTC]    = 0;     /* '\0' */
  tio.c_cc[VSTART]   = 0;     /* Ctrl-q */
  tio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
  tio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
  tio.c_cc[VEOL]     = 0;     /* '\0' */
  tio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
  tio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
  tio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
  tio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
  tio.c_cc[VEOL2]    = 0;     /* '\0' */

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &tio);

  gsm = malloc(sizeof(gsm_t));
  memset(gsm, 0, sizeof(gsm_t));
  gsm->devnode = devnode;
  gsm->fd = fd;

  write(fd, "ATE0\r", 5);
  sleep(1);
  write(fd, "AT^CURC=0\r", 10);
  sleep(1);
  write(fd, "AT+CMGF=1\r", 10);
  sleep(1);
  tcflush(fd,TCIOFLUSH);

  return gsm;
}

char * gsm_cmd(gsm_t *gsm, const char *cmd)
{
  int len, lenr;
  len = strlen(cmd) + 2;
  char ncmd[len], buf[256];
  char * res = NULL;

  strcpy(ncmd, cmd);
  ncmd[len - 2] = '\r';
  ncmd[len - 1] = '\0';
  tcflush(gsm->fd,TCIOFLUSH);

  // for (int i = 0; i < strlen(ncmd) + 1; i++)
  //   printf("%d ", ncmd[i]);
  // printf("\n");

  write(gsm->fd, ncmd, len);

  // printf("Actual Length: %d\n", strlen(cmd));
  // // printf("Written Length: %d\n", wlen);
  // printf("Written buffer: %s\n", ncmd);

  lenr = gsm_read(gsm, buf, sizeof(buf));
  // printf("buf: %s, len: %d\n", buf, lenr);

  res = malloc(lenr + 1);
  strncpy(res, buf, lenr + 1);

  return res;
}

int gsm_sendmsg(gsm_t *gsm, const char *number, const char *msg)
{
  int msglen = strlen(msg);
  const char beg[] = "AT+CMGS=\"";
  const char end[] = "\"\r\n";
  char nmsg[msglen + 2];
  char from[sizeof(beg) + strlen(number) + sizeof(end)];
  char ret[256];
  strcpy(from, beg);
  strcat(from, number);
  strcat(from, end);
  strcpy(nmsg, msg);
  nmsg[msglen + 1] = '\x1A';
  nmsg[msglen + 2] = '\0';

  write(gsm->fd, from, sizeof(from));
  sleep(1);
  write(gsm->fd, nmsg, sizeof(nmsg));
  sleep(10);
  gsm_read(gsm, ret, sizeof(ret));

  // Get result
  if (strcmp(ret, "> ") == 0)
    gsm_read(gsm, ret, sizeof(ret));

  return 0;
}

gsmmsg_t * gsm_readmsg(gsm_t *gsm, int type, int limit)
{
  int statcount, i, msgcount;
  i = 0;
  msgcount = 0;
  char * buf;
  const char *cmdstart = "AT+CMGL=\"";
  const char *gsm_type;
  gsmmsg_t * msgs = malloc(sizeof(*msgs)*limit);
  char *lineptr, *stats, *timestmpptr, *tok, *stattok, *timestmptok;

  // Memset
  // memset(msgs, 0, sizeof(gsmmsg_t) * limit);
  for (i = 0; i < limit; i++)
  {
    msgs[i].orig = NULL;
    msgs[i].origname = NULL;
    msgs[i].msg = NULL;
  }

  switch (type)
  {
    case UNREAD:
    gsm_type = "REC UNREAD";
    break;
    case READ:
    gsm_type = "REC READ";
    break;
    case UNSENT:
    gsm_type = "STO UNSENT";
    break;
    default:
    case ALL:
    gsm_type = "ALL";
    break;
  }

  char cmd[strlen(cmdstart) + strlen(gsm_type) + 2];
  strcpy(cmd, cmdstart);
  strcat(cmd, gsm_type);
  strcat(cmd, "\"");

  buf = gsm_cmd(gsm, cmd);

  // First line
  tok = xstrtok(buf, "\r\n", &lineptr);

  // Until we reached end of tokens or OK
  while (tok != NULL && tok[0] != 'O' && tok[1] != 'K')
  {
    // printf("Tok: %s\n", tok);
    if (msgcount < limit)
    {
      if (strncmp(tok, "+CMGL: ", 7) == 0)
      {
        statcount = 0;
        // printf("line: %s\n", tok+7);
        // printf("Msg Count: %d\n", msgcount);
        stattok = xstrtok(tok + 7, ",", &stats);
        while (stattok != NULL)
        {
          int statlen = strlen(stattok);
          if (stattok[0] != '\0')
          {
            if (statcount == MSG_INDEX)
              msgs[msgcount].index = atoi(stattok);

            if (statcount == MSG_STAT && (strcmp(stattok, gsm_type) == 0 || type == ALL))
              msgs[msgcount].state = type;

            if (statcount == MSG_ORIGADDR)
              msgs[msgcount].orig = xstrndup(stattok+1,statlen-2);

            if (statcount == MSG_ORIGNAME)
              msgs[msgcount].origname = xstrndup(stattok+1, statlen-2);

            if (statcount == MSG_TIMESTMP)
            {
              int fullsecs, year, month, day, hour, min, sec, gmtdiff;
              char *dateptr, *timeptr;
              // TODO Parse timestamp
              char *tmpdate = xstrndup(stattok+1, statlen-2);
              // printf("Date: %s\n", stattok);

              // Calculate the number of years since 1900 according to current date
              fullsecs = time(NULL);
              min = fullsecs/60;
              hour = min/60;
              day = hour/24;
              month = day /30;
              year = (70 + month/12)/100;

              // printf("full: %d, min: %d, hour: %d, day: %d, month: %d, year: %d\n", fullsecs, min, hour, day, month, year);

              // Date
              timestmptok = xstrtok(tmpdate, ",", &timestmpptr);

              // Get year
              timestmptok = xstrtok(timestmptok, "/", &dateptr);
              year = 100*year + atoi(timestmptok);
              // printf("Year: %d\n", year);

              // Get month
              timestmptok = xstrtok(NULL, "/", &dateptr);
              month = atoi(timestmptok);
              // printf("Month: %d\n", month);

              // Get day
              timestmptok = xstrtok(NULL, "/", &dateptr);
              day = atoi(timestmptok);
              // printf("Day: %d\n", day);

              // Get time
              timestmptok = xstrtok(NULL, ",", &timestmpptr);

              // printf("Time: %s\n", timestmptok);

              // Get hour
              timestmptok = xstrtok(timestmptok, ":", &timeptr);
              hour = atoi(timestmptok);
              // printf("Hour: %d\n", hour);

              timestmptok = xstrtok(NULL, ":", &timeptr);
              min = atoi(timestmptok);
              // printf("Min: %d\n", min);

              timestmptok = xstrtok(NULL, ":", &timeptr);
              char timediff = timestmptok[2];
              timestmptok[2] = '\0';
              sec = atoi(timestmptok);
              // printf("Sec: %d\n", sec);

              // calculate time difference
              timestmptok+= 3;
              gmtdiff = atoi(timestmptok) / 4;
              // printf("Time difference: %c%d\n", timediff, gmtdiff);

              if (timediff == '-')
              {
                hour += gmtdiff;
                if (hour >= 24)
                {
                  hour = hour - 24;
                  day += 1;
                }
              }
              else if (timediff == '+')
              {
                hour -= gmtdiff;
                if (hour < 0)
                {
                  hour = 24 + hour;
                  day -= 1;
                }
              }

              msgs[msgcount].datetime.tm_sec = sec;
              msgs[msgcount].datetime.tm_min = min;
              msgs[msgcount].datetime.tm_hour = hour;
              msgs[msgcount].datetime.tm_mday = day;
              msgs[msgcount].datetime.tm_mon = month;
              msgs[msgcount].datetime.tm_year = year;
              msgs[msgcount].datetime.tm_isdst = -1;

              printf("DateTime: %s\n", asctime(&msgs[msgcount].datetime));

              free(tmpdate);
            }
          }
          statcount++;
          stattok = xstrtok(NULL, ",", &stats);
        }
      }
      else
      {
        // printf("line: %s\n", tok);
        // printf("MSG Count: %d\n", msgcount);
        msgs[msgcount].msg = xstrdup(tok);
        msgcount++;
      }
    }

    // Next line
    tok = xstrtok(NULL, "\r\n", &lineptr);
  }

  free(buf);

  if (msgcount > 0)
    return msgs;

  gsm_freemsgs(msgs, limit);

  return NULL;
}

gsmmsg_t * gsm_readlastunread(gsm_t *gsm)
{
  gsmmsg_t *msg = NULL;
  msg = gsm_readmsg(gsm, UNREAD, 1);

  return msg;
}

void gsm_freemsgs(gsmmsg_t * msgs, int n)
{
  for (int i = 0; i < n; i++)
  {
    if (msgs[i].orig != NULL)
      free(msgs[i].orig);

    if (msgs[i].origname != NULL)
      free(msgs[i].origname);

    if (msgs[i].msg != NULL)
      free(msgs[i].msg);
  }
  free(msgs);
}

void    gsm_freemsg(gsmmsg_t *msg)
{
  gsm_freemsgs(msg, 1);
}

void gsm_close(gsm_t *gsm)
{
  close(gsm->fd);
  free(gsm);
}
