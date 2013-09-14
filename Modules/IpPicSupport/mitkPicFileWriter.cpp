/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPicFileWriter.h"
#include "mitkPicFileReader.h"

extern "C"
{
size_t _mitkIpPicFWrite( const void *ptr, size_t size, size_t nitems, mitkIpPicFile_t stream);
}

mitk::PicFileWriter::PicFileWriter()
{
  this->SetNumberOfRequiredInputs( 1 );
}

mitk::PicFileWriter::~PicFileWriter()
{
}

void mitk::PicFileWriter::GenerateData()
{
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  std::ofstream testfilehandle( m_FileName.c_str(), std::ios::out);
  if (!testfilehandle.good())
  {
    testfilehandle.close();
    itkExceptionMacro(<<"File location '" << m_FileName << "' not writeable");
  }
  else
  {
    testfilehandle.close();
  }

  Image::Pointer input = const_cast<Image*>(this->GetInput());

  if ( input.IsNull() )
  {
    itkExceptionMacro(<< "Nothing to write: Input is NULL." );
  }

  if( input->GetNumberOfChannels() > 1)
  {
    std::cout << "Multiple channel. Cannot write. Will throw...";
    itkExceptionMacro(<< "The PicFileWriter does not support multiple channel data. Nothing will be written." );
  }

  mitkIpPicDescriptor * picImage = mitkIpPicNew();
  mitk::ImageWriteAccessor imageAccess(input);
  picImage = CastToIpPicDescriptor(input, &imageAccess, picImage);

  SlicedGeometry3D* slicedGeometry = input->GetSlicedGeometry();
  if (slicedGeometry != NULL)
  {
    //set tag "REAL PIXEL SIZE"
    const Vector3D & spacing = slicedGeometry->GetSpacing();
    mitkIpPicTSV_t *pixelSizeTag;
    pixelSizeTag = mitkIpPicQueryTag( picImage, "REAL PIXEL SIZE" );
    if (!pixelSizeTag)
    {
      pixelSizeTag = (mitkIpPicTSV_t *) malloc( sizeof(mitkIpPicTSV_t) );
      pixelSizeTag->type = mitkIpPicFloat;
      pixelSizeTag->bpe = 32;
      strcpy(pixelSizeTag->tag, "REAL PIXEL SIZE");
      pixelSizeTag->dim = 1;
      pixelSizeTag->n[0] = 3;
      pixelSizeTag->value = malloc( sizeof(float) * 3 );
      mitkIpPicAddTag (picImage, pixelSizeTag);
    }
    ((float*)pixelSizeTag->value)[0] = spacing[0];
    ((float*)pixelSizeTag->value)[1] = spacing[1];
    ((float*)pixelSizeTag->value)[2] = spacing[2];
    //set tag "ISG"
    //ISG == offset/origin           transformation matrix(matrix)                                                              spancings
    //ISG == offset0 offset1 offset2 spalte0_0 spalte0_1 spalte0_2 spalte1_0 spalte1_1 spalte1_2 spalte2_0 spalte2_1 spalte2_2 spacing0 spacing1 spacing2
    mitkIpPicTSV_t *geometryTag;
    geometryTag = mitkIpPicQueryTag( picImage, "ISG" );
    if (!geometryTag)
    {
      geometryTag = (mitkIpPicTSV_t *) malloc( sizeof(mitkIpPicTSV_t) );
      geometryTag->type = mitkIpPicFloat;
      geometryTag->bpe = 32;
      strcpy(geometryTag->tag, "ISG");
      geometryTag->dim = 2;
      geometryTag->n[0] = 3;
      geometryTag->n[1] = 4;
      geometryTag->value = malloc( sizeof(float) * 3 * 4 );
      mitkIpPicAddTag (picImage, geometryTag);
    }
    const AffineTransform3D::OffsetType& offset = slicedGeometry->GetIndexToWorldTransform()->GetOffset();
    ((float*)geometryTag->value)[0] = offset[0];
    ((float*)geometryTag->value)[1] = offset[1];
    ((float*)geometryTag->value)[2] = offset[2];

    const AffineTransform3D::MatrixType& matrix = slicedGeometry->GetIndexToWorldTransform()->GetMatrix();
    const AffineTransform3D::MatrixType::ValueType* row0 = matrix[0];
    const AffineTransform3D::MatrixType::ValueType* row1 = matrix[1];
    const AffineTransform3D::MatrixType::ValueType* row2 = matrix[2];

    Vector3D v;

    FillVector3D(v, row0[0], row1[0], row2[0]);
    v.Normalize();
    ((float*)geometryTag->value)[3] = v[0];
    ((float*)geometryTag->value)[4] = v[1];
    ((float*)geometryTag->value)[5] = v[2];

    FillVector3D(v, row0[1], row1[1], row2[1]);
    v.Normalize();
    ((float*)geometryTag->value)[6] = v[0];
    ((float*)geometryTag->value)[7] = v[1];
    ((float*)geometryTag->value)[8] = v[2];

    ((float*)geometryTag->value)[9] = spacing[0];
    ((float*)geometryTag->value)[10] = spacing[1];
    ((float*)geometryTag->value)[11] = spacing[2];
  }
  PicFileReader::ConvertHandedness(picImage);  // flip upside-down in MITK coordinates

  // Following line added to detect write errors. If saving .pic files from the plugin is broken again,
  // please report a bug, don't just remove this line!
  int ret = MITKIpPicPut((char*)(m_FileName.c_str()), picImage);

  if (ret != 0)
  {
    PicFileReader::ConvertHandedness(picImage); // flip back from upside-down state
    throw std::ios_base::failure("Error during .pic file writing in "__FILE__);
  }

  PicFileReader::ConvertHandedness(picImage); // flip back from upside-down state
}

void mitk::PicFileWriter::SetInputImage( Image* image )
{
  this->ProcessObject::SetNthInput( 0, image );
}

const mitk::Image* mitk::PicFileWriter::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    MITK_ERROR << "No input image present.";
    return NULL;
  }
  else
  {
    return static_cast< const Image * >( this->ProcessObject::GetInput( 0 ) );
  }
}

int mitk::PicFileWriter::MITKIpPicPut( char *outfile_name, mitkIpPicDescriptor *pic )
{
  FILE* outfile;

  mitkIpUInt4_t len;
  mitkIpUInt4_t tags_len;

  if( pic->info->write_protect )
  {
    fprintf( stderr, "mitkIpPicPut: sorry, can't write (missing tags !!!)\n" );
    //return( -1 );
  }

  if( mitkIpPicEncryptionType(pic) != ' ' )
  {
    fprintf( stderr, "mitkIpPicPut: warning: was encrypted !!!\n" );
  }

  if( outfile_name == NULL )
    outfile = stdout;
  else if( strcmp(outfile_name, "stdout") == 0 )
    outfile = stdout;
  else
  {
    mitkIpPicRemoveFile( outfile_name );
    // Removed due to linker problems when compiling
    // an mitk chili plugin using msvc: there appear
    // unresolved external symbol errors to function
    // _ipPicGetWriteCompression()
    /*
    if( mitkIpPicGetWriteCompression() )
    {
      char buff[1024];

      sprintf( buff, "%s.gz", outfile_name );
      outfile = (FILE*) mitkIpPicFOpen( buff, "wb" ); // cast to prevent warning.
    }
    else
    */
    outfile = fopen( outfile_name, "wb" );
  }


  if( outfile == NULL )
  {
    fprintf( stderr, "mitkIpPicPut: sorry, error opening outfile\n" );
    return( -1 );
  }

  tags_len = _mitkIpPicTagsSize( pic->info->tags_head );

  len = tags_len +        3 * sizeof(mitkIpUInt4_t)
      + pic->dim * sizeof(mitkIpUInt4_t);

  /* write oufile */
  if( mitkIpPicEncryptionType(pic) == ' ' )
    mitkIpPicFWrite( mitkIpPicVERSION, 1, sizeof(mitkIpPicTag_t), outfile );
  else
    mitkIpPicFWrite( pic->info->version, 1, sizeof(mitkIpPicTag_t), outfile );

  mitkIpPicFWriteLE( &len, sizeof(mitkIpUInt4_t), 1, outfile );

  mitkIpPicFWriteLE( &(pic->type), sizeof(mitkIpUInt4_t), 1, outfile );
  mitkIpPicFWriteLE( &(pic->bpe), sizeof(mitkIpUInt4_t), 1, outfile );
  mitkIpPicFWriteLE( &(pic->dim), sizeof(mitkIpUInt4_t), 1, outfile );

  mitkIpPicFWriteLE( pic->n, sizeof(mitkIpUInt4_t), pic->dim, outfile );

  _mitkIpPicWriteTags( pic->info->tags_head, outfile, mitkIpPicEncryptionType(pic) );
   // Removed due to linker problems when compiling
   // an mitk chili plugin using msvc: there appear
   // unresolved external symbol errors to function
   // _ipPicGetWriteCompression()
  /*
  if( mitkIpPicGetWriteCompression() )
    pic->info->pixel_start_in_file = mitkIpPicFTell( outfile );
  else
  */
  pic->info->pixel_start_in_file = ftell( outfile );

  if( pic->data )
  {
    size_t number_of_elements = _mitkIpPicElements(pic);
    size_t bytes_per_element = pic->bpe / 8;
    size_t number_of_bytes = number_of_elements * bytes_per_element;
    size_t block_size = 1024*1024; /* Use 1 MB blocks. Make sure that block size is smaller than 2^31 */
    size_t number_of_blocks = number_of_bytes / block_size;
    size_t remaining_bytes = number_of_bytes % block_size;
    size_t bytes_written = 0;
    size_t block_nr = 0;
    mitkIpUInt1_t* data = (mitkIpUInt1_t*) pic->data;

    assert( data != NULL );

    if( pic->type == mitkIpPicNonUniform )
    {
      for ( block_nr = 0 ; block_nr < number_of_blocks ; ++block_nr )
        bytes_written += mitkIpPicFWrite( data + ( block_nr * block_size ), 1, block_size, outfile );
      bytes_written += mitkIpPicFWrite( data + ( number_of_blocks * block_size ), 1, remaining_bytes, outfile );
    }
    else
    {
      for ( block_nr = 0 ; block_nr < number_of_blocks ; ++block_nr )
        bytes_written += mitkIpPicFWriteLE( data + ( block_nr * block_size ), 1, block_size, outfile );
      bytes_written += mitkIpPicFWriteLE( data + ( number_of_blocks * block_size ), 1, remaining_bytes, outfile );
    }

    if ( bytes_written != number_of_bytes )
    {
      fprintf( stderr, "Error while writing (ferror indicates %u), only %u bytes were written! Eof indicator is %u.\n", ferror(outfile), ( (unsigned int) ( bytes_written ) ), feof(outfile) );
      fclose( outfile );
      return( -1 );
    }
  }

  if( outfile != stdout )
  {
    // Removed due to linker problems when compiling
    // an mitk chili plugin using msvc: there appear
    // unresolved external symbol errors to function
    // _ipPicGetWriteCompression()
    /*
    if( mitkIpPicGetWriteCompression() )
      mitkIpPicFClose( outfile );
    else
    */
    fclose( outfile );
  }

  return( 0 );
}

std::vector<std::string> mitk::PicFileWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".pic");
  return possibleFileExtensions;
}
