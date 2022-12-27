/****************************************
 *
 * HISTORY
 * $Log:	pag.c,v $
 * Revision 2.1  89/07/31  10:56:54  dimitris
 * 	Added  	exit(0) to guarantee that login will not give an error message
 * 	when the authentication is succesful.
 * 	[89/07/31  10:49:28  dimitris]
 * 
 * Revision 2.0  89/06/15  15:35:30  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 ****************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include "protocol.h"
#include "fakevice.h"

char *service;

struct hostent *getafsserverhostent()
{
	struct hostent *he;

	if ((he = gethostbyname(fakevice_server)) == 0) {
		fprintf(stderr, "%s: %s: Unknown host\n",
			service, fakevice_server);
		exit(1);
	}
	return he;
}

long getafsserverport()
{
	struct servent *sp;

	sp = getservbyname("pag", "tcp");
	if (sp == NULL) {
/*
		fatal("getservbyname: pag/tcp not found\n");
*/
		return 879;
	}
	return sp->s_port;
}

getafsserverconnection()
{
	struct sockaddr_in sa;
	int s;
	struct hostent *he;

	bzero(&sa, sizeof(sa));
	he = getafsserverhostent();
	sa.sin_port = getafsserverport();
	sa.sin_family = he->h_addrtype;
	bcopy(he->h_addr, &sa.sin_addr, he->h_length);
	s = socket(he->h_addrtype, SOCK_STREAM, 0);
	if (s < 0) {
		fatal("socket");
	}
	if (connect(s, &sa, sizeof(sa)) < 0) {
		fatal("connect");
	}
	set_comm_socket(s);
}

main(argc, argv)
int argc;
char **argv;
{
	int (*client_function)(), (*get_service_by_number())();
	int service_number;
        int setvice ;

	if (argc >= 3 && ! strcmp(argv[1], "-xxx")) {
		argc -= 2;
		argv += 2;
	}
	service = get_last_component(argv[0]);
        if ((setvice = find_setvice()) < 0)
		exit(1);
        if (!setvice) {
        	if (find_fakevice() == 0) {
	        	fprintf(stderr, "%s: Unable to determine which server to use.\n",
			service);
		        exit(1);
		}
	}
	service_number = get_service_number(service);
	client_function = get_service_by_number(service_number);
	if (client_function == 0) {
		fprintf(stderr, "%s: Command not supported.\n", service);
		exit(1);
	}
	getafsserverconnection();
	send_long(PROTOCOL_PROGRAMLEVEL);
	send_long(service_number);
	if (! recv_long()) {
		fprintf(stderr, "%s: Command not supported by server.\n",
			service);
		exit(1);
	}
	(*client_function)(argc, argv);
	exit(0);
}
