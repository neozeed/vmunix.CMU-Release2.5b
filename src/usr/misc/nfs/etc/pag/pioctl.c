/****************************************
 *
 * HISTORY
 * $Log:	pioctl.c,v $
 * Revision 2.0  89/06/15  15:35:11  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 ****************************************/
#include <sys/ioctl.h>
#include <afs/vice.h>

#define	P_SetSysName		_VICEIOCTL(38)
#define	P_SetClientContext	_VICEIOCTL(39)

typedef struct {
	unsigned long hostaddr;
	unsigned long uid;
	unsigned long g0;
	unsigned long g1;
} afs_cred_t;

int pioctl_setsysname(name)
char *name;
{
	struct ViceIoctl blob;
	unsigned long blobbet[2];

	blobbet[0] = (unsigned long) name;
	blobbet[1] = (unsigned long) strlen(name);
	blob.in = (caddr_t) blobbet;
	blob.in_size = sizeof(blobbet);
	blob.out_size = 0;
	return pioctl(0, P_SetSysName, &blob, 1);
}

int pioctl_setclientcontext(afs_cred)
afs_cred_t *afs_cred;
{
	struct ViceIoctl blob;
	unsigned long blobbet[4];

	blobbet[0] = afs_cred->hostaddr;
	blobbet[1] = afs_cred->uid;
	blobbet[2] = afs_cred->g0;
	blobbet[3] = afs_cred->g1;
	blob.in = (caddr_t) blobbet;
	blob.in_size = sizeof(blobbet);
	blob.out_size = 0;
	return pioctl(0, P_SetClientContext, &blob, 1);
}
