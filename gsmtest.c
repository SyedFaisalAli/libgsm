#include "gsm.h"

int main(int argc, char *argv[])
{
	// gsmmsg_t *msgs = NULL;
	gsm_t *gsm = gsm_open(argv[1], B115200);
	// const char * cmds[] = {"AT", "AT+CSQ", "AT+CREG?", "AT+CMGF=1", "AT+CSQ?"};
	// char * res = NULL;

	if (gsm == NULL)
	{
		perror(argv[1]);
		exit(1);
	}

	// Send sms
	// gsm_sendmsg(gsm, "4692744326", "Hello?");

// printf("Dev Node: %s\n", gsm->devnode);

	// if ((msgs = gsm_readmsg(gsm, ALL, 2)) != NULL)
	// {
	// 	for (int i = 0; i < 2; i++)
	// 	{
	// 		printf("\nIndex: %d\n", msgs[i].index);
	// 		printf("State: %d\n", msgs[i].state);
	// 		printf("Phone #: %s\n", msgs[i].orig);
	// 		printf("Phone Name: %s\n", msgs[i].origname);
	// 		printf("Msg: %s\n", msgs[i].msg);
	// 		printf("Date: %s\n", asctime(&msgs[i].datetime));
	// 		printf("-------------------------------------------");
	// 	}
	//
	// 	gsm_freemsgs(msgs, 2);
	// }
	// msgs = gsm_readlastunread(gsm);
	//
	// if (msgs == NULL)
	// {
	// 	printf("No more messages\n");
	// }
	// else
	// {
	// 	printf("\nIndex: %d\n", msgs->index);
	// 	printf("State: %d\n", msgs->state);
	// 	printf("Phone #: %s\n", msgs->orig);
	// 	printf("Phone Name: %s\n", msgs->origname);
	// 	printf("Msg: %s\n", msgs->msg);
	// 	printf("Date: %s\n", asctime(&msgs->datetime));
	// 	printf("-------------------------------------------");
	// 	gsm_freemsg(msgs);
	// }
	// char src[] = "It is easy to send text messages.";
	char src[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.Ut enim ad minim veniam, quinostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.Duis aute irure dolor in reprehenderit in voluptate velit esse cillum doloe eu fugiat nulla pariatur.Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum";
	// char src[] = "Hello world";
	char number[] = "14692744326";
	char dest[(septlen(strlen(src))*2)+1];

	printf("Strlen: %d, Septlen: %d\n", strlen(src), septlen(strlen(src)));
	printf("Src: %s\n", src);
	printf("Num: %s\n", number);

	// printf("Original Len: %d, Len: %d, Res: %s\n", strlen(src), len, dest);
	gsm_sendmsgpdu(gsm, number, src);


	gsm_close(gsm);
	return 0;
}
