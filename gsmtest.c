#include "gsm.h"

int main(int argc, char *argv[])
{
	gsmmsg_t *msgs = NULL;
	gsm_t *gsm = gsm_open(argv[1], B115200);
	const char * cmds[] = {"AT", "AT+CSQ", "AT+CREG?", "AT+CMGF=1", "AT+CSQ?"};
	char * res;

	if (gsm == NULL)
	{
		perror(argv[1]);
		exit(1);
	}

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
	msgs = gsm_readlastunread(gsm);

	if (msgs == NULL)
	{
		printf("No more messages\n");
	}
	else
	{
		printf("\nIndex: %d\n", msgs->index);
		printf("State: %d\n", msgs->state);
		printf("Phone #: %s\n", msgs->orig);
		printf("Phone Name: %s\n", msgs->origname);
		printf("Msg: %s\n", msgs->msg);
		printf("Date: %s\n", asctime(&msgs->datetime));
		printf("-------------------------------------------");
		gsm_freemsg(msgs);
	}


	gsm_close(gsm);
	return 0;
}
