/***********************************************
 * HISTORY
 * $Log:	service.h,v $
 * Revision 2.0  89/06/15  15:35:36  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 **********************************************/
#define	SERVICE_SETPAG		0
#define	SERVICE_SETSYSNAME	1
#define	SERVICE_FS		2
#define	SERVICE_LOG		3
#define	SERVICE_TOKENS		4
#define	SERVICE_UNLOG		5

#define	NUM_SERVICES		6

#define	INVALID_SERVICE		NUM_SERVICES
#define	VALID_SERVICE(service)	((service >= 0) && (service < NUM_SERVICES))
