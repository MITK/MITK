/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkDicomDiffusionImageHeaderReader.h"

#include "mitkGEDicomDiffusionImageHeaderReader.h"
#include "mitkPhilipsDicomDiffusionImageHeaderReader.h"
#include "mitkSiemensDicomDiffusionImageHeaderReader.h"
#include "mitkSiemensMosaicDicomDiffusionImageHeaderReader.h"

void InsertUnique( std::vector<float> & vec, float value )
{
  int n = vec.size();
  if (n == 0)
  {
    vec.push_back( value );
    return;
  }

  for (int k = 0; k < n ; k++)
  {
    if (vec[k] == value)
    {
      return;
    }
  }

  // if we get here, it means value is not in vec.
  vec.push_back( value );
  return;

}

mitk::DicomDiffusionImageHeaderReader::DicomDiffusionImageHeaderReader()
{
  m_SliceOrderIS = true;
  m_SingleSeries = true;
}

mitk::DicomDiffusionImageHeaderReader::~DicomDiffusionImageHeaderReader()
{
}

mitk::DicomDiffusionImageHeaderReader::SupportedVendors
mitk::DicomDiffusionImageHeaderReader::GetVendorID()
{
  // adapted from namic-sandbox
  // DicomToNrrdConverter.cxx

  m_GdcmIO = ImageIOType::New();
  m_GdcmIO->LoadPrivateTagsOn();
  m_GdcmIO->SetMaxSizeLoadEntry( 65536 );

  m_VolumeReader = VolumeReaderType::New();
  m_VolumeReader->SetImageIO( m_GdcmIO );
  m_VolumeReader->SetFileNames( m_DicomFilenames );

  try
  {
    m_VolumeReader->Update();
  }
  catch (itk::ExceptionObject &excp)
  {
    std::cerr << "Exception thrown while reading slice" << std::endl;
    std::cerr << excp << std::endl;
    return SV_UNKNOWN_VENDOR;
  }

  VolumeReaderType::DictionaryArrayRawPointer inputDict
    = m_VolumeReader->GetMetaDataDictionaryArray();

  std::string vendor;
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0008|0070", vendor );
  // std::cout << vendor << std::endl;

  std::string ImageType;
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0008|0008", ImageType );
  //std::cout << ImageType << std::endl;

  if ( vendor.find("GE") != std::string::npos )
  {
    // for GE data
    return SV_GE;
  }
  else if( vendor.find("SIEMENS") != std::string::npos )
  {
    if ( ImageType.find("MOSAIC") != std::string::npos )
    {
      // for Siemens MOSAIC
      return SV_SIEMENS_MOSAIC;
    }
    else
    {
      // for Siemens SPLIT
      return SV_SIEMENS;
    }
  }
  else if( vendor.find("PHILIPS") != std::string::npos )
  {
    // for philips data
    return SV_PHILIPS;
  }
  else
  {
    // for unrecognized vendors
    return SV_UNKNOWN_VENDOR;
  }
}

// do the work
void mitk::DicomDiffusionImageHeaderReader::Update()
{

  // check if there are filenames
  if(m_DicomFilenames.size())
  {

    m_Output = mitk::DiffusionImageHeaderInformation::New();
    m_Output->m_DicomFilenames = m_DicomFilenames;

    // create correct reader
    switch(GetVendorID())
    {
    case(SV_GE):
      {
        GEDicomDiffusionImageHeaderReader::Pointer reader
          = GEDicomDiffusionImageHeaderReader::New();
        reader->SetSeriesDicomFilenames(this->m_DicomFilenames);
        reader->SetGdcmIO(this->m_GdcmIO);
        reader->SetVolumeReader(this->m_VolumeReader);
        reader->SetOutputPointer(this->m_Output);
        reader->Update();
        this->m_Output = reader->GetOutput();
        break;
      }
    case(SV_SIEMENS):
      {
        SiemensDicomDiffusionImageHeaderReader::Pointer reader
          = SiemensDicomDiffusionImageHeaderReader::New();
        reader->SetSeriesDicomFilenames(this->m_DicomFilenames);
        reader->SetGdcmIO(this->m_GdcmIO);
        reader->SetVolumeReader(this->m_VolumeReader);
        reader->SetOutputPointer(this->m_Output);
        reader->Update();
        this->m_Output = reader->GetOutput();
        break;
      }
    case(SV_SIEMENS_MOSAIC):
      {

        SiemensMosaicDicomDiffusionImageHeaderReader::Pointer reader
          = SiemensMosaicDicomDiffusionImageHeaderReader::New();
        reader->SetSeriesDicomFilenames(this->m_DicomFilenames);
        reader->SetGdcmIO(this->m_GdcmIO);
        reader->SetVolumeReader(this->m_VolumeReader);
        reader->SetOutputPointer(this->m_Output);
        reader->Update();
        this->m_Output = reader->GetOutput();
        break;
      }
    case(SV_PHILIPS):
      {

        PhilipsDicomDiffusionImageHeaderReader::Pointer reader
          = PhilipsDicomDiffusionImageHeaderReader::New();
        reader->SetSeriesDicomFilenames(this->m_DicomFilenames);
        reader->SetGdcmIO(this->m_GdcmIO);
        reader->SetVolumeReader(this->m_VolumeReader);
        reader->SetOutputPointer(this->m_Output);
        reader->Update();
        this->m_Output = reader->GetOutput();
        break;
      }
    case(SV_UNKNOWN_VENDOR):
      {
        std::cerr << "diffusion header reader: unknown vendor" << std::endl;
        break;
      }
    }
  }
}

// return output
mitk::DiffusionImageHeaderInformation::Pointer
mitk::DicomDiffusionImageHeaderReader::GetOutput()
{
  return m_Output;
}

void mitk::DicomDiffusionImageHeaderReader::ReadPublicTags()
{
  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, NULL );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not set locale " << locale;
    }
  }

  VolumeReaderType::DictionaryArrayRawPointer inputDict
    = m_VolumeReader->GetMetaDataDictionaryArray();

  // load in all public tags
  m_nSlice = inputDict->size();
  std::string tag;

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0008|103e", tag );
  this->m_Output->seriesDescription = tag.c_str();

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0011", tag );
  this->m_Output->seriesNumber = atoi(tag.c_str());

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0010|0010", tag );
  this->m_Output->patientName = tag.c_str();

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0028|0010", tag );
  this->m_Output->nRows = atoi( tag.c_str() );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0028|0011", tag );
  this->m_Output->nCols = atoi( tag.c_str() );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0028|0030", tag );
  sscanf( tag.c_str(), "%f\\%f", &this->m_Output->xRes, &this->m_Output->yRes );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0032", tag );
  sscanf( tag.c_str(), "%f\\%f\\%f", &this->m_Output->xOrigin, &this->m_Output->yOrigin, &this->m_Output->zOrigin );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0018|0050", tag );
  this->m_Output->sliceThickness = atof( tag.c_str() );

  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0018|0088", tag );
  this->m_Output->sliceSpacing = atof( tag.c_str() );

  // figure out how many slices are there in a volume, each unique
  // SliceLocation represent one slice
  for (int k = 0; k < m_nSlice; k++)
  {
    tag.clear();
    itk::ExposeMetaData<std::string> ( *(*inputDict)[k], "0020|1041",  tag);
    float sliceLocation = atof( tag.c_str() );
    InsertUnique( m_sliceLocations, sliceLocation );
  }

  // check ImageOrientationPatient and figure out slice direction in
  // L-P-I (right-handed) system.
  // In Dicom, the coordinate frame is L-P by default. Look at
  // http://medical.nema.org/dicom/2007/07_03pu.pdf ,  page 301
  tag.clear();
  itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0037", tag );
  float xRow, yRow, zRow, xCol, yCol, zCol, xSlice, ySlice, zSlice /*, orthoSliceSpacing*/;
  sscanf( tag.c_str(), "%f\\%f\\%f\\%f\\%f\\%f", &xRow, &yRow, &zRow, &xCol, &yCol, &zCol );

  // In Dicom, the measurement frame is L-P by default. Look at
  // http://medical.nema.org/dicom/2007/07_03pu.pdf ,  page 301, in
  // order to make this compatible with Slicer's RAS frame, we
  // multiply the direction cosines by the negatives of the resolution
  // (resolution is required by nrrd format). Direction cosine is not
  // affacted since the resulting frame is still a right-handed frame.
  xRow = -xRow;
  yRow = -yRow;

  xCol = -xCol;
  yCol = -yCol;

  // Cross product, this gives I-axis direction
  xSlice = (yRow*zCol-zRow*yCol)*this->m_Output->sliceSpacing;
  ySlice = (zRow*xCol-xRow*zCol)*this->m_Output->sliceSpacing;
  zSlice = (xRow*yCol-yRow*xCol)*this->m_Output->sliceSpacing;

  xRow *= this->m_Output->xRes;
  yRow *= this->m_Output->xRes;
  zRow *= this->m_Output->xRes;

  xCol *= this->m_Output->yRes;
  yCol *= this->m_Output->yRes;
  zCol *= this->m_Output->yRes;

  this->m_Output->xRow = xRow;
  this->m_Output->yRow = yRow;
  this->m_Output->zRow = zRow;
  this->m_Output->xCol = xCol;
  this->m_Output->yCol = yCol;
  this->m_Output->zCol = zCol;
  this->m_Output->xSlice = xSlice;
  this->m_Output->ySlice = ySlice;
  this->m_Output->zSlice = zSlice;

  try
  {
    setlocale(LC_ALL, currLocale.c_str());
  }
  catch(...)
  {
    MITK_INFO << "Could not reset locale " << currLocale;
  }
}


void mitk::DicomDiffusionImageHeaderReader::ReadPublicTags2()
{

  if (!m_SliceOrderIS)
  {
    this->m_Output->xSlice = -this->m_Output->xSlice;
    this->m_Output->ySlice = -this->m_Output->ySlice;
    this->m_Output->zSlice = -this->m_Output->zSlice;
  }

}

void mitk::DicomDiffusionImageHeaderReader::TransformGradients()
{
  // transform gradient directions into RAS frame
    if ( !m_SliceOrderIS )
    {
      this->m_Output->DiffusionVector[2] = -this->m_Output->DiffusionVector[2];  // I -> S
    }
}

