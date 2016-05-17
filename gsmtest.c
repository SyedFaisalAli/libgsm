#include "gsm.h"
#include "minunit.h"
#include <stdio.h>

int tests_run = 0;
gsm_t *gsm;
char inputphone[16];

void get_phonenum()
{
	printf("Enter country code and phone #: ");
	fgets(inputphone, 15, stdin);
	inputphone[strlen(inputphone)-1] = '\0';
	printf("\n");
}

static char * test_at()
{
	mu_assert("Err: AT != OK", CMD_OK(gsm, "AT"));
	return 0;
}

static char * test_textsms()
{
	char msg[] = "Hello?";

	get_phonenum();

	printf("Sending Text SMS with str: %s\n", msg);
	mu_assert("Err: Unable to send SMS", gsm_sendmsgtext(gsm, inputphone, msg) == 0);
	return 0;
}

static char * test_pdulongsms()
{
	char msg[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.Ut enim ad minim veniam, quinostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.Duis aute irure dolor in reprehenderit in voluptate velit esse cillum doloe eu fugiat nulla pariatur.Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum";

	get_phonenum();

	printf("Sending PDU SMS with str: %s\n", msg);
	mu_assert("Err: Unable to send SMS", gsm_sendmsgpdu(gsm, inputphone, msg) == 0);
	return 0;
}

static char * test_pdushortsms()
{
	char msg[] = "This is a message below the 160 SMS character limit.";

	get_phonenum();

	printf("Sending PDU SMS with str: %s\n", msg);
	mu_assert("Err: Unable to send SMS", gsm_sendmsgpdu(gsm, inputphone, msg) == 0);
	return 0;
}

static char * test_csq()
{
	printf("Requesting RSSI...\n");
	int rssi = gsm_rssi(gsm);
	mu_assert("Err: Invalid RSSI Value", rssi != 0);
	return 0;
}

static char * run_tests()
{
	mu_run_test(test_at);
	mu_run_test(test_csq);
	mu_run_test(test_textsms);
	mu_run_test(test_pdushortsms);
	mu_run_test(test_pdulongsms);
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
