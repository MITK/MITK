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

#include "mitkDiffusionDICOMFileReader.h"

#include "mitkDiffusionDICOMFileReaderHelper.h"

#include "mitkDiffusionHeaderSiemensDICOMFileReader.h"
#include "mitkDiffusionHeaderSiemensMosaicDICOMFileReader.h"
#include "mitkDiffusionHeaderGEDICOMFileReader.h"
#include "mitkDiffusionHeaderPhilipsDICOMFileReader.h"

static void PerformHeaderAnalysis( mitk::DiffusionHeaderDICOMFileReader::DICOMHeaderListType headers )
{
  unsigned int images = headers.size();

  unsigned int unweighted_images = 0;
  unsigned int weighted_images = 0;

  mitk::DiffusionHeaderDICOMFileReader::DICOMHeaderListType::const_iterator c_iter = headers.begin();
  while( c_iter != headers.end() )
  {
    const mitk::DiffusionImageDICOMHeaderInformation h = *c_iter;
    if( h.baseline ) unweighted_images++;
    if( h.b_value > 0 ) weighted_images++;

    ++c_iter;
  }

  MITK_INFO << "  :: Analyzed volumes " << images  << "\n"
            << "  :: \t"<< unweighted_images << " b = 0" << "\n"
            << "  :: \t"<< weighted_images << " b > 0";
}

mitk::DiffusionDICOMFileReader::DiffusionDICOMFileReader()
{
  m_IsMosaicData = false;
}

mitk::DiffusionDICOMFileReader::~DiffusionDICOMFileReader()
{

}

bool mitk::DiffusionDICOMFileReader
::LoadImages()
{
  // prepare data reading
  DiffusionDICOMFileReaderHelper helper;
  DiffusionDICOMFileReaderHelper::VolumeFileNamesContainer filenames;

  const size_t number_of_outputs = this->GetNumberOfOutputs();

  for( size_t idx = 0; idx < number_of_outputs; idx++ )
  {
    DICOMImageFrameList flist = this->GetOutput(idx).GetImageFrameList();

    std::vector< std::string > FileNamesPerVolume;

    DICOMImageFrameList::const_iterator cIt = flist.begin();
    while( cIt != flist.end() )
    {
      FileNamesPerVolume.push_back( (*cIt)->Filename );
      ++cIt;
    }

    filenames.push_back( FileNamesPerVolume );
  }

  // TODO : only prototyping to test loading of diffusion images
  // we need some solution for the different types
  typedef mitk::DiffusionImage<short> DiffusionImageType;
  DiffusionImageType::Pointer output_image = DiffusionImageType::New();

  DiffusionImageType::GradientDirectionContainerType::Pointer directions =
      DiffusionImageType::GradientDirectionContainerType::New();


  double max_bvalue = 0;
  for( size_t idx = 0; idx < number_of_outputs; idx++ )
  {
    DiffusionImageDICOMHeaderInformation header = this->m_RetrievedHeader.at(idx);

    if( max_bvalue < header.b_value )
      max_bvalue = header.b_value;
  }

  // normalize the retrieved gradient directions according to the set b-value (maximal one)
  for( size_t idx = 0; idx < number_of_outputs; idx++ )
  {
    DiffusionImageDICOMHeaderInformation header = this->m_RetrievedHeader.at(idx);
    DiffusionImageType::GradientDirectionType grad = header.g_vector;

    grad.normalize();
    grad *= sqrt( header.b_value / max_bvalue );

    directions->push_back( grad );
  }

  // initialize the output image
  output_image->SetDirections( directions );
  output_image->SetReferenceBValue( max_bvalue );
  if( this->m_IsMosaicData )
  {

    mitk::DiffusionHeaderSiemensMosaicDICOMFileReader::Pointer mosaic_reader =
        dynamic_cast< mitk::DiffusionHeaderSiemensMosaicDICOMFileReader* >( this->m_HeaderReader.GetPointer() );

    // retrieve the remaining meta-information needed for mosaic reconstruction
    // it suffices to get it exemplatory from the first file in the file list
    mosaic_reader->RetrieveMosaicInformation( filenames.at(0).at(0) );

    mitk::MosaicDescriptor mdesc = mosaic_reader->GetMosaicDescriptor();
    output_image->SetVectorImage( helper.LoadMosaicToVector<short, 3>( filenames, mdesc ) );

  }
  else
  {
    output_image->SetVectorImage( helper.LoadToVector<short, 3>( filenames ) );
  }
  output_image->InitializeFromVectorImage();
  //output_image->UpdateBValueMap();

  // reduce the number of outputs to 1 as we will produce a single image
  this->SetNumberOfOutputs(1);

  // set the image to output
  DICOMImageBlockDescriptor& block = this->InternalGetOutput(0);
  block.SetMitkImage( (mitk::Image::Pointer) output_image );

  return block.GetMitkImage().IsNotNull();

}

void mitk::DiffusionDICOMFileReader
::AnalyzeInputFiles()
{
  Superclass::AnalyzeInputFiles();

  // collect output from superclass
  size_t number_of_outputs = this->GetNumberOfOutputs();

  if(number_of_outputs == 0)
  {
    MITK_ERROR << "Failed to parse input, retrieved 0 outputs from SeriesGDCMReader ";
  }

  DICOMImageBlockDescriptor block_0 = this->GetOutput(0);

  MITK_INFO << "Retrieved " << number_of_outputs << "outputs.";

  // collect vendor ID from the first output, first image
  StringList inputFilename;
  DICOMImageFrameInfo::Pointer frame_0 = block_0.GetImageFrameList().at(0);
  inputFilename.push_back( frame_0->Filename );

  gdcm::Scanner gdcmScanner;

  gdcm::Tag t_vendor(0x008, 0x0070);
  gdcm::Tag t_imagetype(0x008, 0x008);

  // add DICOM Tag for vendor
  gdcmScanner.AddTag( t_vendor );
  // add DICOM Tag for image type
  gdcmScanner.AddTag( t_imagetype );
  gdcmScanner.Scan( inputFilename );

  // retrieve both vendor and image type
  std::string vendor = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_vendor );
  std::string image_type = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_imagetype );
  MITK_INFO << "Got vendor: " << vendor << " image type " << image_type;


  // parse vendor tag
  if( vendor.find("SIEMENS") != std::string::npos )
  {
    if( image_type.find("MOSAIC") != std::string::npos )
    {
      m_HeaderReader = mitk::DiffusionHeaderSiemensMosaicDICOMFileReader::New();
      this->m_IsMosaicData = true;
    }
    else
    {
      m_HeaderReader = mitk::DiffusionHeaderSiemensDICOMFileReader::New();
    }

  }
  else if( vendor.find("GE") != std::string::npos )
  {
    m_HeaderReader = mitk::DiffusionHeaderGEDICOMFileReader::New();
  }
  else if( vendor.find("Philips") != std::string::npos )
  {
    m_HeaderReader = mitk::DiffusionHeaderPhilipsDICOMFileReader::New();
  }
  else
  {
    // unknown vendor
  }

  if( m_HeaderReader.IsNull() )
  {
    MITK_ERROR << "No header reader for given vendor. ";
    return;
  }

  bool canread = false;
  for( size_t idx = 0; idx < number_of_outputs; idx++ )
  {
    DICOMImageFrameInfo::Pointer frame = this->GetOutput( idx ).GetImageFrameList().at(0);
    canread = m_HeaderReader->ReadDiffusionHeader(frame->Filename);
  }

  // collect the information
  m_RetrievedHeader = m_HeaderReader->GetHeaderInformation();

  // TODO : Analyze outputs + header information, i.e. for the loading confidence
  MITK_INFO << "----- Diffusion DICOM Analysis Report ---- ";

  PerformHeaderAnalysis( m_RetrievedHeader );

  MITK_INFO << "===========================================";

}


bool mitk::DiffusionDICOMFileReader
::CanHandleFile(const std::string &filename)
{
  //FIXME :
  return true;
}

