/****************************************
 *
 * HISTORY
 * $Log:	setpag.c,v $
 * Revision 2.0  89/06/15  15:35:40  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 ****************************************/
#include <errno.h>
#include <sys/param.h>

#define	NOPAG	0xffffffff

/*
 *  Unfortunately this must be called with euid == 0.
 *  Thus on completion we set euid = ruid.
 */
setpag()
{
	int ngroups, gidset[NGROUPS];
	int j;

	ngroups = getgroups(NGROUPS, gidset);
	if (ngroups == -1) {
		return -1;
	}
	if (afs_get_pag_from_groups(gidset[0], gidset[1]) == NOPAG) {
		/* we will have to shift grouplist to make room for pag */
		if (ngroups + 2 > NGROUPS) {
			/* this is what the real setpag returns */
			return E2BIG;
		}
		for (j = ngroups - 1; j >= 0; j--) {
			gidset[j + 2] = gidset[j];
		}
		ngroups += 2;
	}
	/* this should be in a loop until we are sure this is a free pag */
	afs_get_groups_from_pag(afs_genpag(), &gidset[0], &gidset[1]);
	if (setgroups(ngroups, gidset) == -1) {
		return -1;
	}
	return setreuid(-1, getuid());
}
