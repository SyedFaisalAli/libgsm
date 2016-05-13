#include "gsm.h"
#include "minunit.h"

int tests_run = 0;
gsm_t *gsm;

static char * test_at()
{
	mu_assert("Err: AT != OK", CMD_OK(gsm, "AT"));
	return 0;
}

static char * test_textsms()
{
	char msg[] = "Hello?";
	printf("Sending Text SMS with str: %s\n", msg);
	mu_assert("Err: Unable to send SMS", gsm_sendmsgtext(gsm, "4692744326", msg) == 0);
	return 0;
}

static char * test_pdulongsms()
{
	char msg[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.Ut enim ad minim veniam, quinostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.Duis aute irure dolor in reprehenderit in voluptate velit esse cillum doloe eu fugiat nulla pariatur.Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum";
	printf("Sending PDU SMS with str: %s\n", msg);
	mu_assert("Err: Unable to send SMS", gsm_sendmsgpdu(gsm, "4692744326", msg) == 0);
	return 0;
}

static char * test_pdushortsms()
{
	char msg[] = "This is a message below the 160 SMS character limit.";
	printf("Sending PDU SMS with str: %s\n", msg);
	mu_assert("Err: Unable to send SMS", gsm_sendmsgpdu(gsm, "4692744326", msg) == 0);
	return 0;
}

static char * run_tests()
{
	mu_run_test(test_at);
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
