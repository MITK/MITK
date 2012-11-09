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
// mitk includes
#include <mitkToFNrrdImageWriter.h>

// itk includes
#include "itksys/SystemTools.hxx"
#include "itkNrrdImageIO.h"

namespace mitk
{
  ToFNrrdImageWriter::ToFNrrdImageWriter(): ToFImageWriter(),
    m_DistanceOutfile(), m_AmplitudeOutfile(), m_IntensityOutfile()
  {
    m_Extension = std::string(".nrrd");
  }

  ToFNrrdImageWriter::~ToFNrrdImageWriter()
  {
  }

  void ToFNrrdImageWriter::Open()
  {
    this->CheckForFileExtension(this->m_DistanceImageFileName);
    this->CheckForFileExtension(this->m_AmplitudeImageFileName);
    this->CheckForFileExtension(this->m_IntensityImageFileName);
    this->CheckForFileExtension(this->m_RGBImageFileName);

    this->m_ToFPixelNumber = this->m_ToFCaptureWidth * this->m_ToFCaptureHeight;
    this->m_ToFImageSizeInBytes = this->m_ToFPixelNumber * sizeof(float);

    this->m_RGBPixelNumber = this->m_RGBCaptureWidth * this->m_RGBCaptureHeight;
    this->m_RGBImageSizeInBytes = this->m_RGBPixelNumber * sizeof(unsigned char) * 3;

    if (this->m_DistanceImageSelected)
    {
      this->OpenStreamFile( this->m_DistanceOutfile, this->m_DistanceImageFileName);
    }
    if (this->m_AmplitudeImageSelected)
    {
      this->OpenStreamFile(this->m_AmplitudeOutfile, this->m_AmplitudeImageFileName);
    }
    if (this->m_IntensityImageSelected)
    {
      this->OpenStreamFile(this->m_IntensityOutfile, this->m_IntensityImageFileName);
    }
    if (this->m_RGBImageSelected)
    {
      this->OpenStreamFile(this->m_RGBOutfile, this->m_RGBImageFileName);
    }
    this->m_NumOfFrames = 0;
  }

  void ToFNrrdImageWriter::Close()
  {
    if (this->m_DistanceImageSelected)
    {
      this->CloseStreamFile(this->m_DistanceOutfile, this->m_DistanceImageFileName);
    }
    if (this->m_AmplitudeImageSelected)
    {
      this->CloseStreamFile(this->m_AmplitudeOutfile, this->m_AmplitudeImageFileName);
    }
    if (this->m_IntensityImageSelected)
    {
      this->CloseStreamFile(this->m_IntensityOutfile, this->m_IntensityImageFileName);
    }
    if (this->m_RGBImageSelected)
    {
      this->CloseStreamFile(this->m_RGBOutfile, this->m_RGBImageFileName);
    }
  }

  void ToFNrrdImageWriter::Add(float* distanceFloatData, float* amplitudeFloatData, float* intensityFloatData, unsigned char* rgbData)
  {
    if (this->m_DistanceImageSelected)
    {
      this->m_DistanceOutfile.write( (char*) distanceFloatData, this->m_ToFImageSizeInBytes);
    }
    if (this->m_AmplitudeImageSelected)
    {
      this->m_AmplitudeOutfile.write( (char*)amplitudeFloatData, this->m_ToFImageSizeInBytes);
    }
    if (this->m_IntensityImageSelected)
    {
      this->m_IntensityOutfile.write(( char* )intensityFloatData, this->m_ToFImageSizeInBytes);
    }
    if (this->m_RGBImageSelected)
    {
      this->m_RGBOutfile.write(( char* )rgbData, this->m_RGBImageSizeInBytes);
    }
    this->m_NumOfFrames++;
  }

  void ToFNrrdImageWriter::OpenStreamFile( std::ofstream &outfile, std::string outfileName )
  {
    outfile.open(outfileName.c_str(), std::ofstream::binary);
    if(!outfile.is_open())
    {
      MITK_ERROR << "Error opening outfile: " << outfileName;
      throw std::logic_error("Error opening outfile.");
      return;
    }
  }

  void ToFNrrdImageWriter::CloseStreamFile( std::ofstream &outfile, std::string fileName )
  {
    if (this->m_NumOfFrames == 0)
    {
      outfile.close();
      throw std::logic_error("File is empty.");
      return;
    }

    // flush the last data to the file and convert the stream data to nrrd file
    outfile.flush();
    this->ConvertStreamToNrrdFormat( fileName );
    outfile.close();
  }

  void ToFNrrdImageWriter::ConvertStreamToNrrdFormat( std::string fileName )
  {
    int CaptureWidth = 0;
    int CaptureHeight = 0;
    int PixelNumber = 0;
    int ImageSizeInBytes = 0;
    if (fileName==this->m_RGBImageFileName)
    {
        CaptureWidth = this->m_RGBCaptureWidth;
        CaptureHeight = this->m_RGBCaptureHeight;
        PixelNumber = this->m_RGBPixelNumber;
        ImageSizeInBytes = this->m_RGBImageSizeInBytes;
    } else
    {
        CaptureWidth = this->m_ToFCaptureWidth;
        CaptureHeight = this->m_ToFCaptureHeight;
        PixelNumber = this->m_ToFPixelNumber;
        ImageSizeInBytes = this->m_ToFImageSizeInBytes;
    }
    Image::Pointer imageTemplate = Image::New();
    int dimension ;
    unsigned int* dimensions;
    if(m_ToFImageType == ToFImageType2DPlusT)
    {
      dimension = 4;
      dimensions = new unsigned int[dimension];
      dimensions[0] = CaptureWidth;
      dimensions[1] = CaptureHeight;
      dimensions[2] = 1;
      dimensions[3] = this->m_NumOfFrames;
    }
    else if( m_ToFImageType == ToFImageType3D)
    {
      dimension = 3;
      dimensions = new unsigned int[dimension];
      dimensions[0] = CaptureWidth;
      dimensions[1] = CaptureHeight;
      dimensions[2] = this->m_NumOfFrames;
    }
    else
    {
      throw std::logic_error("No image type set, please choose between 2D+t and 3D!");
    }
    float* floatData;
    unsigned char* rgbData;
    if (fileName==this->m_RGBImageFileName)
    {
      rgbData = new unsigned char[PixelNumber*3];
      for(int i=0; i<PixelNumber*3; i++)
      {
        rgbData[i] = i + 0.0;
      }
      mitk::PixelType RGBType = MakePixelType<unsigned char, itk::RGBPixel<unsigned char>, 3>();
      imageTemplate->Initialize( RGBType,dimension, dimensions, 1);
      imageTemplate->SetSlice(rgbData, 0, 0, 0);
    }
    else
    {
      floatData = new float[PixelNumber];
      for(int i=0; i<PixelNumber; i++)
      {
        floatData[i] = i + 0.0;
      }
      mitk::PixelType FloatType = MakeScalarPixelType<float>();
      imageTemplate->Initialize( FloatType,dimension, dimensions, 1);
      imageTemplate->SetSlice(floatData, 0, 0, 0);
    }

    itk::NrrdImageIO::Pointer nrrdWriter = itk::NrrdImageIO::New();
    nrrdWriter->SetNumberOfDimensions(dimension);
    nrrdWriter->SetPixelTypeInfo(imageTemplate->GetPixelType().GetTypeId());
    if(imageTemplate->GetPixelType().GetNumberOfComponents() > 1)
    {
      nrrdWriter->SetNumberOfComponents(imageTemplate->GetPixelType().GetNumberOfComponents());
    }

    itk::ImageIORegion ioRegion( dimension );
    mitk::Vector3D spacing = imageTemplate->GetGeometry()->GetSpacing();
    mitk::Point3D origin = imageTemplate->GetGeometry()->GetOrigin();

    for(unsigned int i = 0; i < dimension; i++)
    {
      nrrdWriter->SetDimensions(i,dimensions[i]);
      nrrdWriter->SetSpacing(i,spacing[i]);
      nrrdWriter->SetOrigin(i,origin[i]);

      mitk::Vector3D direction;
      direction.Set_vnl_vector(imageTemplate->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(i));
      vnl_vector< double > axisDirection(dimension);

      for(unsigned int j = 0; j < dimension; j++)
      {
        axisDirection[j] = direction[j]/spacing[i];
      }
      nrrdWriter->SetDirection( i, axisDirection );

      ioRegion.SetSize(i, imageTemplate->GetLargestPossibleRegion().GetSize(i) );
      ioRegion.SetIndex(i, imageTemplate->GetLargestPossibleRegion().GetIndex(i) );
    }

    nrrdWriter->SetIORegion(ioRegion);
    nrrdWriter->SetFileName(fileName);
    nrrdWriter->SetUseStreamedWriting(true);

    std::ifstream stream(fileName.c_str(), std::ifstream::binary);
    if (fileName==m_RGBImageFileName)
    {
      unsigned int size = PixelNumber*3 * this->m_NumOfFrames;
      unsigned int sizeInBytes = size * sizeof(unsigned char);
      unsigned char* data = new unsigned char[size];
      stream.read((char*)data, sizeInBytes);
      nrrdWriter->Write(data);
      stream.close();
      delete[] data;
    }
    else
    {
      unsigned int size = PixelNumber * this->m_NumOfFrames;
      unsigned int sizeInBytes = size * sizeof(float);
      float* data = new float[size];
      stream.read((char*)data, sizeInBytes);
      nrrdWriter->Write(data);
      stream.close();
      delete[] data;
    }

    delete[] dimensions;
    if (fileName==m_RGBImageFileName)
    {
      delete[] rgbData;
    }
    else
    {
      delete[] floatData;
    }
  }

} // end namespace mitk
