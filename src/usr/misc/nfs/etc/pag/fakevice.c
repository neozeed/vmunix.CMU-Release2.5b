/****************************************
 *
 * HISTORY
 * $Log:	fakevice.c,v $
 * Revision 2.1  89/07/31  10:56:42  dimitris
 * 	Fixed bug in reading /etc/mtab
 * 	[89/07/31  10:48:10  dimitris]
 * 
 * Revision 2.0  89/06/15  15:35:25  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 ****************************************/
#include <stdio.h>
#include "fakevice.h"
#include <mntent.h>
#include <strings.h>
#include <netdb.h>
#include <errno.h>


#define	SERVER_NAME_LEN	128

char fakevice_server[SERVER_NAME_LEN];
static char newfilename[1024];

int find_setvice()

{
  char *AFS_SERVER, *getenv();
  struct hostent *afsgate;
  extern int h_errno;

  if (AFS_SERVER = getenv("AFS_SERVER")) {
        if ((afsgate=gethostbyname(AFS_SERVER)) == 0) {
		printf("Unable to locate server %s ",AFS_SERVER);
		if (h_errno == TRY_AGAIN)
			printf("Please try again later \n");
		else
			printf("Unknown host \n");
		return(-1);
	}
        strcpy(fakevice_server,afsgate->h_name);
        printf("Using as NFS to AFS gateway %s  \n",fakevice_server);
        return(1);
  }
  return (0);
}
int find_fakevice()

{ 

  struct mntent *mntp;
  struct hostent *afsgate;
  char *remsign;
  FILE *mnttab;

  extern int errno;
  int cc;
  char afsdir[1024];
  

  cc=readlink("/afs",afsdir,1023); /* Is there a symbolic link for /afs ? */
  if (cc <= 0) {
	if (errno == ENOENT) {
		printf("/afs does not exist \n");
		return (0);
	}
	if (errno == EINVAL)               /* /afs is not a symbolic link */
		strcpy(afsdir,"/afs");
  }

  mnttab = setmntent(MOUNTED, "r"); 

  
  while ((mntp = getmntent(mnttab)) != NULL) { 
	if (strcmp(afsdir,mntp->mnt_dir) == 0) {
		remsign = index(mntp->mnt_fsname,'@');
		if (remsign == NULL) {
				printf("/afs is not a remote NFS mount \n");
				return(0);
		}
		mntp->mnt_fsname = ++remsign;
		afsgate=gethostbyname(mntp->mnt_fsname);
		strcpy(fakevice_server,afsgate->h_name);
	        endmntent(mnttab);
		return (1);
	}

  }
  endmntent(mnttab);
  printf("/afs is not mounted to any NFS/AFS gateway \n");
  return (0);
}



char *get_last_component(pathname)
char *pathname;
{
	char *s, *last_slash = 0;

	for (s = pathname; *s; s++) {
		if (*s == '/') {
			last_slash = s;
		}
	}
	return (last_slash ? last_slash + 1 : pathname);
}

static char *punt(filename)
char *filename;
{
	sprintf(newfilename, "<%s>", filename);
	return newfilename;
}

char *locate_in_afs(filename)
char *filename;
{
	char afspath[1024], cwdpath[1024];
	int afspathlen, cwdpathlen;

	if (*filename == '/') {
		return filename;
	}
	if (getwd(cwdpath) == 0) {
		fprintf(stderr, "%s\n", cwdpath);
		return punt(filename);
	}
	cwdpathlen = strlen(cwdpath);
	if (chdir("/afs") < 0) {
		return punt(filename);
	}
	getwd(afspath);
	afspathlen = strlen(afspath);
	if (afspathlen > cwdpathlen) {
		return punt(filename);
	}
	if (strncmp(afspath, cwdpath, afspathlen)) {
		return punt(filename);
	}
	sprintf(newfilename, "/afs%s/%s", &cwdpath[afspathlen], filename);
	return newfilename;
}
