/*****************************************************************************

 Copyright (c) 1993-2000,  Div. Medical and Biological Informatics, 
 Deutsches Krebsforschungszentrum, Heidelberg, Germany
 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

 - All advertising materials mentioning features or use of this software must 
   display the following acknowledgement: 
          
     "This product includes software developed by the Div. Medical and 
      Biological Informatics, Deutsches Krebsforschungszentrum, Heidelberg, 
      Germany."

 - Neither the name of the Deutsches Krebsforschungszentrum nor the names of 
   its contributors may be used to endorse or promote products derived from 
   this software without specific prior written permission. 

   THIS SOFTWARE IS PROVIDED BY THE DIVISION MEDICAL AND BIOLOGICAL
   INFORMATICS AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE DIVISION MEDICAL AND BIOLOGICAL INFORMATICS,
   THE DEUTSCHES KREBSFORSCHUNGSZENTRUM OR CONTRIBUTORS BE LIABLE FOR 
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

 Send comments and/or bug reports to:
   mbi-software@dkfz-heidelberg.de

*****************************************************************************/

/* include files                                                         */

#include "mitkIpFuncP.h"

#ifdef _WIN32
#define strcasecmp _stricmp
#define PATH_MAX 255
#endif

/* main function                                                         */

int
main (int argc, char **argv) 
{
	/* variables                                                          */
	
	mitkIpPicDescriptor *pic_new=NULL;   /* image descriptors            */
	mitkIpPicDescriptor *pic_ret=NULL;   /* image descriptors            */
	mitkIpPicDescriptor *pic_hlp=NULL;   /* image descriptors            */
	mitkIpPicDescriptor *pic_old=NULL;   /* image descriptors            */
	mitkIpPicDescriptor *pic_mask=NULL;  /* convolution mask             */
	mitkIpPicDescriptor *mask_1=NULL, *mask_2=NULL;     /* convolution mask             */
	char            operation[PATH_MAX];  /* image processing operation   */
	char            pic_name[PATH_MAX];   /* file name of original image  */
	char            pic_name_t[PATH_MAX]; /* file name of transformed im. */
	char            mask_name[PATH_MAX];  /* file name of transformed im. */
	char            error_nr[PATH_MAX];   /* file name of transformed im. */
	mitkIpPicType_t     type;
	mitkIpUInt4_t       range;                /* result of range function     */
	mitkIpUInt4_t       no_label;
	mitkIpUInt4_t       border;               /* handling of the borders      */
	mitkIpUInt4_t       rank;                 /*                              */
	mitkIpUInt4_t       keep;                 /*                              */
	mitkIpUInt4_t       radius;               /*                              */
	mitkIpUInt4_t       *begin;               /* start of window              */
	mitkIpUInt4_t       *length;              /* length of window             */
	mitkIpUInt4_t       len_mask;             /* length of mask               */
	mitkIpUInt4_t       dim_mask;             /* dimension of mask            */
	mitkIpInt4_t        axis;                 /* reflection axis              */
	mitkIpUInt4_t       i;
	mitkIpFloat8_t      min, max;             /* extreme greyvalues           */
	mitkIpFloat8_t      min_gv, max_gv;       /* extreme greyvalues           */
	mitkIpFloat8_t      gv_low, gv_up;        /*                              */
	mitkIpFloat8_t      threshold;         
	mitkIpFloat8_t      value;                /* value that is added to image */
	mitkIpFloat8_t      mean;                 /* mean greyvalue               */
	mitkIpFloat8_t      var;                  /* variance of greyvalues       */
	mitkIpFloat8_t      gv;                   /* new greyvalue                */
	mitkIpFloat8_t      s_der;                /* standard derivation          */
	mitkIpFloat8_t      *sc;                  /* standard derivation          */
	int             *perm;                /* permutation vector for transpose */
	mitkIpFloat8_t      *grav;                /* center of gravity            */
	mitkIpFloat8_t      *inertia, *ev;
	mitkIpUInt4_t       *hist;                /* greylevel histogram          */
	mitkIpUInt4_t       size_hist;            /* no. of elements in histogram */
	mitkIpUInt4_t       mask_size;            /* size of transformation mask  */
	mitkIpBool_t        picput_flag=mitkIpTrue;   /* */
	int             *grad, *order;
	
	/* input of operation and image file name                             */
	
	
	if ( ((unsigned int) argc == 1) || ( ((unsigned int) argc == 2 ) && (strcasecmp (argv[1], "-h") == 0)))
	{
		printf("Usage: ipFunc operation infile outfile parameterlist\n");
        printf(" Arithmetic:   AddC, AddI, DivC, DivI, MultC, MultI, SubC, SubI\n");
        printf(" Logic:        And, Not, Or\n");
        printf(" Geometric:    Refl, Rotate, Scale, Transpose, Window, WindowR\n");
        printf(" Point:        Equal, Exp, Inv, LevWin, LN, Log, Norm, NormY\n");
        printf("               Pot, Select, SelInv, SelMM, Sqrt, Thresh, ZeroCr\n");
        printf(" Locale:       Canny, Conv, GaussF, Grad, Laplace, Mean\n");
        printf("               Rank, Roberts, Shp, Sobel\n");
        printf(" Morpho:       Close, Dila, Ero, Open\n");
        printf(" Statistic:    Extr, ExtrC, ExtrR, ExtrROI, Mean, MeanC, MeanR\n");
        printf("               MeanROI, Median, SDev, SDevC, SDevR, SDevROI\n");
        printf("               Var, VarC, VarR, VarROI\n");
        printf(" Segmentation: Label, RegGrow\n");
        printf(" Misc:         Border, BorderX, DrawPoly, Frame, Convert, MakeMorphSquareMask\n");
		exit(1);
	}
	
	strcpy  ( operation , argv[1]);
	if( ((unsigned int) argc != 2) && (strcasecmp (argv[2], "-h") != 0))
	{
		strcpy ( pic_name, argv[2]);
		pic_old = mitkIpPicGet ( pic_name, NULL );
		pic_ret = pic_old;
		if ( pic_old == NULL )
		{
            printf("could not open file\n");
            exit(1);
		}
	}
    if ( strcasecmp ( operation, "Inv" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{ 
            printf ( " usage: ipFunc Inv infile outfile \n" );
            exit ( 1 );
		}
		pic_new = mitkIpFuncInv ( pic_old, pic_ret );
	}
    else if ( strcasecmp ( operation, "Label" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{
			printf ( " ipFunc Label infile outfile \n" );
			exit ( 1 );
		}
		pic_new = mitkIpFuncLabel ( pic_old, &no_label );
		printf ("Number of Labels: %d \n", no_label );
	}
    else if ( strcasecmp ( operation, "RegGrow" ) == 0 )
	{
		mitkIpUInt4_t blabel, rlabel, kind;

		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != pic_old->dim * 2 + 8 ) )  
		{
			printf ( " usage: ipFunc RegGrow infile outfile beg_seed1...beg_seedn end_seed1...end_seedn border_label region_label std_dev_factor(double!!) kind(0=show border,1=show region)\n" );
			exit ( 1 );
		}
		begin  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+pic_old->dim+i], "%d", &length[i] );
        printf ( " %d %d %d       %d %d %d \n", begin[0], begin[1], begin[2], length[0], length[1], length[2] );

		sscanf  ( argv[4+pic_old->dim*2], "%d", &blabel );
		sscanf  ( argv[5+pic_old->dim*2], "%d", &rlabel );
		s_der=atof(argv[6+pic_old->dim*2]);
		sscanf  ( argv[7+pic_old->dim*2], "%d", &kind );

		pic_new = mitkIpFuncRegGrow ( pic_old, pic_old->dim, begin, length, blabel, rlabel, s_der, kind );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "Convert" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0) ) ||
			( (unsigned int) argc != 6 ) )
		{
            printf("Usage: ipFunc convert infile outfile type bpe \n");
			printf("type must be given as number\n");
			printf("type:     mitkIpPicInt           =   3\n");
			printf("          mitkIpPicUInt          =   4\n");
			printf("          mitkIpPicFloat         =   5\n");
			exit(1);
		}
		sscanf  ( argv[4],"%d", &rank );
		sscanf  ( argv[5],"%d", &range );
		type = rank;
		pic_new = mitkIpFuncConvert ( pic_old, rank, range );
	}
    else if ( strcasecmp ( operation, "Refl" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Refl infile outfile axis \n");
			printf("axis must be given as number\n");
			printf("   x-axis                    =   0\n");
			printf("   y-axis                    =   1\n");
			printf("   z-axis                    =   2\n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &axis );
		pic_new = mitkIpFuncRefl ( pic_old, axis );
	}
    else if ( strcasecmp ( operation, "Sqrt" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{
			printf ( " ipFunc Sqrt infile outfile \n" );
			exit ( 1 );
		}
		keep = mitkIpFuncKeep; 
		pic_new = mitkIpFuncSqrt ( pic_old, keep, pic_ret );
	}
    else if ( strcasecmp ( operation, "Pot" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Pot infile outfile exponent \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		sscanf  ( argv[4], "%lf", &value );
		pic_new = mitkIpFuncPot ( pic_old, value, keep, pic_ret );
		printf ( " Fehler : %d \n", mitkIpFuncErrno );
	}
    else if ( strcasecmp ( operation, "DivC" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc DivC infile outfile value \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		sscanf  ( argv[4], "%lf", &value );
		pic_new = mitkIpFuncDivC ( pic_old, value, keep, pic_ret  );
	}
    else if ( strcasecmp ( operation, "MultC" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc MultC infile outfile value \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		sscanf  ( argv[4], "%lf", &value );
		pic_new = mitkIpFuncMultC ( pic_old, value, keep, pic_ret );
	}
    else if ( strcasecmp ( operation, "SubC" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc SubC infile outfile value    \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		sscanf  ( argv[4], "%lf", &value );
		pic_new = mitkIpFuncSubC ( pic_old, value, keep, pic_ret );
	}
    else if ( strcasecmp ( operation, "AddC" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc AddC infile outfile value    \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		sscanf  ( argv[4], "%lf", &value );
		pic_new = mitkIpFuncAddC ( pic_old, value, keep, pic_ret );
	}
    else if ( strcasecmp ( operation, "AddSl" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 4 ) )
		{
			printf("Usage: ipFunc AddSl infile outfile    \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		pic_new = mitkIpFuncAddSl ( pic_old, keep );                
	}
    else if ( strcasecmp ( operation, "DivI" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc DivI infile outfile image    \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_hlp = mitkIpPicGet ( mask_name, NULL );
		if ( pic_hlp != NULL ) 
			pic_new = mitkIpFuncDivI ( pic_old, pic_hlp, keep, pic_ret );
		else 
			printf ( " iamge doesn't exist \n" ); 
	}
    else if ( strcasecmp ( operation, "MultI" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc MultI infile outfile image    \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_hlp = mitkIpPicGet ( mask_name, NULL );
		if ( pic_hlp != NULL ) 
			pic_new = mitkIpFuncMultI ( pic_old, pic_hlp, keep, pic_ret );
		else 
			printf ( " iamge doesn't exist \n" ); 
	}
    else if ( strcasecmp ( operation, "AddI" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc AddI infile outfile image    \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_hlp = mitkIpPicGet ( mask_name, NULL );
		if ( pic_hlp != NULL ) 
			pic_new = mitkIpFuncAddI ( pic_old, pic_hlp, keep, pic_ret );
		else 
			printf ( " iamge doesn't exist \n" ); 
	}
    else if ( strcasecmp ( operation, "SubI" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc SubI infile outfile image    \n");
			exit(1);
		}
		keep = mitkIpFuncKeep; 
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_hlp = mitkIpPicGet ( mask_name, NULL );
		if ( pic_hlp != NULL ) 
			pic_new = mitkIpFuncSubI ( pic_old, pic_hlp, keep, pic_ret );
		else 
			printf ( " iamge doesn't exist \n" ); 
	}
    else if ( strcasecmp ( operation, "Or" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Or infile outfile image    \n");
			exit(1);
		}
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_hlp = mitkIpPicGet ( mask_name, NULL );
		if ( pic_hlp != NULL ) 
			pic_new = mitkIpFuncOr ( pic_old, pic_hlp, pic_ret );
		else 
			printf ( " iamge doesn't exist \n" ); 
	}
    else if ( strcasecmp ( operation, "And" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc And infile outfile image    \n");
			exit(1);
		}
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_hlp = mitkIpPicGet ( mask_name, NULL );
		if ( pic_hlp != NULL ) 
			pic_new = mitkIpFuncAnd ( pic_old, pic_hlp, pic_ret );
		else 
			printf ( " iamge doesn't exist \n" ); 
	}
    else if ( strcasecmp ( operation, "Not" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{
			printf ( " ipFunc Not infile outfile \n" );
			exit ( 1 );
		}
		pic_new = mitkIpFuncNot ( pic_old, NULL );
	}
    else if ( strcasecmp ( operation, "Close" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Close infile outfile mask     \n");
			printf("  mask must be the name of a PIC-file containing the mask \n" );
			exit(1);
		}
		border = mitkIpFuncBorderZero;
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_mask = mitkIpPicGet ( mask_name, NULL );
        mitkIpPicPut ( "Test.mask", pic_mask );
		if ( pic_mask != NULL )
		{
			pic_new = mitkIpFuncClose( pic_old, pic_mask, border );
			mitkIpPicFree ( pic_mask );
		}
		else
			printf ( " iamge doesn't exist \n" );
	}   
    else if ( strcasecmp ( operation, "Open" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Open  infile outfile mask     \n");
			printf("  mask must be the name of a PIC-file containing the mask \n" );
			exit(1);
		}
		border = mitkIpFuncBorderZero;
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_mask = mitkIpPicGet ( mask_name, NULL );
		if ( pic_mask != NULL )
		{
			pic_new = mitkIpFuncOpen ( pic_old, pic_mask, border );
			mitkIpPicFree ( pic_mask );
		}
		else
			printf ( " iamge doesn't exist \n" );
	}   
    else if ( strcasecmp ( operation, "Dila" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Dila infile outfile mask     \n");
			printf("  mask must be the name of a PIC-file containing the mask \n" );
			exit(1);
		}
		border = mitkIpFuncBorderZero;
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_mask = mitkIpPicGet ( mask_name, NULL );
		if ( pic_mask != NULL )
		{
			pic_new = mitkIpFuncDila  ( pic_old, pic_mask, border );
			mitkIpPicFree ( pic_mask );
		}
		else
			printf ( " iamge doesn't exist \n" );
	}    
    else if ( strcasecmp ( operation, "Ero" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Ero infile outfile mask     \n");
			printf("  mask must be the name of a PIC-file containing the mask \n" );
			exit(1);
		}
		border = mitkIpFuncBorderZero;
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_mask = mitkIpPicGet ( mask_name, NULL );
		if ( pic_mask != NULL )
		{
			pic_new = mitkIpFuncEro   ( pic_old, pic_mask, border );
			mitkIpPicFree ( pic_mask );
		}
		else
			printf ( " iamge doesn't exist \n" );
	}    
    /*   else if ( strcasecmp ( operation, "hitmiss" ) == 0 )
	{
	strcpy ( mask_name, "" );
	printf ( " mask name 1     : " );
	scanf  ( "%s", mask_name );
	mask_1 = mitkIpPicGet ( mask_name, NULL );
	if ( mask_1 != NULL )
	{
	pic_new = mitkIpFuncHitMissI( pic_old, mask_1, mitkIpFuncBorderZero );
	mitkIpPicFree ( mask_1 );
	}
	else 
	printf ( " iamge doesn't exist \n" ); 
	 }*/
    else if ( strcasecmp ( operation, "Scale" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != pic_old->dim + 5 ) )  
		{
			printf("Usage: ipFunc Scale infile outfile sc_fact1 ... sc_factn kind   \n");
			printf("  sc_factx must contain a positive float number \n" );
			printf("  kind describes the interpolation used to scale :\n" );
            printf("      next neighbour        :   7 \n" );
            printf("      bilinear              :   6 \n" ); 
			exit(1);
		}
		sc  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpFloat8_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
		{
			sscanf  ( argv[4+i], "%lf", &sc[i] );
		}
        sscanf ( argv[4+pic_old->dim], "%d", &keep );
		pic_new = mitkIpFuncScale ( pic_old, sc, keep );
		pic_ret = NULL;
		free(sc);
	}
    else if ( strcasecmp ( operation, "Transpose" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != pic_old->dim + 4 ) )  
		{
			printf("Usage: ipFunc Transpose infile outfile perm_1 ... perm_n  \n");
			printf("  perm_1 ... perm_n must be a permutation vector, e.g., 4 2 1 3 \n" );
			exit(1);
		}
		perm  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpFloat8_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
		{
			sscanf  ( argv[4+i], "%d", &perm[i] );
		}
		pic_new = mitkIpFuncTranspose ( pic_old, NULL, perm );
		pic_ret = NULL;
		free(perm);
	}
    else if ( strcasecmp ( operation, "Roberts" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Roberts infile outfile dim_mask    \n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &dim_mask );
		border = 3;
		pic_new = mitkIpFuncRoberts ( pic_old, dim_mask, border  );
	}
    else if ( strcasecmp ( operation, "GaussF" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 6 ) )
		{
			printf("Usage: ipFunc GaussF infile outfile len_mask dim_mask    \n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &len_mask );
		sscanf  ( argv[5], "%d", &dim_mask );
		border = 3;
		pic_new = mitkIpFuncGausF ( pic_old, len_mask, dim_mask, border  );
	}
    else if ( strcasecmp ( operation, "Canny" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 7 ) )
		{
			printf("Usage: ipFunc Canny infile outfile len_mask dim_mask threshold  \n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &len_mask );
		sscanf  ( argv[5], "%d", &dim_mask );
		sscanf  ( argv[6], "%lf", &threshold );
		border = 3;
		if ( pic_new != NULL ) mitkIpPicFree ( pic_new );
		pic_new = mitkIpFuncCanny ( pic_old, dim_mask, len_mask,
			threshold, border );
	}
    else if ( strcasecmp ( operation, "Rank" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 7 ) )
		{
			printf("Usage: ipFunc Rank infile outfile len_mask dim_mask rank  \n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &len_mask );
		sscanf  ( argv[5], "%d", &dim_mask );
		sscanf  ( argv[6], "%d", &rank );
		border = 3;
		pic_new = mitkIpFuncRank ( pic_old, rank, dim_mask, len_mask, border  );
	}
    else if ( strcasecmp ( operation, "MeanF" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 6 ) )
		{
			printf("Usage: ipFunc MeanF infile outfile len_mask dim_mask  \n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &len_mask );
		sscanf  ( argv[5], "%d", &dim_mask );
		border = 3;
		pic_new = mitkIpFuncMeanF ( pic_old, len_mask, dim_mask, border  );
	}
    else if ( strcasecmp ( operation, "Shp" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 6 ) )
		{
			printf("Usage: ipFunc Shp infile outfile len_mask dim_mask  \n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &len_mask );
		sscanf  ( argv[5], "%d", &dim_mask );
		border = 3;
		pic_new = mitkIpFuncShp ( pic_old, dim_mask, border, len_mask  );
	}
    else if ( strcasecmp ( operation, "Laplace" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Laplace infile outfile dim_mask  \n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &dim_mask );
		border = 3;
		pic_new = mitkIpFuncLaplace ( pic_old, dim_mask, border  );
	}
    else if ( strcasecmp ( operation, "Sobel" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Sobel infile outfile dim_mask  \n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &dim_mask );
		border = 3;
		pic_new = mitkIpFuncSobel ( pic_old, dim_mask, border  );
	}
    else if ( strcasecmp ( operation, "Grad" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Grad infile outfile dim_mask  \n");
			exit(1);
		}
		sscanf  ( argv[4], "%d", &dim_mask );
		border = 3;
		pic_new = mitkIpFuncGrad ( pic_old, dim_mask, border  );
	}
    else if ( strcasecmp ( operation, "Thresh" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Thresh infile outfile threshold \n");
			exit(1);
		}
		sscanf  ( argv[4], "%lf", &threshold );
		pic_new = mitkIpFuncThresh ( pic_old, threshold, pic_ret );
	}
    else if ( strcasecmp ( operation, "Conv" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 5 ) )
		{
			printf("Usage: ipFunc Conv infile outfile mask     \n");
			printf("  mask must be the name of a PIC-file containing the mask \n" );
			exit(1);
		}
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		border = mitkIpFuncBorderZero;
		
		pic_mask = mitkIpPicGet ( mask_name, NULL );
		if ( pic_mask != NULL )
		{
			pic_new = mitkIpFuncConv ( pic_old, pic_mask, border );
			mitkIpPicFree ( pic_mask );
		}
	}
    else if ( strcasecmp ( operation, "ZeroCr" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{
			printf ( " ipFunc ZeroCr infile outfile \n" );
			exit ( 1 );
		}
		pic_new = mitkIpFuncZeroCr ( pic_old );
	}
    else if ( strcasecmp ( operation, "Exp" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{
			printf ( " ipFunc Exp infile outfile \n" );
			exit ( 1 );
		}
		pic_new = mitkIpFuncExp ( pic_old, mitkIpFuncMinMax, pic_ret );
	}
    else if ( strcasecmp ( operation, "Log" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{
			printf ( " ipFunc Log infile outfile \n" );
			exit ( 1 );
		}
		pic_new = mitkIpFuncLog ( pic_old );
	}
    else if ( strcasecmp ( operation, "LN" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{
			printf ( " ipFunc LN infile outfile \n" );
			exit ( 1 );
		}
		pic_new = mitkIpFuncLN ( pic_old );
	}
    else if ( strcasecmp ( operation, "Norm" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{
			printf ( " ipFunc Norm infile outfile \n" );
			exit ( 1 );
		}
		pic_new = mitkIpFuncNorm ( pic_old, pic_ret );
	}
    else if ( strcasecmp ( operation, "NormXY" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 6 ) )
		{
			printf("Usage: ipFunc NormXY infile outfile lower_value upper_value\n");
			exit(1);
		}
		sscanf  ( argv[4],"%lf", &min_gv );
		sscanf  ( argv[5],"%lf", &max_gv );
		pic_new = mitkIpFuncNormXY ( pic_old, min_gv, max_gv, pic_ret );
	}
    else if ( strcasecmp ( operation, "Equal" ) == 0 )
	{
        if ( (unsigned int) argc != 4 )
		{
			printf ( " ipFunc Equal infile outfile \n" );
			exit ( 1 );
		}
		keep    = mitkIpFuncMinMax;
		pic_new = mitkIpFuncEqual ( pic_old, keep, pic_ret );
	}
    else if ( strcasecmp ( operation, "Extr" ) == 0 )
	{
        if ( ( (unsigned int) argc != 3 ) || ( strcasecmp (argv[2], "-h") == 0 ) )
		{
			printf ( " ipFunc Extr infile  \n" );
			exit ( 1 );
		}
		mitkIpFuncExtr ( pic_old, &min, &max );
        picput_flag = mitkIpFalse;
		printf  ( " min: %12.2lf max: %12.2lf \n", min, max );
	}
#ifdef MESCHACH
    else if ( strcasecmp ( operation, "Inertia" ) == 0 )
	{
        if ( ( (unsigned int) argc != 3 ) || ( strcasecmp (argv[2], "-h") == 0 ) )
		{
			printf ( " ipFunc Inertia infile  \n" );
			exit ( 1 );
		}
		mitkIpFuncInertia ( pic_old, &inertia, &ev );
        picput_flag = mitkIpFalse;
        for ( i = 0; i < pic_old->dim; i++ ) 
			printf  ( " eigenvalue[%d] = %lf \n", i, ev[i] );
        for ( i = 0; i < pic_old->dim * pic_old->dim; i++ ) 
			printf  ( " inertia[%d] = %lf \n", i, inertia[i] );
	}
#endif
    else if ( strcasecmp ( operation, "Grav" ) == 0 )
	{
        if ( ( (unsigned int) argc != 3 ) || ( strcasecmp (argv[2], "-h") == 0 ) )
		{
			printf ( " ipFunc Grav infile  \n" );
			exit ( 1 );
		}
		grav = mitkIpFuncGrav ( pic_old );
        picput_flag = mitkIpFalse;
        for ( i = 0; i < pic_old->dim; i++ ) 
			printf  ( " center of gravity[%d] = %lf \n", i, grav[i] );
	}
    else if ( strcasecmp ( operation, "Median" ) == 0 )
	{
        if ( ( (unsigned int) argc != 3 ) || ( strcasecmp (argv[2], "-h") == 0 ) )
		{
			printf ( " ipFunc Median infile  \n" );
			exit ( 1 );
		}
		mean = mitkIpFuncMedI ( pic_old );
		printf  ( " median: %lf\n", mean );
	}
    else if ( strcasecmp ( operation, "Mean" ) == 0 )
	{
        if ( ( (unsigned int) argc != 3 ) || ( strcasecmp (argv[2], "-h") == 0 ) )
		{
			printf ( " ipFunc Mean infile  \n" );
			exit ( 1 );
		}
		mean = mitkIpFuncMean ( pic_old );
		printf  ( " mean: %lf\n", mean );
	}
    else if ( strcasecmp ( operation, "Var" ) == 0 )
	{
        if ( ( (unsigned int) argc != 3 ) || ( strcasecmp (argv[2], "-h") == 0 ) )
		{
			printf ( " ipFunc Var infile  \n" );
			exit ( 1 );
		}
		var = mitkIpFuncVar ( pic_old );
		printf  ( " variance : %lf\n", var );
	}
    else if ( strcasecmp ( operation, "SDev" ) == 0 )
	{
        if ( ( (unsigned int) argc != 3 ) || ( strcasecmp (argv[2], "-h") == 0 ) )
		{
			printf ( " ipFunc SDev infile  \n" );
			exit ( 1 );
		}
		s_der = mitkIpFuncSDev ( pic_old );
		printf  ( " standard derivation : %lf\n", s_der );
	}
    else if ( strcasecmp ( operation, "SelMM" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 6 ) )
		{
			printf("Usage: ipFunc SelMM infile outfile lower_value upper_value \n");
			exit(1);
		}
		sscanf  ( argv[4], "%lf", &gv_low );
		sscanf  ( argv[5], "%lf", &gv_up );
		pic_new = mitkIpFuncSelMM ( pic_old, gv_low, gv_up, pic_ret );
	}
    /*
	else if ( strcasecmp ( operation, "sel_mm_inv" ) == 0 )
	{
	printf ( " lower greyvalue : " );
	scanf  ( "%lf", &gv_low );
	printf ( " upper greyvalue : " );
	scanf  ( "%lf", &gv_up );
	printf ( " lower greyvalue (new): " );
	scanf  ( "%lf", &gv_new_low );
	printf ( " upper greyvalue (new): " );
	scanf  ( "%lf", &gv_new_up );
	pic_new = mitkIpFuncSelMM_Inv ( pic_old, gv_low, gv_up, gv_new_low, gv_new_up, pic_ret );
	}
    */
    else if ( strcasecmp ( operation, "Select" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 7 ) )
		{
			printf("Usage: ipFunc Select infile outfile lower_value upper_value new_value\n");
			exit(1);
		}
		sscanf  ( argv[4], "%lf", &gv_low );
		sscanf  ( argv[5], "%lf", &gv_up );
		sscanf  ( argv[6], "%lf", &gv );
		pic_new = mitkIpFuncSelect ( pic_old, gv_low, gv_up, gv, pic_ret );
	}
    else if ( strcasecmp ( operation, "SelInv" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 7 ) )
		{
			printf("Usage: ipFunc SelInv infile outfile lower_value upper_value new_value\n");
			exit(1);
		}
		sscanf  ( argv[4], "%lf", &gv_low );
		sscanf  ( argv[5], "%lf", &gv_up );
		sscanf  ( argv[6], "%lf", &gv );
		pic_new = mitkIpFuncSelInv ( pic_old, gv_low, gv_up, gv, pic_ret );
	}
    else if ( strcasecmp ( operation, "LevWin" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) || 
			( (unsigned int) argc != 6 ) )
		{
			printf("Usage: ipFunc LevWin infile outfile level window \n" );
			exit(1);
		}
		sscanf  ( argv[4], "%lf", &gv_low );
		sscanf  ( argv[5], "%lf", &gv_up );
		pic_new = mitkIpFuncLevWin ( pic_old, gv_low, gv_up, pic_ret );
	}
    else if ( strcasecmp ( operation, "Box" ) == 0 )
	{
        if ( ( (unsigned int) argc != 3 ) || ( strcasecmp (argv[2], "-h") == 0 ) )
		{
			printf ( " usage: ipFunc Box infile \n" );
			exit ( 1 );
		}
        picput_flag = mitkIpFalse; length=NULL;
        mitkIpFuncBox ( pic_old, &begin, &length );
        for ( i =0; i < pic_old->dim; i++ )
			printf ( " [%d]\t links oben  %d \t\t rechts unten: %d \n", i, begin[i], length[i] );
	}
    else if ( strcasecmp ( operation, "BorderX" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != pic_old->dim + 5 ) )  
		{
			printf ( " usage: ipFunc BorderX infile outfile edge1...edgen value\n" );
			exit ( 1 );
		}
		begin  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
		{
			sscanf  ( argv[4+i], "%d", &begin[i] );
		}
        sscanf ( argv[4+pic_old->dim], "%lf", &value );
        pic_mask = mitkIpPicNew ();
        pic_mask->type = 3;
        pic_mask->bpe  = 16;
        pic_mask->dim  = pic_old->dim;  
		for ( i = 0; i < pic_old->dim; i++ )
			pic_mask->n[i] = begin[i] *2 + 1;
		pic_new = mitkIpFuncBorderX ( pic_old, pic_mask, value, NULL );                
        mitkIpPicFree ( pic_mask );
		free ( begin );
	}
    else if ( strcasecmp ( operation, "Frame" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != pic_old->dim + 5 ) )  
		{
			printf ( " usage: ipFunc Edge infile outfile edge1...edgen value\n" );
			exit ( 1 );
		}
		begin  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
		{
			sscanf  ( argv[4+i], "%d", &begin[i] );
		}
        sscanf ( argv[4+pic_old->dim], "%lf", &value );
		pic_new = mitkIpFuncFrame ( pic_old, begin, value );                
		free ( begin );
	}
    else if ( strcasecmp ( operation, "WindowR" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != pic_old->dim + 5 ) )  
		{
			printf ( " usage: ipFunc WindowR infile outfile image beg1...begn \n" );
			exit ( 1 );
		}
		keep = mitkIpFuncKeep; 
		strcpy ( mask_name, "" );
		sscanf  ( argv[4], "%s", mask_name );
		pic_hlp = mitkIpPicGet ( mask_name, NULL );
		begin  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
		{
			sscanf  ( argv[5+i], "%d", &begin[i] );
		}
		pic_new = mitkIpFuncWindowR ( pic_old, pic_hlp, begin, mitkIpFuncNoKeep );
		free ( begin );
	}
    else if ( strcasecmp ( operation, "Rotate" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != pic_old->dim * 2 + 4 ) )  
		{
			printf ( " usage: ipFunc Rotate infile outfile grad1...gradn order1...ordern \n" );
			exit ( 1 );
		}
		grad   = malloc ( _mitkIpPicNDIM * sizeof ( int ) );
		order  = malloc ( _mitkIpPicNDIM * sizeof ( int ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &grad[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+pic_old->dim+i], "%d", &order[i] );
		pic_new = mitkIpFuncRotate ( pic_old, NULL, grad, order );
		free ( grad );
		free ( order );
	}
    else if ( strcasecmp ( operation, "Window" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != pic_old->dim * 2 + 4 ) )  
		{
			printf ( " usage: ipFunc Window infile outfile beg1...begn length1...lengthn \n" );
			exit ( 1 );
		}
		begin  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+pic_old->dim+i], "%d", &length[i] );
        printf ( " %d %d %d       %d %d %d \n", begin[0], begin[1], begin[2], length[0], length[1], length[2] );
		pic_new = mitkIpFuncWindow ( pic_old, begin, length );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "DrawPoly" ) == 0 )
	{
	/*
	if ( ( (unsigned int) argc == 2 ) || 
	( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
	( (unsigned int) argc != 2 * pic_old->dim + 5 ) )  
        */
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) )
		{
			printf ( " usage: ipFunc Window infile outfile nr_points x1...xn y1...yn \n" );
			exit ( 1 );
		}
		sscanf ( argv[4], "%d", &radius );
		begin  = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < radius; i++ )
			sscanf  ( argv[5+i], "%d", &begin[i] );
		for ( i = 0; i < radius; i++ )
			sscanf  ( argv[5+radius+i], "%d", &length[i] );
		pic_new = mitkIpFuncDrawPoly ( pic_old, begin, length, radius );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "MeanROI" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 4 + pic_old->dim * 2 ) )  
		{
			printf ( " usage: ipFunc MeanROI infile nr_points x1...xn y1...yn \n" );
			exit ( 1 );
		}
		sscanf ( argv[3], "%d", &radius );
		begin  = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+pic_old->dim+i], "%d", &length[i] );
		mean = mitkIpFuncMeanROI ( pic_old, begin, length, radius );
		printf ( " mean: %lf \n", mean );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "VarROI" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 4 + pic_old->dim * 2 ) )  
		{
			printf ( " usage: ipFunc VarROI infile nr_points x1...xn y1...yn \n" );
			exit ( 1 );
		}
		sscanf ( argv[3], "%d", &radius );
		begin  = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+pic_old->dim+i], "%d", &length[i] );
		var = mitkIpFuncVarROI ( pic_old, begin, length, radius );
		printf ( " var: %lf \n", var );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "SDevROI" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 4 + pic_old->dim * 2 ) )  
		{
			printf ( " usage: ipFunc SDevROI infile nr_points x1...xn y1...yn \n" );
			exit ( 1 );
		}
		sscanf ( argv[3], "%d", &radius );
		begin  = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+pic_old->dim+i], "%d", &length[i] );
		var = mitkIpFuncSDevROI ( pic_old, begin, length, radius );
		printf ( " var: %lf \n", var );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "ExtrROI" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 4 + pic_old->dim * 2 ) )  
		{
			printf ( " usage: ipFunc ExtrROI infile nr_points x1...xn y1...yn \n" );
			exit ( 1 );
		}
		sscanf ( argv[3], "%d", &radius );
		begin  = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( radius * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+pic_old->dim+i], "%d", &length[i] );
		mitkIpFuncExtrROI ( pic_old, &min, &max, begin, length, radius );
		printf ( " min: %lf max: %lf\n", min, max );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "VarR" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 3 + 2 * pic_old->dim ) )  
		{
			printf ( " usage: ipFunc VarR infile  beg1...begn length1...lengthn \n" );
			exit ( 1 );
		}
		begin  = malloc ( pic_old->dim * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( pic_old->dim * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[3+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[3+pic_old->dim+i], "%d", &length[i] );
		var  = mitkIpFuncVarR ( pic_old, begin, length );
		printf ( " var: %lf \n", var );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "SDevR" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 3 + 2 * pic_old->dim ) )  
		{
			printf ( " usage: ipFunc SDevR infile  beg1...begn length1...lengthn \n" );
			exit ( 1 );
		}
		begin  = malloc ( pic_old->dim * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( pic_old->dim * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[3+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[3+pic_old->dim+i], "%d", &length[i] );
		var  = mitkIpFuncVarR ( pic_old, begin, length );
		s_der = mitkIpFuncSDevR ( pic_old, begin, length );
		printf ( " s_der: %lf \n", s_der );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "MeanR" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 3 + 2 * pic_old->dim ) )  
		{
			printf ( " usage: ipFunc MeanR infile  beg1...begn length1...lengthn \n" );
			exit ( 1 );
		}
		begin  = malloc ( pic_old->dim * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( pic_old->dim * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[3+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[3+pic_old->dim+i], "%d", &length[i] );
		var  = mitkIpFuncVarR ( pic_old, begin, length );
		mean = mitkIpFuncMeanR ( pic_old, begin, length );
		printf ( " mean: %lf \n", mean );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "ExtrR" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 3 + 2 * pic_old->dim ) )  
		{
			printf ( " usage: ipFunc ExtrR infile  beg1...begn length1...lengthn \n" );
			exit ( 1 );
		}
		begin  = malloc ( pic_old->dim * sizeof ( mitkIpUInt4_t ) );
		length = malloc ( pic_old->dim * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[3+i], "%d", &begin[i] );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[3+pic_old->dim+i], "%d", &length[i] );
		var  = mitkIpFuncVarR ( pic_old, begin, length );
		mitkIpFuncExtrR ( pic_old, &min, &max, begin, length );
		printf ( " min: %lf max: %lf \n", min, max );
		free ( begin );
		free ( length );
	}
    else if ( strcasecmp ( operation, "SDevC" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc !=  4 +  pic_old->dim ) )  
		{
			printf ( " usage: ipFunc SDevR infile radius begin1...beginn \n" );
			exit ( 1 );
		}
		begin  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &begin[i] );
		sscanf  ( argv[3], "%d", &radius );
		s_der = mitkIpFuncSDevC ( pic_old, begin, radius );
		printf ( " s_dev: %lf \n", s_der );
		free ( begin );
	}
    else if ( strcasecmp ( operation, "VarC" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 4 + pic_old->dim  ) )  
		{
			printf ( " usage: ipFunc VarC infile radius begin1...beginn \n" );
			exit ( 1 );
		}
		begin  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &begin[i] );
		sscanf  ( argv[3], "%d", &radius );
		var = mitkIpFuncVarC ( pic_old, begin, radius );
		printf ( " var: %lf \n", var );
		free ( begin );
	}
    else if ( strcasecmp ( operation, "MeanC" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc != 4 +  pic_old->dim ) )  
		{
			printf ( " usage: ipFunc MeanC infile radius begin1...beginn \n" );
			exit ( 1 );
		}
		begin  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			sscanf  ( argv[4+i], "%d", &begin[i] );
		sscanf  ( argv[3], "%d", &radius );
		mean = mitkIpFuncMeanC ( pic_old, begin, radius );
		printf ( " mean: %lf \n", mean );
		free ( begin );
	}
    else if ( strcasecmp ( operation, "ExtrC" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc !=  4 +  pic_old->dim ) )  
		{
			printf ( " usage: ipFunc ExtrC infile radius begin1...beginn \n" );
			exit ( 1 );
		}
		begin  = malloc ( _mitkIpPicNDIM * sizeof ( mitkIpUInt4_t ) );
		for ( i = 0; i < pic_old->dim; i++ )
			scanf  ( argv[4+i], "%d", &begin[i] );
		sscanf  ( argv[3], "%d", &radius );
		mitkIpFuncExtrC ( pic_old, &min, &max, begin, radius );
		printf ( " min: %lf max: %lf \n", min, max );
		free ( begin );
	}
    else if ( strcasecmp ( operation, "MakeMorphSquareMask" ) == 0 )
	{
		if ( ( (unsigned int) argc == 2 ) || 
			( ( (unsigned int) argc == 3 ) && ( strcasecmp (argv[2], "-h") == 0 ) ) ||
			( (unsigned int) argc !=  6 ) )  
		{
			printf ( " usage: ipFunc MakeMorphSquareMask infile outfile dim size\n" );
			exit ( 1 );
		}
		sscanf  ( argv[4], "%d", &dim_mask );
		sscanf  ( argv[5], "%d", &mask_size );

		pic_new=mitkIpPicCopyHeader(pic_old,NULL);
		pic_new->dim=dim_mask;
		for ( i = 0; i < pic_old->dim; i++ )
			pic_new->n[i]=mask_size;
		pic_new->data  = calloc ( _mitkIpPicSize(pic_new), 1 );
		mitkIpFuncAddC(pic_new, 1.0, mitkIpFuncKeep, pic_new);
	}
    else
		printf ( " illegal operation \n" );
	
	
    if ( mitkIpFuncErrno > mitkIpOK )
	{
        mitkIpFuncPError ( error_nr );
        exit ( 1 );
	}
    if ( ( pic_new != 0 ) && ( picput_flag ) )
	{
        strcpy ( pic_name_t, argv[3] );                   
		mitkIpPicPut ( pic_name_t, pic_new );
        picput_flag = mitkIpTrue;
	}
	
	
    if ( pic_ret )      
	{
		mitkIpPicFree ( pic_ret );
		pic_old = NULL;
		pic_ret = NULL;
		pic_new = NULL;
	}
    else
	{
		if ( pic_old != NULL ) mitkIpPicFree  ( pic_old );
		pic_old = NULL;
		if ( pic_new != NULL ) mitkIpPicFree  ( pic_new );
		pic_new = NULL;
		if ( pic_hlp != NULL ) mitkIpPicFree  ( pic_hlp );
		pic_hlp = NULL;
	}

    return 0;
}

