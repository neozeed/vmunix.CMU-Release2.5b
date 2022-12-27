/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	format.c,v $
 * Revision 1.4  89/08/04  10:01:38  mrt
 * 		Changed the filename for the output .mss file to use
 * 		only the filename and not the full pathname of the .fsrc source.
 * 		This will allow make to put the derived .mss files in the
 * 		object directory.
 * 	[89/08/03            mrt]
 * 
 * Revision 1.2  89/05/05  17:40:25  mrt
 * 	Cleanup for Mach 2.5
 * 
 *
 *  Jan-19-85   Mary Thompson
 *	Changed @t() to generate bold for man section instead of
 *	italics.
 *
 *  Jan-15-87	Mary Thompson
 *		removed extra newline in man file after arg names and error
 *		codes. It was causing an extra space in man output.
 *
 *  Nov-26-86   Mary Thompson
 *		reset nameopen to false between FILES.
 *
 *  Nov-13-86   Mary Thompson
 *		Omit the writing of any unrecognized scribe commands to
 *		the man file. Will work only for commands that take no more
 *		than 1 line.
 *
 *  Nov-12-86	Mary Thompson
 *		Put the argument names and return codes in @t font in
 *		the mss sections instead of @i.
 *
 *  Nov-4-86	Mary Thompson
 *   		Changed the .man keyword to .title and picked up the
 *		name from there as the mss Heading as well as the man 
 *		entry name.
 *		Changed the code to deal with multiple .name and .call lines.
 *
 *  Sept-86	Mary Thompson
 *		Started
 */
/*
 * File: format.c
 *
 * Author - Mary R. Thompson
 *
 * Usage:  format file ...
 *	Translates one or more source files into .mss sections
 *	and man sections. Recognizes the suffix .fsrc and replaces
 *	it with .mss for the mss sections. The name of the man
 *	section is taken from the .man line in the input file.
 *
 * Abstract:
 *	Takes as input a subroutine or program description
 *	in a simple format and generates a scribe formated
 *	version and a man formated version of the information.
 *
 * Design of input file.
 * 	Format recognizes key words which begin with a '.'
 *	and are the first item on a line. Some of the 
 *	key words take arguments on the same line, and
 *	most of them expect text on the following line(s).
 *	The following scribe conventions are followed:
 *	  paragraphs and new items are separated by
 *	    blanklines.
 *	  @i(...), @t(...) and @b(...) generate italics and bold
 *	    respectively. nb. the open and close parens
 *	    symbols must be () and the item within the parens
 *	    must not be greater than 1 line.
 *	  any other scribe commands will be written directly to
 *	  the .mss file and omitted from the man file.
 *
 *	The key words and their arguments are:
 *   .part  scribe-part-name  scribe-manual-name (optional)
 *   .title   man-entry-name/mss-header-name  man-section-number date
 *	 (optional)
 *   .name  name-of-routine(s) @-  description for man table of contents
 *   .include  (optional)
 *	the following lines are #include file-names which are 
 *	entered just as they are formated in the input file.
 *	This text is output in bold-face for the man file.
 *   .call callname
 *	the following lines are a correctly formated c
 *	calling sequence and args defs for the routine.
 *	This text is output in bold-face for the man file.
 *   .agruments
 *	the following lines are of the form
 *	  argument-name argument description
 *	with each argument separated by a blank line.
 *	The argument is output in bold-face for the man file
 *	and example font for the mss file.
 *   .description
 *	The following lines are a plain text description of
 *	routine.  New paragraphs are separated by blank lines.
 *   .returns
 *	the following lines are of the form
 *	   return-value-name  description
 *	with each value separated by a blank line.
 *   .bugs
 *	the following lines are plain text describing any know
 *	shortcommings of the routine.
 *   .files
 *   	the following lines are plani text listing associated
 *	files. ( output only to man file )
 *   .seealso
 *	the following lines are plain text listing any other
 *	relevant man entries.
 *   .history
 *	the following lines are separate history entries of
 *	the form
 *	   date user at institution
 *	   what was changed about the program
 *	the entries are separated by blank lines.
 *      this is output only to the man file.
 *
 *  Note:
 *	The mss format assumes that the enclosing document
 *	is using the acall.lib macro for formating routine
 *	names and generating a primitive summary at the 
 *	end of the document.  A copy of this macro can be
 *	found in /usr/mach/src/doc/manual. An example of
 *	a manual using it is /usr/mach/src/doc/manual/manual.mss.
 */

#include <stdio.h>
#include <strings.h>

#define debug 0

#define getline  buf = fgets(linebuf,100,instream);\
        if (buf == NULL)\
	   goto eof;\
        if (linebuf[0] == NL)\
	   blankline = true; \
	else blankline = false;
#define putmss err = fputs(linebuf,mssstream); \
	if (err == EOF) \
	    break

#define putmannl fputc(NL,manstream)

#define tag '.'
#define NL '\n'
#define RPAR ')'
#define LPAR '('
#define NULLCHAR '\0'


typedef int	boolean_t;
#define true	1
#define false   0

static  char	linebuf[100];
static  FILE  *instream;
static  FILE	*mssstream;
static  FILE  *manstream;


void putman(killws,charbuf)
  boolean_t	killws;
  char		*charbuf;
/****************************************************
 *  Abstract;
 *	Outputs the contents of charbuf to the man file.
 *
 *  Parameter:
 *	killws	if true any extra leading whitespace in charbuf
 *		  is not output to the man file.
 *  		otherwise extra whitespace is kept as is
 *		  required in an .nf(nonfill) environment.
 *  Design:
 *     This routine looks for an @i()/@t() or @b() constructs
 *	in linebuf and outputs the appropriate .I and .B
 *	man format lines. If a @ other than @i, @t or @b 
 *	is found the rest of that line will be omitted.
 *      Note: troff does not ignore extra spaces, and a leading
 *	space on a line causes a line break. Hence care must
 *	be taken to not include any unecessary spaces in the output
 *	regardless of what is included in the input.
 *      Trailing spaces on a line are ignored by troff.
 *
 ******************************************************/
 {
   char  	*lp,*cp;

#define italic	".I "
#define bold  ".B "
#define killwhitespace(p)  while ((*p == ' ') || (*p == '\t')) p++

    lp = charbuf;
    if (killws)
       killwhitespace(lp);
    cp = lp;

scan: 
    while ((*cp != '@') && (*cp != NULLCHAR))
       { fputc(*cp,manstream);
	 cp++;
       }
    if (*cp == NULLCHAR)
      { 
	return;   /* done with whole buffer */
      }
    if (*(cp+1) == 'i')
      { if (cp == charbuf)  /*@i is first thing on line */
       	   fprintf(manstream,"%s",italic);
        else
       	   fprintf(manstream,"\n%s",italic);
      }
    else if ((*(cp+1) == 'b') || (*(cp+1) == 't'))

      { if (cp == charbuf) /* @b is first thing on line */
           fprintf(manstream, "%s",bold);
   	else
           fprintf(manstream, "\n%s",bold);
      }
    else   /* unrecognized scribe command */ 
       return;

    cp += 2;
    if (*cp != LPAR)
        { printf("@i or @b must be followed by (, aborting line\n%s\n",
			linebuf);
	  return;
	}
    cp++;   /* skip the ( */
    killwhitespace(cp);
    while ((*cp != RPAR) && (*cp != NL))
      { fputc(*cp,manstream);
	cp++;
      }
    if (*cp == NL)
      {  printf("italics or bold may not overlap a line boundary. Aborting line\n%s\n",
		linebuf);
	 return;
      }
    cp++;  /* skip the ) */
    if (*cp != NL)
     putmannl;
    killwhitespace(cp);
    goto scan;

 }

main(argc,argv)
  int 	argc;
  char  *argv[];

{
  char	infile[80];
  char  mssfile[80];
  char	manfile[80];
  char	callname[50];
  char  argname[50];
  char  names[80];
  char  manualname[80];
  char  partname[80];
  char	mantitle[50];
  char  mansec[10];
  char  date[20];
  char	*buf;
  char	*rol;
  int	err;
  char	key[15];
  boolean_t	blankline;
  boolean_t	cdescopen = false;
  boolean_t	descopen = false;
  boolean_t	callopen = false;
  boolean_t	nameopen =false;
  boolean_t	includeopen = false;
  boolean_t	synprint = false;
  boolean_t	makeman = false;
  boolean_t	killws  = false;
  int	i;
  char	*cp;

  for (i = 1; i < argc; i++)
  {
    strcpy(infile,argv[i]);
#if debug
    printf("opening file %s\n",infile);
#endif
    instream = fopen(infile,"r");
    if (instream == NULL)
      { strcat(infile,".fsrc");
        instream = fopen(infile,"r");
	if (instream == NULL)
	  { printf("open of %s and %s failed\n",argv[i],infile);
            return;
	  }
      }
    cp = rindex(infile,'.');
    if (cp != 0 )   /* drop the suffix */
      { if (strcmp(cp,".fsrc") == 0)
	 *cp = '\0';
      }
    cp = rindex(infile,'/');
    if (cp != 0)
	strcpy(mssfile,cp+1);	/* keep file name only */
    else strcpy(mssfile,infile);
    strcat(mssfile,".mss");
    mssstream = fopen(mssfile,"w");
    if (mssstream == NULL)
      { printf("open of %s failed\n",mssfile);
        return;
      }
   nameopen = false;  		/* doesn't get reset anywhere else */
   buf = fgets(linebuf,100,instream);
   while (true)
     {  
#if debug
	printf("%s",linebuf);
#endif
        if (linebuf[0] == tag)
	  {  sscanf(linebuf,"%s",key);
#if debug
	     printf("key is %s\n",key);
#endif
	     if (strcmp(".part",key) == 0)
	      { sscanf(linebuf,"%s%s%s",key,partname,manualname);
		fprintf(mssstream,"@part(%s, root='%s')\n",
				partname,manualname);
		do {getline;} while(blankline);
	      }
	     else if (strcmp(".title",key) == 0)
	      { sscanf(linebuf,"%s%s%s%s",key,mantitle,mansec,date);
		strcpy(manfile,mantitle);
		strcat(manfile,".");
		strcat(manfile,mansec);
   		manstream = fopen(manfile,"w");
		if (manstream == NULL)
		   { printf("open of %s failed\n",manfile);
		     return;
		   }
      		fprintf(manstream,".TH %s %s %s\n",mantitle,mansec,date);
		fprintf(manstream,".CM 4\n");  /* page cut macro */
		makeman = true;
		fprintf(mssstream,"@newpage\n@subheading(%s)\n",
			mantitle);
		do {getline; } while(blankline);
	      }
	     else if (strcmp(".name",key) == 0)
	      {	if (makeman)
		  { if (! nameopen)
			fprintf(manstream,".SH NAME\n.nf\n");
		    nameopen = true;
		    rol = index(linebuf,'@');
		    if (rol != 0)
		      { strncpy(names,(linebuf+6),(rol-linebuf-6));
			names[rol-linebuf-6] = '\0';
 		        rol += 2;
		        fprintf(manstream,"%s \\- %s",names,rol);
		      }
		    else /* no description given */
		      { strcpy(names,(linebuf+6));
 			names[strlen(names)-1] = '\0';    /* remove nl char */
			fprintf(manstream,"%s \\-\n",names);
		      }
		  }
		do {getline; } while (blankline);
	      }

	     else if (strcmp(".include",key) == 0)
	      {	fprintf(mssstream,"@begin(CallEnv)\n");
		includeopen = true;
		if (makeman)
		   { fprintf(manstream,".SH SYNOPSIS\n.nf\n.ft B\n");
		     synprint = true;
		   }
		getline;
	   	while (linebuf[0] != tag)
		  { putmss;
		    if (makeman) putman(false,linebuf);
		    getline;
		  }
		fprintf(mssstream,"@end(CallEnv)\n");
		includeopen = false;
	      }

	     else if (strcmp(".call",key) == 0)
	      { callopen = true;
		if (makeman && ! synprint)
			{fprintf(manstream,".SH SYNOPSIS\n.nf\n.ft B\n");
			 synprint = true;
			}
		else if (makeman)
			fprintf(manstream,".nf\n.ft B\n");
		sscanf(linebuf,"%s%s",key,callname);
		fprintf(mssstream,"@machCall{callname='%s',\n",callname);
		fprintf(mssstream,"callf='\n");
		getline;
	   	while (linebuf[0] != tag)
		  { putmss;
		    if (makeman) putman(false,linebuf);
		    getline;
		  }
		fprintf(mssstream,"'}\n");
		if (makeman)
		    fprintf(manstream,"\n.fi\n.ft P\n");
		callopen = false;

	      }

	     else if (strcmp(".arguments",key)== 0)
	      {	fprintf(mssstream,"\n@subheading(Arguments)\n\n");
		fprintf(mssstream,"@begin(calldescription)\n\n");
		cdescopen = true;
		if (makeman)
		    fprintf(manstream,".SH ARGUMENTS\n");
		do { getline; } while (blankline);
		while (linebuf[0] != tag )
		  {
		    sscanf(linebuf,"%s",argname);
		    rol = &linebuf[0]+strlen(argname)+1;
 		    fprintf(mssstream,"@t(%s)@\\@~\n%s",argname,rol);
		    if (makeman)
		       { fprintf(manstream,".TP 15\n.B\n");
			 putman(true,argname);
			 if (argname[0] != '@')  /* otherwise NL has already been written */
			     putmannl;
			 putman(true,rol);
		       }
		    getline;
		    while ((!blankline) && (linebuf[0] != tag))
		      	{ putmss;     /* copy arg description */
			  if (makeman) putman(true,linebuf);
			  getline;}   
		    while (blankline)
			{ putmss;getline;}  /* copy blanklines to mss file */
		  }
		fprintf(mssstream,"@end(calldescription)\n\n");
		putmannl; 
		cdescopen = false;
	      }  /* end arguments */

	     else if (strcmp(".description",key)  == 0)
	        { fprintf(mssstream,"@subheading(Description)\n\n");
		  if (makeman) 
		     fprintf(manstream,".SH DESCRIPTION\n");
		  getline;
	   	  while (linebuf[0] != tag)
		    { putmss;
		      if (makeman) putman(true,linebuf);
		      getline;}
		}

	     else if (strcmp(".returns",key) == 0)
	      {	fprintf(mssstream,"\n@subheading(Returns)\n\n");
		fprintf(mssstream,"@begin(calldescription)\n\n");
		cdescopen = true;
		if (makeman)
		  fprintf(manstream,".SH DIAGNOSTICS\n");
		do { getline;} while (blankline);
		while (linebuf[0] != tag )
		  {
		    sscanf(linebuf,"%s",argname);
		    rol = &linebuf[0]+strlen(argname)+1;
			/* strip any leading white space */
		    while ((*rol == ' ') || (*rol == '\t')) rol++;
 		    fprintf(mssstream,"@t(%s)@\\@~\n%s",argname,rol);
		    if (makeman)
			{ fprintf(manstream,".TP 25\n");
	   		  putman(true,argname);
			  if (argname[0] != '@')  /* otherwise NL has already been written */
			      putmannl;
			  putman(true,rol);
			}
		    getline;
		    while ((!blankline) && (linebuf[0] != tag))
		      	{ putmss;    /* copy return value description */
			  if (makeman) putman(true,linebuf);
			  getline;}   
		    while (blankline)
			{ putmss;getline;}  /* copy blanklines to mss file */
		  }
		fprintf(mssstream,"@end(calldescription)\n\n");
 		putmannl;
		cdescopen = false;
		}
		
	      else if (strcmp(".history",key) == 0)
		{  /* output only to man section */
		  if (makeman)
		     fprintf(manstream,".SH HISTORY\n");
		  do { getline;} while (blankline);
		  while (linebuf[0] != tag )
		   {
		    if (makeman) 
		      fprintf(manstream,".TP\n%s",linebuf);
		    getline;
		    while ((!blankline) && (linebuf[0] != tag))
		      	{    /* copy return value description */
			  if (makeman) putman(true,linebuf);
			  getline;}   
		    while (blankline)
		       {  getline;}  /* skip blanklines */

		   }
		 putmannl;
		}
	      else if (strcmp(".bugs",key) == 0)
		{ fprintf(mssstream,"@subheading(Notes)\n");
		  if (makeman)
		     fprintf(manstream,".SH BUGS\n");
		  getline;
	   	  while (linebuf[0] != tag)
		    { putmss;
		      if (makeman) putman(true,linebuf);
		      getline;
		    }
		}
	      else if (strcmp(".seealso",key) == 0)
		{ /* output only to man section */
		  fprintf(mssstream,"@subheading(See Also)\n");
		  if (makeman)
		     fprintf(manstream,".SH SEE ALSO\n");
		  getline;
	   	  while (linebuf[0] != tag)
		    { putmss;
		      if (makeman) putman(true,linebuf);
		      getline;
		    }
		}
	      else if (strcmp(".files",key) == 0)
		{ /* output only to man section */
		  if (makeman)
		     fprintf(manstream,".SH FILES\n");
		  getline;
	   	  while (linebuf[0] != tag)
		    { 
		      if (makeman) putman(true,linebuf);
		      getline;
		    }
		}
	      else 
		{ printf("unrecognized key word %s\n",key);
		  getline;
		}
	  } /* end of case on key */
	else
	  { printf("expecting a key word at line %s\n",linebuf);
	    getline;
	  }
     } /* while not EOF */

eof: if (includeopen)	
	fprintf(mssstream,"@end(CallEnv)\n");
     if (cdescopen)
	fprintf(mssstream,"@end(calldescription)\n");
     if (descopen)
	fprintf(mssstream,"@end(description)\n");
     if (callopen)
	fprintf(mssstream,"'}\n");
    putmannl;  /* write final nl to man section */
    fprintf(mssstream,"\n");  /* write final nl to mss file */
    err = fclose(instream);
    err = fclose(mssstream);
    err = fclose(manstream);
  }  /* end loop over all input args */
}

