/****************************************
 *
 * HISTORY
 * $Log:	pagd.c,v $
 * Revision 2.0  89/06/15  15:35:33  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 ****************************************/

#include "protocol.h"

extern char *service;

main()
{
	int (*server_function)(), (*get_service_by_number())();
	int service_number;
	unsigned long recv_long();

	service = "pagd";
	set_comm_socket(0);
	close(1); dup(0);
	close(2); dup(0);
	if (recv_long() != PROTOCOL_PROGRAMLEVEL) {
		send_long(0);
		exit(1);
	}
	service_number = recv_long();
	server_function = get_service_by_number(service_number);
	if (server_function == 0) {
		send_long(0);
		exit(1);
	}
	send_long(1);
	(*server_function)();
	exit(0);
}
