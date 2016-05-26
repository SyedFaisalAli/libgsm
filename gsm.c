#include "gsm.h"

// Expect s1 to be malloc
int xstrcmp(char *s1, const char *s2, int frees1)
{
  int ret = strcmp(s1, s2);

  if (frees1)
    free(s1);

  return ret;
}

int septlen(int origsize)
{
  if (origsize / 7 == 1)
    return origsize;
  else
    return origsize - (origsize / 7);
}

char *xstrdup(char *str)
{
  int len = strlen(str);
  return xstrndup(str, len);
}

char * str2nhex(char *dest, char *src, int n)
{
  char *res, *ptr;
  res = NULL;
  ptr = NULL;

  if (dest == NULL)
  {
    res = malloc(n+1);
    ptr = res;
  }
  else
  {
    ptr = dest;
    res = dest;
  }

  for (int i = 0; i < n; i++)
  {
    sprintf(ptr, "%02X", (uint8_t)src[i]);
    ptr += 2;
  }

  return res;
}

char * str2hex(char *dest, char *src)
{
  return str2nhex(dest, src, strlen(src));
}

int pdu_pack(char *dest, const char *src, int udhpadding)
{
  return pdu_npack(dest, src, udhpadding, strlen(src));
}

int pdu_npack(char *dest, const char *src, int udhpadding, int n)
{
  int i, mbits, rlen;
  uint8_t octet;
  char *ptr;
  const char *srcptr;
  mbits = 0; // # Of bites to shift
  rlen = 0; // # 7-bit length

  ptr = dest;
  srcptr = src;

  if (udhpadding > 0)
  {
    mbits = 7 - udhpadding;
    octet = *srcptr << (7 - mbits);
    *ptr++ = octet;
    mbits++;
    rlen++;
  }

  // Iterate through each byte
  for (i = 0; i < n; i++)
  {
    if (mbits == 7)
    {
      mbits = 0;
      continue;
    }

    octet = (*(srcptr + i) & 0x7F) >> mbits;

    if (i < n - 1)
      octet |= *(srcptr + i + 1) << (7 - mbits);

    *ptr++ = octet;

    mbits++;
    rlen++;
  }

  // *(ptr-1) = '\0';

  return rlen;
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

int gsm_numresults(char *buf, const char *cmdsuff)
{
  int numres, cmdsufflen;
  numres = 0;
  cmdsufflen = strlen(cmdsuff);
  char *ptr, *dup, *tok;
  dup = strdup(buf);

  tok = xstrtok(dup, "\r\n", &ptr);
  while (tok != NULL)
  {
    if (tok[0] == '+' && strncmp(tok+1, cmdsuff, cmdsufflen) == 0)
      numres++;

    tok = xstrtok(NULL, "\r\n", &ptr);
  }

  free(dup);

  return numres;
}

int gsm_rssi(gsm_t *gsm)
{
  char *csq_res = gsm_cmd(gsm, "AT+CSQ");
  int len, num_rssi_digits;
  len = strlen(csq_res+6);
  num_rssi_digits = len - strlen(strchr(csq_res, ','));
  char rssi_str[3]; // Largest number for RSSI is 99 (unknown) w/null char

  // Strip uneeded information
  strncpy(rssi_str, csq_res+6, num_rssi_digits);
  rssi_str[num_rssi_digits] = '\0';
  free(csq_res);

  return atoi(rssi_str);
}

// Returns number of "bars" according to AT&T formula
int att_bars(gsm_t *gsm)
{
  int rssi = gsm_rssi(gsm);

  if (rssi == 99)
    return -1; // unknown
  else if (rssi == 1)
    return 0;
  else if (rssi > 1 && rssi < 6)
    return 1;
  else if (rssi >= 6 && rssi < 10)
    return 2;
  else if (rssi >= 10 && rssi < 15)
    return 3;
  else if (rssi >= 15 && rssi < 20)
    return 4;
  else if (rssi >= 20)
    return 5;
}

int gsm_readall(gsm_t *gsm, char **buf)
{
  int initialSize, currSize, read;
  initialSize = 1024;
  currSize = 0;
  read = 0;

  if (*buf == NULL)
    *buf = malloc(initialSize);

  while ((read = gsm_read(gsm, *buf + currSize, initialSize - currSize)) > 0)
  {
    currSize += read;

    if (currSize >= initialSize - 1)
    {
      initialSize *= 2;
      *buf = realloc(*buf, initialSize);
    }
  }

  return currSize;
}

int gsm_read(gsm_t *gsm, char *buf, int len)
{
  int nb, pos, resp, pollret;
  char ign[2];
  struct pollfd fds[1];
  int initalnewlines = 0;
  nb = 0;
  pos = 0;
  resp = 0;
  bzero(buf, len);

  fds[0].fd = gsm->fd;
  fds[0].events = POLLIN;

  do
  {
    pollret = poll(fds, 1, READ_TIMEOUT);

    if (fds[0].revents & POLLIN)
    {
      nb = read(gsm->fd, buf + pos, 1);

      if ((*(buf + pos) == '\r' || *(buf + pos) == '\n') && !initalnewlines)
        continue;
      else
        initalnewlines = 1;

      if (nb > 0)
      {
        // printf("Char code: %d\n", buf[pos]);
        pos += nb;
      }

      if (*(buf + pos - 4) == 'O' && *(buf + pos - 3) == 'K' && *(buf + pos - 2) == 13 && *(buf + pos - 1) == 10)
      {
        *(buf + pos - 2) = 0;
        break;
      }
    }
  }
  while(pos < len && pollret > 0);

  return pos;
}

gsm_t * gsm_open(const char *devnode, int baudrate)
{
  int fd;
  struct termios tio;
  gsm_t *gsm = NULL;
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
  write(fd, "AT+CMGF=0\r", 10);
  sleep(1);
  tcflush(fd,TCIOFLUSH);

  return gsm;
}

int gsm_deletemsg(gsm_t *gsm, gsmmsg_t *msg)
{
  char cmgdcmd[32];
  sprintf(cmgdcmd, "AT+CMGD=%d", msg->index);

  if (CMD_OK(gsm, cmgdcmd))
    return 1;

  return 0;
}

char * gsm_cmd(gsm_t *gsm, const char *cmd)
{
  int len, lenr;
  len = strlen(cmd) + 2;
  char ncmd[len];
  char buf[512];

  strcpy(ncmd, cmd);
  ncmd[len - 2] = '\r';
  ncmd[len - 1] = '\0';
  tcflush(gsm->fd,TCIOFLUSH);

  write(gsm->fd, ncmd, len);

  lenr = gsm_read(gsm, buf, sizeof(buf));
  // printf("Buf2: %s\n", buf);

  return strdup(buf);
}

int gsm_sendmsgpdu(gsm_t *gsm, const char *number, char *msg)
{
  char * gsm_pdu_mode_res;
  int ret, i, dest_len, msg_read_len, tpdu_len, msg_body_len, msg_len, msg_pos, msg_read, msg_enc_hex_len;
  uint8_t msg_id, num_msgs;
  static uint8_t msg_ref = 0;
  msg_len = strlen(msg);
  msg_enc_hex_len = septlen(msg_len)*2;
  num_msgs = (msg_len / MAX_SMS_LEN) + 1;
  msg_id = 1;
  dest_len = strlen(number) + 1; // With padding bits

  char msg_cpy[MAX_SMS_LEN];
  char msg_encoded[MAX_SMS_BYTES];
  char msg_encoded_hex[MAX_SMS_BYTES*2];

  tpdu_len = 0;
  msg_read = 0;

  // We use the SMSC that the SIM card automatically sets
  char smsc[] = "00";
  char tpdu_type[] = "01";
  char msgref[] = "00";
  char destlen_hex[3]; // 11 characters long for dest #
  char desttype[] = "91"; // International format
  char dest[17]; // Maximum phone number length (15) + padding (1) + null byte.
  char protocol[] = "00";
  char dataencoding[] = "00"; // 7-bit encoding
  char udlh[] = "05";
  char udh[] = "0003a10302";
  char msgpart[MAX_SMS_BYTES*2];
  char pdustr[38 + MAX_SMS_BYTES*2];
  char lenstr[4];

  // Set phone number length in HEX
  sprintf(destlen_hex, "%02X", dest_len);
  // Reference
  snprintf(udh+4, 3, "%02X", msg_ref++);
  // Total Messages
  snprintf(udh+6, 3, "%02X", num_msgs);
  // Current message
  snprintf(udh+8, 3, "%02X", msg_id++);
  // Copy number to destination
  strcpy(dest, number);
  // Append padding since destination length is at least 11 characters long
  strcat(dest, "F");

  // Convert the phone # into PDU format
  for (i = 0; i < dest_len; i+=2)
  {
    // First and second character holders
    char f,s;

    if (i + 1 < dest_len)
    {
      // Store characters
      f = dest[i];
      s = dest[i+1];

      // Swap
      dest[i] = s;
      dest[i+1] = f;
    }
  }
  dest[dest_len] = '\0';


  if (msg_len > MAX_SMS_LEN)
  {
    // UDH header exists
    tpdu_type[0] = '4';

    // Break down the message into multiple SMS
    while (msg_read != msg_len)
    {
      // SMS needs to be split into multipart messages
      if (msg_len - msg_read > MAX_SMS_LEN)
      {
        strncpy(msg_cpy, msg+msg_read, MAX_SMS_LEN - 7);
        msg_cpy[MAX_SMS_LEN - 7] = '\0';
        msg_read += MAX_SMS_LEN - 7;
        msg_read_len = MAX_SMS_LEN;
      }
      else
      {
        strcpy(msg_cpy, msg+msg_read);
        msg_read_len = strlen(msg_cpy) + 7;
        msg_read += msg_read_len - 7;
      }

      msg_body_len = pdu_pack(msg_encoded, msg_cpy, 1);
      tpdu_len = sprintf(pdustr, "%s%s%s%s%s%s%s%s%02X%s%s%s", smsc, tpdu_type, msgref, destlen_hex, desttype, dest, protocol, dataencoding, msg_read_len, udlh, udh, str2hex(msgpart, msg_encoded));
      tpdu_len = (tpdu_len-2)/2;
      sprintf(lenstr, "%d", tpdu_len);
      snprintf(udh+8, 3, "%02X", msg_id++);
      printf("Sending PDU Str: %s, Len: %d\n", pdustr, tpdu_len);
      ret = gsm_sendmsg(gsm, lenstr, pdustr, 1);

      // Error
      if (ret > 0)
        return ret;
    }
  }
  else
  {
    // UDH header not necessary
    msg_body_len = pdu_pack(msg_encoded, msg, 0);
    tpdu_len = sprintf(pdustr, "%s%s%s%s%s%s%s%s%02X%s", smsc, tpdu_type, msgref, destlen_hex, desttype, dest, protocol, dataencoding, msg_len, str2hex(msgpart, msg_encoded));
    tpdu_len = (tpdu_len-2)/2;
    sprintf(lenstr, "%d", tpdu_len);
    return gsm_sendmsg(gsm, lenstr, pdustr, 1);
  }

  return 0;
}

int gsm_sendmsgtext(gsm_t *gsm, const char *number, char *msg)
{
  return gsm_sendmsg(gsm, number, msg, 0);
}

int gsm_sendmsg(gsm_t *gsm, const char *number, const char *msg, int pdu)
{
  int msglen = strlen(msg);
  int retlen = 0;
  const char beg[] = "AT+CMGS=";
  const char end[] = "\r";
  char nmsg[msglen + 2];
  char from[sizeof(beg) + (!pdu % 2) + strlen(number) + (!pdu % 2) + sizeof(end)];
  char ret[256];
  char *buf, *bufptr;
  // buf = NULL;

  // Create SMS mode command
  char sms_mode_cmd[10];
  sprintf(sms_mode_cmd, "AT+CMGF=%d", !pdu % 2);

  if (CMD_OK(gsm, sms_mode_cmd) && gsm_rssi(gsm) != 99)
  {
    strcpy(from, beg);

    // Beginning quotes for text mode
    if (!pdu)
      strcat(from, "\"");

    strcat(from, number);

    // End quotes for text mode
    if (!pdu)
      strcat(from, "\"");

    strcat(from, end);
    strcpy(nmsg, msg);
    nmsg[msglen + 1] = '\x1A';
    nmsg[msglen + 2] = '\0';

    write(gsm->fd, from, sizeof(from));
    write(gsm->fd, nmsg, sizeof(nmsg));
    sleep(1);
    retlen = gsm_read(gsm, ret, sizeof(ret));
    // retlen = gsm_readall(gsm, &buf);
    // bufptr = buf;
    // printf("Buf: %s, Len: %d\n", ret, retlen);

    // Get result
    if (ret[0] == '>' && retlen <= 4)
    {
      sleep(5);
      retlen = gsm_read(gsm, ret, sizeof(ret));
    }

    printf("Buf: %s, Len: %d\n", ret, retlen);

    // Error
    if (ret[retlen - 2] != 'O' && ret[retlen - 1] != 'K')
    {
      char * cmserr = strchr(ret, '+');
      if (strncmp(cmserr, "+CMS ERROR: ", 12) == 0)
        return atoi(bufptr + 12);
    }
  }
  else
  {
    return -1;
  }

  return 0;
}

gsmmsg_t * gsm_readmsg(gsm_t *gsm, int type, int *nummsgs)
{
  int statcount, i, j, msgcount;
  i = 0;
  j = 0;
  msgcount = 0;
  char * buf;
  const char *cmdstart = "AT+CMGL=\"";
  const char *gsm_type;
  gsmmsg_t * msgs = NULL;
  char *lineptr, *stats, *timestmpptr, *tok, *stattok, *timestmptok;

  // Text mode only
  if (CMD_OK(gsm, "AT+CMGF=1"))
  {
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

      // printf("Buffer: %s\n", buf) ;

      // Get number of results from buffer
      msgcount = gsm_numresults(buf, "CMGL");
      // printf("Msg count: %d\n", msgcount);

      // NO messages;
      if (msgcount == 0)
      {
        free(buf);
        return NULL;
      }

      msgs = malloc(sizeof(gsmmsg_t)*msgcount);

      for (i = 0; i < msgcount; i++)
      {
        msgs[i].orig = NULL;
        msgs[i].origname = NULL;
        msgs[i].msg = NULL;
      }

      // printf("Buffer: %s\n", buf) ;

      // First line
      tok = xstrtok(buf, "\r\n", &lineptr);

      // Until we reached end of tokens or OK
      while (tok != NULL && tok[0] != 'O' && tok[1] != 'K')
      {
        if (strncmp(tok, "+CMGL: ", 7) == 0)
        {
          statcount = 0;
          stattok = xstrtok(tok + 7, ",", &stats);
          while (stattok != NULL)
          {
            // printf("Tok: %s\n", stattok);
            int statlen = strlen(stattok);
            if (stattok[0] != '\0')
            {
              if (statcount == MSG_INDEX)
                msgs[j].index = atoi(stattok);

              if (statcount == MSG_STAT && (strcmp(stattok, gsm_type) == 0 || type == ALL))
                msgs[j].state = type;

              if (statcount == MSG_ORIGADDR)
              {
                if (stattok[1] == '+')
                  msgs[j].orig = xstrndup(stattok+2,statlen-3);
                else
                  msgs[j].orig = xstrndup(stattok+1,statlen-2);

                // printf("Orig: %s\n", msgs[j].orig);
              }

              if (statcount == MSG_ORIGNAME)
                msgs[j].origname = xstrndup(stattok+1, statlen-2);

              if (statcount == MSG_TIMESTMP)
              {
                int fullsecs, year, month, day, hour, min, sec, gmtdiff;
                char *dateptr, *timeptr;
                // TODO Parse timestamp
                char *tmpdate = xstrndup(stattok+1, statlen-2);

                // Calculate the number of years since 1900 according to current date
                fullsecs = time(NULL);
                min = fullsecs/60;
                hour = min/60;
                day = hour/24;
                month = day /30;
                year = (70 + month/12)/100;

                // Date
                timestmptok = xstrtok(tmpdate, ",", &timestmpptr);

                // Get year
                timestmptok = xstrtok(timestmptok, "/", &dateptr);
                year = 100*year + atoi(timestmptok);

                // Get month
                timestmptok = xstrtok(NULL, "/", &dateptr);
                month = atoi(timestmptok);

                // Get day
                timestmptok = xstrtok(NULL, "/", &dateptr);
                day = atoi(timestmptok);

                // Get time
                timestmptok = xstrtok(NULL, ",", &timestmpptr);

                // Get hour
                timestmptok = xstrtok(timestmptok, ":", &timeptr);
                hour = atoi(timestmptok);

                timestmptok = xstrtok(NULL, ":", &timeptr);
                min = atoi(timestmptok);

                timestmptok = xstrtok(NULL, ":", &timeptr);
                char timediff = timestmptok[2];
                timestmptok[2] = '\0';
                sec = atoi(timestmptok);

                // calculate time difference
                timestmptok+= 3;
                gmtdiff = atoi(timestmptok) / 4;

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

                msgs[j].datetime.tm_sec = sec;
                msgs[j].datetime.tm_min = min;
                msgs[j].datetime.tm_hour = hour;
                msgs[j].datetime.tm_mday = day;
                msgs[j].datetime.tm_mon = month;
                msgs[j].datetime.tm_year = year;
                msgs[j].datetime.tm_isdst = -1;

                free(tmpdate);
              }
            }
            statcount++;
            stattok = xstrtok(NULL, ",", &stats);
          }
        }
        else
        {
            // printf("Tok: %s\n", tok);
            if (tok[0] != '\0')
            {
              msgs[j].msg = xstrdup(tok);
              j++;
            }
        }

      // Next line
      tok = xstrtok(NULL, "\r\n", &lineptr);
    }

    free(buf);
  }

  if (msgcount > 0)
  {
    if (nummsgs != NULL)
      *nummsgs = msgcount;

    return msgs;
  }

  gsm_freemsgs(msgs, msgcount);

  return NULL;
}

gsmmsg_t * gsm_readlastunread(gsm_t *gsm)
{
  gsmmsg_t *msg = NULL;
  msg = gsm_readmsg(gsm, UNREAD, NULL);

  return &msg[0];
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

void gsm_freemsg(gsmmsg_t *msg)
{
  gsm_freemsgs(msg, 1);
}

void gsm_close(gsm_t *gsm)
{
  close(gsm->fd);
  free(gsm);
}
