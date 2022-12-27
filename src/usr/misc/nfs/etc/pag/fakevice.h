/*******************************************************
 * HISTORY
 * $Log:	fakevice.h,v $
 * Revision 2.0  89/06/15  15:35:03  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 *******************************************************/
extern char *fakevice_absolute();
extern char *get_last_component();
extern char *locate_in_afs();
extern char fakevice_server[];

#ifdef	SVR3
#define	NGROUPS	2
#define	getgroups(ngroups, gidset) (gidset[0] = gidset[1] = 0)
#endif	SVR3
