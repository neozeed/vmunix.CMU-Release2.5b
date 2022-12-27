/****************************************
 *
 * HISTORY
 * $Log:	afs_groups.c,v $
 * Revision 2.0  89/06/15  15:35:07  dimitris
 *   Organized into a misc collection and SSPized
 * 
 * 
 *
 ****************************************/

#define	NOPAG	0xffffffff

unsigned long afs_get_pag_from_groups(g0, g1)
unsigned long g0, g1;
{
	unsigned long h, l;

	g0 -= 0x3f00;
	g1 -= 0x3f00;
	if (g0 < 0xc000 && g1 < 0xc000) {
		l = ((g0 & 0x3fff) << 14) | (g1 & 0x3fff);
		h = (g0 >> 14);
		h = (g1 >> 14) + h + h + h;
		return ((h << 28) | l);
	}
	return NOPAG;
}

afs_get_groups_from_pag(pag, g0p, g1p)
unsigned long pag;
unsigned long *g0p, *g1p;
{
	unsigned short g0, g1;

	pag &= 0x7fffffff;
	g0 = 0x3fff & (pag >> 14);
	g1 = 0x3fff & pag;
	g0 |= ((pag >> 28) / 3) << 14;
	g1 |= ((pag >> 28) % 3) << 14;
	*g0p = g0 + 0x3f00;
	*g1p = g1 + 0x3f00;
}
