#include "gsm.h"
#include "minunit.h"
#include <stdio.h>

int tests_run = 0;
gsm_t *gsm;
char inputphone[16];

void print_section(char *section)
{
	printf("*********************************\n* %s\n*********************************\n", section);
}

void get_phonenum()
{
	printf("Enter country code and phone #: ");
	fgets(inputphone, 15, stdin);
	inputphone[strlen(inputphone)-1] = '\0';
	printf("\n");
}

int confirm()
{
	int res = 0;
	printf("Ready? ");
	res = fgetc(stdin);

	while (fgetc(stdin) != '\n');

	if (res == 'y' || res == 'Y')
		return 1;
	else
		return 0;
}

static char * test_at()
{
	print_section("TEST AT");
	mu_assert("Err: AT != OK", CMD_OK(gsm, "AT"));
	printf("DONE!\n");
	return 0;
}

static char * test_readmsgs()
{
	print_section("TEST MESSAGE RESULTS");
	int msgcount = 0;
	gsmmsg_t * msgs = gsm_readmsg(gsm, ALL, &msgcount);
	mu_assert("No messages in storage.", msgs != NULL && msgcount > 0);

	// Print messages
	for (int i = 0; i < msgcount; i++)
	{
		gsmmsg_t *msg = &msgs[i];
		printf("Message Index: %d\nMessage State: %d\nMessage Orig: %s\nMessage Origname: %s\nMessage Contents: %s\n\n", msg->index, msg->state, msg->orig, msg->origname, msg->msg);
	}

	gsm_freemsgs(msgs, msgcount);

	return 0;
}

static char * test_textsms()
{
	print_section("TEST TEXT SMS");
	char msg[] = "Hello?";

	get_phonenum();

	printf("Sending Text SMS with str: %s\n", msg);
	mu_assert("Err: Unable to send SMS", gsm_sendmsgtext(gsm, inputphone, msg) == 0);
	printf("DONE!\n");
	return 0;
}

static char * test_pdulongsms()
{
	print_section("TEST PDU CONCATENATED SMS");
	char msg[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.Ut enim ad minim veniam, quinostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.Duis aute irure dolor in reprehenderit in voluptate velit esse cillum doloe eu fugiat nulla pariatur.Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum";

	get_phonenum();

	printf("Sending PDU SMS with str: %s\n", msg);
	mu_assert("Err: Unable to send SMS", gsm_sendmsgpdu(gsm, inputphone, msg) == 0);
	printf("DONE!\n");
	return 0;
}

static char * test_pdushortsms()
{
	print_section("TEST PDU SHORT SMS");
	char msg[] = "This is a message below the 160 SMS character limit.";

	get_phonenum();

	printf("Sending PDU SMS with str: %s\n", msg);
	mu_assert("Err: Unable to send SMS", gsm_sendmsgpdu(gsm, inputphone, msg) == 0);
	printf("DONE!\n");
	return 0;
}

static char * test_csq()
{
	print_section("TEST RSSI READING");
	int rssi = gsm_rssi(gsm);
	mu_assert("Err: Invalid RSSI Value", rssi != 0);
	printf("DONE!\n");
	return 0;
}

static char * test_readall()
{
	print_section("TEST READ OUTPUT OF UNDETERMINED LENGTH");
	int len, numresponse;
	char *buf = NULL;
	char textmode[] = "AT+CMGF=1\r";
	char cmd[] = "AT+CMGL=\"ALL\"\r";

	if (CMD_OK(gsm, textmode))
	{
		write(gsm->fd, cmd, sizeof(cmd));
		len = gsm_readall(gsm, &buf);
		printf("--------------------------------\nResult:\n%s, len: %d\n", buf, len);
		mu_assert("Incomplete read of command output or ERROR", buf[len - 1] != 'O' && buf[len] != 'K');

		// Get number of responses
		numresponse = gsm_numresults(buf, "CMGL");
		printf("Num results: %d\n", numresponse);
		mu_assert("Number of responses is not greater than zero.", numresponse > 0);

		printf("DONE!\n");
		free(buf);
	}
	return 0;
}

static char * test_read()
{
	print_section("TEST READ OUTPUT OF BUFFER LENGTH");
	int len;
	char textmode[] = "AT+CMGF=1\r";
	char cmd[] = "AT+CMGL=\"ALL\"\r";
	char buf[2048];

	printf("Send SMS to target modem.\n");
	if (confirm())
	{
		printf("Switching to text mode\n");
		write(gsm->fd, textmode, sizeof(textmode));
		gsm_read(gsm, buf, sizeof(buf));
		printf("Sending cmd: \"%s\"\n", cmd);
		write(gsm->fd, cmd, sizeof(cmd));
		gsm_read(gsm, buf, sizeof(buf));
		printf("------------------------------\nResult:\n\"%s\"\n", buf);
		len = strlen(buf);
		mu_assert("Incomplete read of command output or ERROR", buf[len - 1] != 'O' && buf[len] != 'K');
		printf("DONE!\n");
	}

	return 0;
}

static char * run_tests()
{
	mu_run_test(test_at);
	mu_run_test(test_csq);
	mu_run_test(test_textsms);
	mu_run_test(test_pdushortsms);
	mu_run_test(test_pdulongsms);
	mu_run_test(test_read);
	mu_run_test(test_readall);
	mu_run_test(test_readmsgs);
	return 0;
}

int main(int argc, char *argv[])
{
	char *res;
	gsm = gsm_open(argv[1], B115200);
	if (gsm == NULL)
	{
		perror(argv[1]);
		exit(1);
	}

	res = run_tests();

	if (res != 0)
		printf("%s\n", res);
	else
		printf("ALL TESTS PASSED\n");

	printf("Tests run: %d\n", tests_run);

	gsm_close(gsm);
	return 0;
}
