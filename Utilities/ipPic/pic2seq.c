
/********************************************************************* /
/   pic2seq                                                            /
/ -------------------------------------------------------------------- /
/   DESCRIPTION                                                        /
/                                                                      /
/      converts a pic-seq to single pic-files                          /
/                                                                      /
/                                                                      /
/   FUNCTION DECLARATION                                               /
/                                                                      /
/                                                                      /
/   PROTOTYPE CALL                                                     /
/                                                                      /
/                                                                      /
/   PARAMETER                                                          /
/                                                                      /
/                                                                      /
/   RETURN VALUES                                                      /
/                                                                      /
/      0 - no error occured                                            /
/     -1 - error oocured                                               /
/                                                                      /
/   AUTHOR & DATE                                                      /
/                                                                      /
/      Andre Schroeter    08.06.1992                                   /
/                                                                      /
/   UPDATES                                                            /
/                                                                      /
/                                                                      /
/                                                                      /
/   COPYRIGHT (c) 1992 by DKFZ (Dept. MBI) Heidelberg, FRG             /
/**********************************************************************/
static char *what[] = { 
	"@(#)seqtopic \t06/92\tDKFZ (Dept. MBI)\t08.06.1992",
};
/*   standard include files   */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <ipPic/ipPic.h>


#ifndef FILENAME_MAX
#  define FILENAME_MAX 1024
#endif

#ifndef Bool
typedef char Bool;
# define True 1
# define False 0
#endif

void main( int argc, char *argv[] )
{ 
	int i;
	
	unsigned int n;
	Bool is_number;
	Bool scan_error;
	Bool use_stdin;
	Bool use_stdout;
	
	Bool append;
	int soffset;
	int aoffset;
	
	char infile_name[FILENAME_MAX],
		outfile_name[FILENAME_MAX],
		outfile_string[FILENAME_MAX];
	
	ipPicDescriptor *pic = NULL,*header;
	
	
	/*--------------- commandline scaning starts here -----------*/ 
	append = False;
	use_stdin = True;
	use_stdout = True;
	scan_error = False;
	
	n = 1;
	while( n < argc )
    {
		if( *argv[n]  == '-' )
			switch( *(argv[n]+1) )
		{
            case 'a': append = True;
				break;
				/*case 'o': use_stdout = False;
				strcpy( outfile_string, argv[n]+2 );
				if( outfile_string[0] == '\0' )
				scan_error = True;
				break;*/
			case 'h':
			case 'H': scan_error = True;
				break;
			default: fprintf( stderr,
						 "%s: sorry, unknown option -%c\n",
						 argv[0], *(argv[n]+1) );
				scan_error = True;
				break;
		}
		else
		{
			use_stdin = False;
			strcpy( outfile_string, argv[n] );
		}
		n++;
    }
	
	if( scan_error )
    { 
		fprintf( stderr, "Usage:   %s -a outfile infiles \n", argv[0] );
		fprintf( stderr, "  -a         appends the infiles to outfile\n" );
		fprintf( stderr, "  outfile    the outputfile to create\n" );
		fprintf( stderr, "  infiles    a list of inputfiles\n" );
		exit( -1 );
    }
	
	/*--------------- commandline scaning ends here -------------*/
	
	
	/* make filenames */
	if( !use_stdout) 
		if( !strchr(outfile_name, '.') )
			strcat( outfile_name, ".seq" );
	aoffset = 0;
	soffset = 0;
	header=ipPicGetHeader( argv[aoffset+2],NULL);
	header = ipPicGetTags( argv[aoffset+2],header );
	if( append )
	{
		aoffset = 1;
		pic = ipPicGetHeader( argv[aoffset+1],
			pic );
		if( pic != NULL )
			soffset = pic->n[pic->dim-1];
		else
			soffset = 0;
	}
	
	for( i=0; i<argc-2-aoffset; i++ )
	{
		printf( "%.3i %s\n", i, argv[aoffset+i+2] );
		
		
/*		pic = ipPicGetSlice( argv[aoffset+i+2],
			pic,
			1 );*/
		pic = ipPicGet( argv[aoffset+i+2],
			pic);
		
		if( pic == NULL )
		{
			fprintf( stderr, "sorry, can't open %s\n", argv[aoffset+i+2] );
			exit( -1 );
		}
		pic->info->write_protect = ipFalse;
		pic->info->tags_head = _ipPicCloneTags(header->info->tags_head );
		if((i==0) && (!append))
		{
			pic->dim++; pic->n[pic->dim-1]=1;
			ipPicPut(argv[aoffset+1], pic);	
		}
		else
			ipPicPutSlice( argv[aoffset+1],
				pic,
				soffset + i + 1 );
		
	}
	
	exit( 0 );
}
