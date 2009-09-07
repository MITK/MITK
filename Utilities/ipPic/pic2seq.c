
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

#include <mitkIpPic.h>


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
	
	int n_infiles;
	char **infile_names;

	unsigned int n;
	Bool is_number;
	Bool scan_error;
	Bool use_stdin;
	Bool use_stdout;
	Bool use_textfile;
	
	Bool append;
	int soffset;
	int aoffset;
	
	char outfile_name[FILENAME_MAX],
		outfile_string[FILENAME_MAX];
	
	mitkIpPicDescriptor *pic = NULL,*header;
	
	
	/*--------------- commandline scaning starts here -----------*/ 
	append = False;
	use_stdin = True;
	use_stdout = True;
	scan_error = False;
	use_textfile = False;
	
	aoffset = 0;
	soffset = 0;

	n = 1;
	while( n < argc )
    {
		if( *argv[n]  == '-' )
			switch( *(argv[n]+1) )
		{
            case 'a': append = True;
				++aoffset;
				break;
				/*case 'o': use_stdout = False;
				strcpy( outfile_string, argv[n]+2 );
				if( outfile_string[0] == '\0' )
				scan_error = True;
				break;*/
			case 'h':
			case 'H': scan_error = True;
				break;
            case 'f': use_textfile = True;
				++aoffset;
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
		fprintf( stderr, "Usage:   %s -a -f outfile infiles \n", argv[0] );
		fprintf( stderr, "  -a         appends the infiles to outfile\n" );
		fprintf( stderr, "  -f         \"infiles\" is a text file containing the list of inputfiles\n" );
		fprintf( stderr, "  outfile    the outputfile to create\n" );
		fprintf( stderr, "  infiles    a list of inputfiles\n" );
		exit( -1 );
    }
	
	/*--------------- commandline scaning ends here -------------*/
	
	/* read text file, if specified*/
	if(use_textfile)
	{
		char tmp[FILENAME_MAX];
		FILE *f;

		n_infiles=0;

		f=fopen(argv[aoffset+2], "rt");
		if(f==NULL)
		{
			fprintf( stderr, "Couldn't open text file containing the list of inputfiles.\n", argv[0] );
			exit( -2 );
		}

		while(!feof(f))
		{
			fgets(tmp, 255, f);
			++n_infiles;
		}

		infile_names=malloc(n_infiles*sizeof(char*));

		rewind(f);

		i=0;
		while(!feof(f))
		{
			char *nl;
			infile_names[i]=malloc(FILENAME_MAX*sizeof(char));
			fgets(infile_names[i], FILENAME_MAX, f);
			nl=infile_names[i]+strlen(infile_names[i])-1;
			if(*nl=='\n') *nl=0;
			if(mitkIpPicAccess(infile_names[i], 4)==0)
				++i;
			else
			{
				free(infile_names[i]);
				--n_infiles;
			}
		}
	}
	else
	{
		n_infiles=argc-2-aoffset;
		if(n_infiles<=0)
			exit( -3 );

		infile_names=malloc(n_infiles*sizeof(char*));
		for(i=0;i<n_infiles;++i)
			infile_names[i]=argv[aoffset+i+2];
	}
		
	/* make filenames */
	if( !use_stdout) 
		if( !strchr(outfile_name, '.') )
			strcat( outfile_name, ".seq" );
	if( append )
	{
		pic = mitkIpPicGetHeader( argv[aoffset+1],
			pic );
		if( pic != NULL )
			soffset = pic->n[pic->dim-1];
		else
			soffset = 0;
	}

	header=mitkIpPicGetHeader( infile_names[0], NULL);
	header = mitkIpPicGetTags( infile_names[0], header );

	for( i=0; i<n_infiles; i++ )
	{
		printf( "%.3i %s\n", i, infile_names[i] );
		
		
/*		pic = mitkIpPicGetSlice( infile_names[i],
			pic,
			1 );*/
		pic = mitkIpPicGet( infile_names[i],
			pic);
		
		if( pic == NULL )
		{
			fprintf( stderr, "sorry, can't open %s\n", infile_names[i] );
			exit( -1 );
		}
		pic->info->write_protect = ipFalse;
		pic->info->tags_head = _mitkIpPicCloneTags(header->info->tags_head );
		if((i==0) && (!append))
		{
			pic->dim++; pic->n[pic->dim-1]=1;
			mitkIpPicPut(argv[aoffset+1], pic);	
		}
		else
			mitkIpPicPutSlice( argv[aoffset+1],
				pic,
				soffset + i + 1 );
		
	}
	
	exit( 0 );
}
