#include "Pic2vtk.h"
#include <vtkStructuredPointsReader.h>
#include <vtkImageCast.h> 


/**
 * konvertiert ein vtkImageReader in ein pic-Image 
 */ 
ipPicDescriptor* Pic2vtk::convert( vtkImageData* vtkImage ) {

	if ( vtkImage == NULL )
		return NULL;

	ipPicDescriptor* pic = ipPicNew();

	if ( pic == NULL )
		return NULL;

	int dim[3];
	vtkImage->GetDimensions(dim);

	if ( dim[2] > 1 ) {
		pic->dim = 3;		
	} else if( dim[1] > 1 ) {
		pic->dim = 2;
	} else if ( dim[1] > 1 ) {
		pic->dim = 1;
	}

	pic->n[0] = dim[0];
	pic->n[1] = dim[1];
	pic->n[2] = dim[2];

	register unsigned long size = dim[0] * dim[1] * dim[2];

	switch ( vtkImage->GetScalarType () ) {

	case VTK_BIT: {

		pic->type = ipPicInt;
		pic->bpe = 8;
		unsigned int vtkSize = size>>3;

		if (vtkSize%8)
			vtkSize++;

		register unsigned char *s = (unsigned char*) vtkImage->GetScalarPointer();	
		register unsigned char *a = new unsigned char[size];
		pic->data = a;

		for ( register unsigned int i = 0; i < vtkSize; i++ ) {

			for ( int k=0; k < 8; k++ ) 
				*a++ = 0x80 & (*s<<k);	

			s++;
		}

		break;
	}
	case VTK_CHAR: {

		pic->type = ipPicInt;
		pic->bpe = 8;

		register char *s = (char*) vtkImage->GetScalarPointer();	
		register char *a = new char[size];
		pic->data = a;

		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	case VTK_UNSIGNED_CHAR: {

		pic->type = ipPicUInt;
		pic->bpe = 8;

		register unsigned char *s = (unsigned char*) vtkImage->GetScalarPointer();	
		register unsigned char *a = new unsigned char[size];
		pic->data = a;

		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	case VTK_SHORT: {

		pic->type = ipPicInt;
		pic->bpe = 16;

		register short *s = (short*) vtkImage->GetScalarPointer();	
		register short *a = new short[size];
		pic->data = a;

		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	case VTK_UNSIGNED_SHORT: {

		pic->type = ipPicUInt;
		pic->bpe = 16;

		register unsigned short *s = (unsigned short*) vtkImage->GetScalarPointer();	
		register unsigned short *a = new unsigned short[size];
		pic->data = a;


		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	case VTK_INT: {

		pic->type = ipPicInt;
		pic->bpe = 32;

		register int *s = (int*) vtkImage->GetScalarPointer();	
		register int *a = new int[size];
		pic->data = a;

		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	case VTK_UNSIGNED_INT: {

		pic->type = ipPicUInt;
		pic->bpe = 32;

		register unsigned int *s = (unsigned int*) vtkImage->GetScalarPointer();	
		register unsigned int *a = new unsigned int[size];
		pic->data = a;

		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	case VTK_LONG: {

		pic->type = ipPicInt;
		pic->bpe = 64;

		register long *s = (long*) vtkImage->GetScalarPointer();	
		register long *a = new long[size];
		pic->data = a;

		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	case VTK_UNSIGNED_LONG: {

		pic->type = ipPicUInt;
		pic->bpe = 64;

		register unsigned long *s = (unsigned long*) vtkImage->GetScalarPointer();	
		register unsigned long *a = new unsigned long[size];
		pic->data = a;

		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	case VTK_FLOAT: {

		pic->type = ipPicFloat;
		pic->bpe = 32;

		register float *s = (float*) vtkImage->GetScalarPointer();	
		register float *a = new float[size];
		pic->data = a;

		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	case VTK_DOUBLE: {

		pic->type = ipPicFloat;
		pic->bpe = 64;

		register double *s = (double*) vtkImage->GetScalarPointer();	
		register double *a = new double[size];
		pic->data = a;


		for ( unsigned long i = 0; i < size; i++ )
			*a++ = *s++;

		break;
	}
	default:
		ipPicFree( pic );
		return NULL;	
	}

	return pic;
}


/**
 * konvertiert ein pic-Image in ein vtkImageReader
 */ 
vtkImageData* Pic2vtk::convert( ipPicDescriptor* pic ) {

	if ( pic == NULL )
		return NULL;

	vtkImageData *inData = vtkImageData::New();

	if ( inData == NULL )
		return NULL;

	unsigned long size = 0;

	if ( pic->dim == 1 ) {
		inData->SetDimensions( pic->n[0] -1, 1, 1);		
		size = pic->n[0];		
		inData->SetOrigin( ((float) pic->n[0]) / 2.0f, 0, 0 );		
	} else if ( pic->dim == 2 ) {
		inData->SetDimensions( pic->n[0] , pic->n[1] , 1 );
		size = pic->n[0] * pic->n[1];
		inData->SetOrigin( ((float) pic->n[0]) / 2.0f, ((float) pic->n[1]) / 2.0f, 0 );
	} else if ( pic->dim >= 3 ) {
		inData->SetDimensions( pic->n[0], pic->n[1], pic->n[2] );
		size = pic->n[0] * pic->n[1] * pic->n[2];
		// Test
		//inData->SetOrigin( (float) pic->n[0] / 2.0f, (float) pic->n[1] / 2.0f, (float) pic->n[2] / 2.0f );
		inData->SetOrigin( 0, 0, 0 );
	} else {
		inData->Delete () ;
		return NULL;
	}	

	inData->SetNumberOfScalarComponents(1);


	if ( ( pic->type == ipPicInt || pic->type == ipPicUInt ) && pic->bpe == 1 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();
		size = size>>3;
		register unsigned char* s = (unsigned char*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = *s++;

	} else if ( pic->type == ipPicInt && pic->bpe == 8 ) {
		inData->SetScalarType( VTK_CHAR );
		inData->AllocateScalars();

		register char* s = (char*) pic->data;
		register char* a = (char*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = *s++;

	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {
		inData->SetScalarType( VTK_UNSIGNED_CHAR );
		inData->AllocateScalars();

		register unsigned char* s = (unsigned char*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = *s++;

		
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {
		inData->SetScalarType( VTK_SHORT );
		inData->AllocateScalars();

		register short* s = (short*) pic->data;
		register short* a = (short*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = *s++;

	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {
		inData->SetScalarType( VTK_UNSIGNED_SHORT );
		inData->AllocateScalars();

		register unsigned short* s = (unsigned short*) pic->data;
		register unsigned short* a = (unsigned short*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ ) 
			*a++ = *s++;

	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {
		inData->SetScalarType( VTK_INT );
		inData->AllocateScalars();

		register int* s = (int*) pic->data;
		register int* a = (int*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ ) 
			*a++ = *s++;

	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {
		inData->SetScalarType( VTK_UNSIGNED_INT );
		inData->AllocateScalars();

		register unsigned int* s = (unsigned int*) pic->data;
		register unsigned int* a = (unsigned int*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = *s++;

	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {
		inData->SetScalarType( VTK_LONG );
		inData->AllocateScalars();

		register long* s = (long*) pic->data;
		register long* a = (long*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = *s++;

	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {
		inData->SetScalarType( VTK_UNSIGNED_LONG );
		inData->AllocateScalars();

		register unsigned long* s = (unsigned long*) pic->data;
		register unsigned long* a = (unsigned long*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = *s++;

	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {
		inData->SetScalarType( VTK_FLOAT );
		inData->AllocateScalars();

		register float* s = (float*) pic->data;
		register float* a = (float*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = *s++;

	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {
		inData->SetScalarType( VTK_DOUBLE );
		inData->AllocateScalars();

		register double* s = (double*) pic->data;
		register double* a = (double*) inData->GetScalarPointer();	

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = *s++;

	} else {
		inData->Delete();
		return NULL;
	}    
	    
	return inData;
}



/**
 * läd ein unkomprimiertes pic file
 */ 
vtkImageData* Pic2vtk::loadImage( char* fileName, bool littleEndian ) {

	if ( fileName == NULL )
		return NULL;

	ipPicDescriptor* pic = ipPicGetHeader( fileName, NULL );

	if ( pic == NULL )
		return NULL;

	if ( pic->info->pixel_start_in_file == 0 ) {
		// komprimierte Date
		
		ipPicFree( pic );
		pic = NULL;				
		
		pic = ipPicGet( fileName, NULL );

		if ( pic == NULL )
			return NULL;

		vtkImageData* vtkImage = convert( pic );	
		ipPicFree( pic );
		return vtkImage;
	}

	vtkImageReader *reader = vtkImageReader::New();

	if ( reader == NULL )
		return NULL;

	reader->ReleaseDataFlagOff();

	if ( littleEndian == true )
		reader->SetDataByteOrderToLittleEndian();
	else
		reader->SetDataByteOrderToBigEndian();

	if ( pic->dim == 1 ) {
		reader->SetFileDimensionality(1);
		reader->SetDataExtent ( 0, pic->n[0] -1, 0, 0, 0, 0);
		reader->SetDataOrigin( ((float) pic->n[0]) / 2.0f, 0, 0 );		
	} else if ( pic->dim == 2 ) {
		reader->SetFileDimensionality(2);
		reader->SetDataExtent ( 0, pic->n[0] -1, 0, pic->n[1] -1, 0, 0);
		reader->SetDataOrigin( ((float) pic->n[0]) / 2.0f, ((float) pic->n[1]) / 2.0f, 0 );
	} else if ( pic->dim >= 3 ) {
		reader->SetFileDimensionality(3);
		reader->SetDataOrigin( (float) pic->n[0] / 2.0f, (float) pic->n[1] / 2.0f, (float) pic->n[2] / 2.0f );
		reader->SetDataExtent( 0, pic->n[0] - 1, 0, pic->n[1] - 1, 0, pic->n[2] - 1 );
	} else {
		reader->Delete () ;
		ipPicFree( pic );
		return NULL;
	}

	// Datatype 
	if ( ( pic->type == ipPicInt || pic->type == ipPicUInt ) && pic->bpe == 1 ) {
		reader->SetDataScalarType( VTK_BIT );

	} else if ( pic->type == ipPicInt && pic->bpe == 8 ) {
		reader->SetDataScalarType( VTK_CHAR );

	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {
		reader->SetDataScalarType( VTK_UNSIGNED_CHAR );
		
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {
		reader->SetDataScalarType( VTK_SHORT );

	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {
		reader->SetDataScalarType( VTK_UNSIGNED_SHORT );

	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {
		reader->SetDataScalarType( VTK_INT );

	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {
		reader->SetDataScalarType( VTK_UNSIGNED_INT );

	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {
		reader->SetDataScalarType( VTK_LONG );

	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {
		reader->SetDataScalarType( VTK_UNSIGNED_LONG );

	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {
		reader->SetDataScalarType( VTK_FLOAT );

	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {
		reader->SetDataScalarType( VTK_DOUBLE );

	} else {
		reader->Delete();
		ipPicFree( pic );
		return NULL;
	}

	reader->SetDataMask(0xffff);
	reader->SetHeaderSize ( pic->info->pixel_start_in_file );

	reader->SetFileName( fileName );
	reader->UpdateWholeExtent();
	ipPicFree( pic );

	vtkImageData* vtkImage = reader->GetOutput();
	reader->Delete();
	return vtkImage; 
}


/**
 * konvertiert ein pic-Image in ein vtkImageReader
 * mask erwartet ein Bittmuster, das im Zielbild eingeblendet wird.
 * move gibt die Verschiebung an, mit der das Bitmuster im Zielbilt plaziert wird.
 */ 
vtkImageData* Pic2vtk::convert( ipPicDescriptor* pic, unsigned long mask, int move ) {

	if ( pic == NULL )
		return NULL;

	vtkImageData *inData = vtkImageData::New();

	if ( inData == NULL )
		return NULL;

	unsigned long size = 0;

	if ( pic->dim == 1 ) {

		inData->SetDimensions( pic->n[0] -1, 1, 1);		
		size = pic->n[0];		
		inData->SetOrigin( ((float) pic->n[0]) / 2.0f, 0, 0 );		
	} else if ( pic->dim == 2 ) {

		inData->SetDimensions( pic->n[0] , pic->n[1] , 1 );
		size = pic->n[0] * pic->n[1];
		inData->SetOrigin( ((float) pic->n[0]) / 2.0f, ((float) pic->n[1]) / 2.0f, 0 );
	} else if ( pic->dim >= 3 ) {

		inData->SetDimensions( pic->n[0], pic->n[1], pic->n[2] );
		size = pic->n[0] * pic->n[1] * pic->n[2];
		inData->SetOrigin( 0, 0, 0 );
	} else {

		inData->Delete () ;
		return NULL;
	}	

	inData->SetNumberOfScalarComponents(1);


	if ( ( pic->type == ipPicInt || pic->type == ipPicUInt ) && pic->bpe == 1 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();

		size = size>>3;
		register unsigned char* s = (unsigned char*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	
		register unsigned char v;
		
		for ( register unsigned long i = 0; i < size ; i++ ) {

			v = 0;

			for ( int j = 0; j < 8; j++ )			
				v |= (((mask & (*s++))>>move)<<j);
			 
			*a++ = v;
		}

	} else if ( pic->type == ipPicInt && pic->bpe == 8 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();

		register char* s = (char*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	
		register unsigned char v;


		for ( register unsigned long i = 0; i < size ; i++ ) {

			v = 0;

			for ( int j = 0; j < 8; j++ )			
				v |= (((mask & (*s++))>>move)<<j);
			 
			*a++ = v;
		}

	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();

		register unsigned char* s = (unsigned char*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	
		register unsigned char v;

		for ( register unsigned long i = 0; i < size ; i++ ) {

			v = 0;

			for ( int j = 0; j < 8; j++ )			
				v |= (((mask & (*s++))>>move)<<j);
			 
			*a++ = v;
		}

		
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();

		register short* s = (short*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	
		register unsigned char v;

		for ( register unsigned long i = 0; i < size ; i++ ) {

			v = 0;

			for ( int j = 0; j < 8; j++ )			
				v |= (((mask & (*s++))>>move)<<j);
			 
			*a++ = v;
		}

	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();

		register unsigned short* s = (unsigned short*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	
		register unsigned char v;

		for ( register unsigned long i = 0; i < size ; i++ ) {

			v = 0;

			for ( int j = 0; j < 8; j++ )			
				v |= (((mask & (*s++))>>move)<<j);
			 
			*a++ = v;
		}

	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();

		register int* s = (int*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	
		register unsigned char v;

		for ( register unsigned long i = 0; i < size ; i++ ) {

			v = 0;

			for ( int j = 0; j < 8; j++ )			
				v |= (((mask & (*s++))>>move)<<j);
			 
			*a++ = v;
		}

	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();

		register unsigned int* s = (unsigned int*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	
		register unsigned char v;

		for ( register unsigned long i = 0; i < size ; i++ ) {

			v = 0;

			for ( int j = 0; j < 8; j++ )			
				v |= (((mask & (*s++))>>move)<<j);
			 
			*a++ = v;
		}

	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();

		register long* s = (long*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	
		register unsigned char v;

		for ( register unsigned long i = 0; i < size ; i++ ) {

			v = 0;

			for ( int j = 0; j < 8; j++ )			
				v |= (((mask & (*s++))>>move)<<j);
			 
			*a++ = v;
		}

	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {
		inData->SetScalarType( VTK_BIT );
		inData->AllocateScalars();

		register unsigned long* s = (unsigned long*) pic->data;
		register unsigned char* a = (unsigned char*) inData->GetScalarPointer();	
		register unsigned char v;

		for ( register unsigned long i = 0; i < size ; i++ ) {

			v = 0;

			for ( int j = 0; j < 8; j++ )			
				v |= (((mask & (*s++))>>move)<<j);
			 
			*a++ = v;
		}

	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {

		inData->Delete();
		return NULL;

	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {

		inData->Delete();
		return NULL;

	} else {
		inData->Delete();
		return NULL;
	}    
	    
	return inData;
}

/**
 * 
 */
 ipPicDescriptor* Pic2vtk::convertSigned2Unsigned( ipPicDescriptor* pic, unsigned long* offset  ) {
 
	if ( pic == NULL )
		return NULL;

	if ( pic->type == ipPicUInt )
		return pic;

	ipPicDescriptor* newPic = ipPicCopyHeader( pic, NULL );	
	const unsigned int size = _ipPicElements( pic );

	if ( pic->type == ipPicInt && pic->bpe == 8 ) {

		newPic->type = ipPicUInt;
		newPic->data = new unsigned char[ size ];

		register char* s = (char*) pic->data;
		register unsigned char* a = (unsigned char*) newPic->data;	
		register unsigned short localOffset = 2^7;

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = (unsigned char) ((*s++) + localOffset);

		if ( offset ) 
			*offset = localOffset;
	
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {

		newPic->type = ipPicUInt;
		newPic->data = new unsigned short[ size ];

		register short* s = (short*) pic->data;
		register unsigned short* a = (unsigned short*) newPic->data;
		register unsigned int localOffset = 1<<15;

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = (unsigned short) ((*s++) + localOffset);

		if ( offset ) 
			*offset = localOffset;

	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {

		newPic->type = ipPicUInt;
		newPic->data = new unsigned int[ size ];

		register int* s = (int*) pic->data;
		register unsigned int* a = (unsigned int*) newPic->data;	
		register unsigned long localOffset = 1<<31;

		for ( register unsigned long i = 0; i < size ; i++ ) 
			*a++ = (unsigned int)((*s++) + localOffset);

		if ( offset ) 
			*offset = localOffset;

	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {

		newPic->type = ipPicUInt;
		newPic->data = new unsigned long[ size ];

		register long* s = (long*) pic->data;
		register unsigned long* a = (unsigned long*) newPic->data;
		register unsigned long localOffset = 1<<63;

		for ( register unsigned long i = 0; i < size ; i++ )
			*a++ = (unsigned long) ((*s++) + localOffset);

		if ( offset ) 
			*offset = localOffset;
	}

	return newPic;
 }


 /**
  *
  */

vtkImageData* Pic2vtk::loadVtkImage( char* fileName ) {
 
	vtkStructuredPointsReader* reader = vtkStructuredPointsReader::New();
	// vtkDataReader* reader = vtkDataReader::New();
	

	reader->SetFileName( fileName );

	if ( reader->OpenVTKFile() == 0 )
		return NULL;

	int test = reader->IsFileStructuredPoints();


	int a = reader->GetFileType();
	char * kk = reader->GetHeader();

	// vtkStructuredPoints* points = reader->GetOutput();
	return NULL;
 }

