/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <mitkToFPicImageWriter.h>
#include <mitkPicFileWriter.h>
#include <mitkPicFileReader.h>

// itk includes
#include "itksys/SystemTools.hxx"

extern "C" 
{
size_t _mitkIpPicFWrite( const void *ptr, size_t size, size_t nitems, mitkIpPicFile_t stream);
}

namespace mitk
{
  ToFPicImageWriter::ToFPicImageWriter(): ToFImageWriter(), m_DistanceOutfile(NULL),
    m_AmplitudeOutfile(NULL),  m_IntensityOutfile(NULL)
  {
    m_Extension = std::string(".pic");
  }

  ToFPicImageWriter::~ToFPicImageWriter()
  {
  }

  void ToFPicImageWriter::Open()
  {
    this->CheckForFileExtension(this->m_DistanceImageFileName);
    this->CheckForFileExtension(this->m_AmplitudeImageFileName);
    this->CheckForFileExtension(this->m_IntensityImageFileName);

    this->m_PixelNumber = this->m_CaptureWidth * this->m_CaptureHeight;
    this->m_ImageSizeInBytes = this->m_PixelNumber * sizeof(float);
    float* floatData = new float[this->m_PixelNumber];
    for(int i=0; i<this->m_PixelNumber; i++)
    {
      floatData[i] = i + 0.0;
    }

    this->m_MitkImage = Image::New();
    unsigned int dimensions[4];
    dimensions[0] = this->m_CaptureWidth;
    dimensions[1] = this->m_CaptureHeight;
    if (this->m_ToFImageType == ToFImageWriter::ToFImageType2DPlusT)
    {
      dimensions[2] = 1;
      dimensions[3] = 2;
      this->m_MitkImage->Initialize( PixelType(typeid(float)), 4, dimensions, 1);
    }
    else
    {
      dimensions[2] = 2;
      dimensions[3] = 1;
      this->m_MitkImage->Initialize( PixelType(typeid(float)), 3, dimensions, 1);
    }
    this->m_MitkImage->SetSlice(floatData, 0, 0, 0);

    mitkIpPicDescriptor* pic = this->m_MitkImage->GetPic();

    if (this->m_DistanceImageSelected)
    {
      this->OpenPicFile(&(this->m_DistanceOutfile), this->m_DistanceImageFileName);
      this->WritePicFileHeader(this->m_DistanceOutfile, pic);
    }
    if (this->m_AmplitudeImageSelected)
    {
      this->OpenPicFile(&(this->m_AmplitudeOutfile), this->m_AmplitudeImageFileName);
      this->WritePicFileHeader(this->m_AmplitudeOutfile, pic);
    }
    if (this->m_IntensityImageSelected)
    {
      this->OpenPicFile(&(this->m_IntensityOutfile), this->m_IntensityImageFileName);
      this->WritePicFileHeader(this->m_IntensityOutfile, pic);
    }
    this->m_NumOfFrames = 0;
    delete[] floatData;
  }

  void ToFPicImageWriter::Close()
  {
    if (this->m_DistanceImageSelected)
    {
      this->ClosePicFile(this->m_DistanceOutfile);
    }
    if (this->m_AmplitudeImageSelected)
    {
      this->ClosePicFile(this->m_AmplitudeOutfile);
    }
    if (this->m_IntensityImageSelected)
    {
      this->ClosePicFile(this->m_IntensityOutfile);
    }
  }

  void ToFPicImageWriter::Add(float* distanceFloatData, float* amplitudeFloatData, float* intensityFloatData)
  {
    if (this->m_DistanceImageSelected)
    {
      fwrite( distanceFloatData, this->m_ImageSizeInBytes, 1, this->m_DistanceOutfile );
    }
    if (this->m_AmplitudeImageSelected)
    {
      fwrite( amplitudeFloatData, this->m_ImageSizeInBytes, 1, this->m_AmplitudeOutfile );
    }
    if (this->m_IntensityImageSelected)
    {
      fwrite( intensityFloatData, this->m_ImageSizeInBytes, 1, this->m_IntensityOutfile );
    }
    this->m_NumOfFrames++;
  }

  void ToFPicImageWriter::OpenPicFile(FILE** outfile,std::string outfileName)
  {
    (*outfile) = fopen( outfileName.c_str(), "w+b" );
    if( !outfile ) // if fopen_s was not successful!
    {
      MITK_ERROR << "Error opening outfile: " << outfileName;
      throw std::logic_error("Error opening outfile.");
      return;
    }
  }

  void ToFPicImageWriter::ClosePicFile(FILE* outfile)
  {
    if (this->m_NumOfFrames == 0)
    {
      fclose(outfile);
      throw std::logic_error("File is empty.");
      return;
    }
    this->ReplacePicFileHeader(outfile);
    fclose(outfile);
  }

  void ToFPicImageWriter::ReplacePicFileHeader(FILE* outfile)
  {
    mitkIpPicDescriptor* pic = this->m_MitkImage->GetPic();

    if (this->m_ToFImageType == ToFImageWriter::ToFImageType2DPlusT)
    {
      pic->dim = 4;
      pic->n[2] = 1;
      pic->n[3] = this->m_NumOfFrames;
    }
    else
    {
      pic->dim = 3;
      pic->n[2] = this->m_NumOfFrames;
      pic->n[3] = 1;
    }

    fseek ( outfile, 0, SEEK_SET );

    this->WritePicFileHeader( outfile, pic );
  }

  void ToFPicImageWriter::WritePicFileHeader(FILE* outfile, mitkIpPicDescriptor* pic)
  {
    mitkIpUInt4_t len;
    mitkIpUInt4_t tagsLen;

    tagsLen = _mitkIpPicTagsSize( pic->info->tags_head );
    len = tagsLen + 3 * sizeof(mitkIpUInt4_t) + pic->dim * sizeof(mitkIpUInt4_t);
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
    pic->info->pixel_start_in_file = ftell( outfile );
  }

} // end namespace mitk
