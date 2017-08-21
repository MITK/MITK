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
#include <mitkToFImageCsvWriter.h>

namespace mitk
{
  ToFImageCsvWriter::ToFImageCsvWriter(): ToFImageWriter(), m_DistanceOutfile(nullptr),
    m_AmplitudeOutfile(nullptr), m_IntensityOutfile(nullptr)
  {
    this->m_Extension = std::string(".csv");
  }

  ToFImageCsvWriter::~ToFImageCsvWriter()
  {
  }

  void ToFImageCsvWriter::Open()
  {
    this->CheckForFileExtension(this->m_DistanceImageFileName);
    this->CheckForFileExtension(this->m_AmplitudeImageFileName);
    this->CheckForFileExtension(this->m_IntensityImageFileName);

    this->m_ToFPixelNumber = this->m_ToFCaptureWidth * this->m_ToFCaptureHeight;
    this->m_ToFImageSizeInBytes = this->m_ToFPixelNumber * sizeof(float);

    if (this->m_DistanceImageSelected)
    {
      this->OpenCsvFile(&(this->m_DistanceOutfile), this->m_DistanceImageFileName);
    }
    if (this->m_AmplitudeImageSelected)
    {
      this->OpenCsvFile(&(this->m_AmplitudeOutfile), this->m_AmplitudeImageFileName);
    }
    if (this->m_IntensityImageSelected)
    {
      this->OpenCsvFile(&(this->m_IntensityOutfile), this->m_IntensityImageFileName);
    }
    this->m_NumOfFrames = 0;
  }

  void ToFImageCsvWriter::Close()
  {
    if (this->m_DistanceImageSelected)
    {
      this->CloseCsvFile(this->m_DistanceOutfile);
    }
    if (this->m_AmplitudeImageSelected)
    {
      this->CloseCsvFile(this->m_AmplitudeOutfile);
    }
    if (this->m_IntensityImageSelected)
    {
      this->CloseCsvFile(this->m_IntensityOutfile);
    }
  }

  void ToFImageCsvWriter::Add(float* distanceFloatData, float* amplitudeFloatData, float* intensityFloatData, unsigned char*)
  {
    if (this->m_DistanceImageSelected)
    {
      this->WriteCsvFile(this->m_DistanceOutfile, distanceFloatData);
    }
    if (this->m_AmplitudeImageSelected)
    {
      this->WriteCsvFile(this->m_AmplitudeOutfile, amplitudeFloatData);
    }
    if (this->m_IntensityImageSelected)
    {
      this->WriteCsvFile(this->m_IntensityOutfile, intensityFloatData);
    }
    this->m_NumOfFrames++;
  }

  void ToFImageCsvWriter::WriteCsvFile(FILE* outfile, float* floatData)
  {
    for(int i=0; i<this->m_ToFPixelNumber; i++)
    {
      if (this->m_NumOfFrames==0 && i==0)
      {
        fprintf(outfile, "%f", floatData[i]);
      }
      else
      {
        fprintf(outfile, ",%f", floatData[i]);
      }
    }
  }

  void ToFImageCsvWriter::OpenCsvFile(FILE** outfile, std::string outfileName)
  {
    (*outfile) = fopen( outfileName.c_str(), "w+" );
    if( !outfile )
    {
      MITK_ERROR << "Error opening outfile: " << outfileName;
      throw std::logic_error("Error opening outfile.");
      return;
    }
  }

  void ToFImageCsvWriter::CloseCsvFile(FILE* outfile)
  {
    if (this->m_NumOfFrames == 0)
    {
      fclose(outfile);
      throw std::logic_error("File is empty.");
      return;
    }
    fclose(outfile);
  }

}
