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
#include <mitkDiffusionPropertyHelper.h>
#include <mitkITKImageImport.h>

#include "mitkStringProperty.h"
#include <mitkImageCast.h>

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

}

mitk::DiffusionDICOMFileReader::~DiffusionDICOMFileReader()
{

}

bool mitk::DiffusionDICOMFileReader
::LoadImages()
{
  unsigned int numberOfOutputs = this->GetNumberOfOutputs();
  bool success = true;

  for(unsigned int o = 0; o < numberOfOutputs; ++o)
  {
    success &= this->LoadSingleOutputImage( this->m_OutputHeaderContainer.at(o),
                                            this->InternalGetOutput(o), this->m_IsMosaicData.at(o) );
  }

  return success;
}

bool mitk::DiffusionDICOMFileReader
::LoadSingleOutputImage( DiffusionHeaderDICOMFileReader::DICOMHeaderListType retrievedHeader,
                         DICOMImageBlockDescriptor& block, bool is_mosaic)
{
  // prepare data reading
  DiffusionDICOMFileReaderHelper helper;
  DiffusionDICOMFileReaderHelper::VolumeFileNamesContainer filenames;

  const DICOMImageFrameList& frames = block.GetImageFrameList();
  int numberOfDWImages = block.GetIntProperty("timesteps", 1);

  int numberOfFramesPerDWImage = frames.size() / numberOfDWImages;

  assert( int( double((double) frames.size() / (double) numberOfDWImages)) == numberOfFramesPerDWImage );
  for( int idx = 0; idx < numberOfDWImages; idx++ )
  {
    std::vector< std::string > FileNamesPerVolume;

    auto timeStepStart = frames.begin() + idx * numberOfFramesPerDWImage;
    auto timeStepEnd   = frames.begin() + (idx+1) * numberOfFramesPerDWImage;
    for (auto frameIter = timeStepStart;
         frameIter != timeStepEnd;
         ++frameIter)
    {
      FileNamesPerVolume.push_back( (*frameIter)->Filename );
    }

    filenames.push_back( FileNamesPerVolume );
  }

  // TODO : only prototyping to test loading of diffusion images
  // we need some solution for the different types
  mitk::Image::Pointer output_image = mitk::Image::New();

  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer directions =
      mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::New();


  double max_bvalue = 0;
  for( int idx = 0; idx < numberOfDWImages; idx++ )
  {
    DiffusionImageDICOMHeaderInformation header = retrievedHeader.at(idx);

    if( max_bvalue < header.b_value )
      max_bvalue = header.b_value;
  }

  // normalize the retrieved gradient directions according to the set b-value (maximal one)
  for( int idx = 0; idx < numberOfDWImages; idx++ )
  {
    DiffusionImageDICOMHeaderInformation header = retrievedHeader.at(idx);
    mitk::DiffusionPropertyHelper::GradientDirectionType grad = header.g_vector;

    grad.normalize();
    grad *= sqrt( header.b_value / max_bvalue );

    directions->push_back( grad );
  }

  // initialize the output image
  output_image->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( directions ) );
  output_image->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( max_bvalue ) );

  if( is_mosaic && this->m_ResolveMosaic )
  {
    mitk::DiffusionHeaderSiemensMosaicDICOMFileReader::Pointer mosaic_reader =
        mitk::DiffusionHeaderSiemensMosaicDICOMFileReader::New();

    // retrieve the remaining meta-information needed for mosaic reconstruction
    // it suffices to get it exemplatory from the first file in the file list
    mosaic_reader->RetrieveMosaicInformation( filenames.at(0).at(0) );

    mitk::MosaicDescriptor mdesc = mosaic_reader->GetMosaicDescriptor();
    mitk::CastToMitkImage( helper.LoadMosaicToVector<short, 3>( filenames, mdesc ), output_image );

  }
  else
  {
    mitk::CastToMitkImage( helper.LoadToVector<short, 3>( filenames ), output_image );
  }

  mitk::DiffusionPropertyHelper propertyHelper( output_image );
  propertyHelper.InitializeImage();

  output_image->SetProperty("diffusion.dicom.importname", mitk::StringProperty::New( helper.GetOutputName(filenames) ) );
  block.SetMitkImage( (mitk::Image::Pointer) output_image );

  return block.GetMitkImage().IsNotNull();

}

void mitk::DiffusionDICOMFileReader
::AnalyzeInputFiles()
{
  this->SetGroup3DandT(true);

  Superclass::AnalyzeInputFiles();

  // collect output from superclass
  size_t number_of_outputs = this->GetNumberOfOutputs();

  if(number_of_outputs == 0)
  {
    MITK_ERROR << "Failed to parse input, retrieved 0 outputs from SeriesGDCMReader ";
  }

  MITK_INFO("diffusion.dicomreader") << "Retrieved " << number_of_outputs << " outputs.";
  std::vector< bool > valid_input_list;

  for( unsigned int outputidx = 0; outputidx < this->GetNumberOfOutputs(); outputidx++ )
  {
    DICOMImageBlockDescriptor block_0 = this->GetOutput(outputidx);

    // collect vendor ID from the first output, first image
    StringList inputFilename;
    DICOMImageFrameInfo::Pointer frame_0 = block_0.GetImageFrameList().at(0);
    inputFilename.push_back( frame_0->Filename );


    mitk::DiffusionHeaderDICOMFileReader::Pointer headerReader;

    bool isMosaic = false;
    gdcm::Scanner gdcmScanner;

    gdcm::Tag t_vendor(0x008, 0x0070);
    gdcm::Tag t_imagetype(0x008, 0x008);

    // add DICOM Tag for vendor
    gdcmScanner.AddTag( t_vendor );
    // add DICOM Tag for image type
    gdcmScanner.AddTag( t_imagetype );
    if( gdcmScanner.Scan( inputFilename ) )
    {

      // retrieve both vendor and image type
      const char* ch_vendor = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_vendor );
      const char* ch_image_type = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_imagetype );

      if( ch_vendor == NULL || ch_image_type == NULL )
      {
        MITK_WARN << "Unable to retrieve vendor/image information from " << frame_0->Filename.c_str() << "\n" <<
                     "Output " << outputidx+1 << " is not valid, skipping analysis.";
        valid_input_list.push_back(false);
        continue;

      }

      std::string vendor = std::string( ch_vendor );
      std::string image_type = std::string( ch_image_type );
      MITK_INFO("diffusion.dicomreader") << "Output " << outputidx+1 << "  Got vendor: " << vendor << " image type " << image_type;

      // parse vendor tag
      if(    vendor.find("SIEMENS") != std::string::npos )
        //&& image_type.find("DIFFUSION") != std::string::npos )
      {
        if( image_type.find("MOSAIC") != std::string::npos )
        {
          headerReader = mitk::DiffusionHeaderSiemensMosaicDICOMFileReader::New();
          isMosaic = true;
        }
        else
        {
          headerReader = mitk::DiffusionHeaderSiemensDICOMFileReader::New();
        }

      }
      else if( vendor.find("GE") != std::string::npos )
      {
        headerReader = mitk::DiffusionHeaderGEDICOMFileReader::New();
      }
      else if( vendor.find("Philips") != std::string::npos )
      {
        headerReader = mitk::DiffusionHeaderPhilipsDICOMFileReader::New();
      }
      else
      {
        // unknown vendor
      }

      if( headerReader.IsNull() )
      {
        MITK_ERROR << "No header reader for given vendor. ";
        valid_input_list.push_back(false);
        continue;
      }

    }
    else
    {
      valid_input_list.push_back(false);
      continue;
    }


    bool canread = false;
    // iterate over the threeD+t block

    int numberOfTimesteps = block_0.GetIntProperty("timesteps", 1);
    int framesPerTimestep = block_0.GetImageFrameList().size() / numberOfTimesteps;

    for( int idx = 0; idx < numberOfTimesteps; idx++ )
    {
      int access_idx = idx * framesPerTimestep;
      DICOMImageFrameInfo::Pointer frame = this->GetOutput( outputidx ).GetImageFrameList().at( access_idx );
      canread = headerReader->ReadDiffusionHeader( frame->Filename );
    }

    if( canread )
    {
      // collect the information
      mitk::DiffusionHeaderDICOMFileReader::DICOMHeaderListType retrievedHeader = headerReader->GetHeaderInformation();

      m_IsMosaicData.push_back(isMosaic);
      m_OutputHeaderContainer.push_back( retrievedHeader );
      m_OutputReaderContainer.push_back( headerReader );
      valid_input_list.push_back(true);
    }
  }

  // check status of the outputs and remove the non-valid
  std::vector< DICOMImageBlockDescriptor > valid_outputs;
  for ( unsigned int outputidx = 0; outputidx < this->GetNumberOfOutputs(); ++outputidx )
  {
    if( valid_input_list.at(outputidx) )
    {
      valid_outputs.push_back( this->InternalGetOutput( outputidx ) );
    }
  }

  // clear complete list
  this->ClearOutputs();
  this->SetNumberOfOutputs( valid_outputs.size() );

  // insert only the valid ones
  for ( unsigned int outputidx = 0; valid_outputs.size(); ++outputidx )
  {
      this->SetOutput( outputidx, valid_outputs.at( outputidx ) );
  }



  for( unsigned int outputidx = 0; outputidx < this->GetNumberOfOutputs(); outputidx++ )
  {
    // TODO : Analyze outputs + header information, i.e. for the loading confidence
    MITK_INFO("diffusion.dicomreader") << "---- DICOM Analysis Report ----  :: Output " << outputidx+1 << " of " << this->GetNumberOfOutputs();

    try{
      PerformHeaderAnalysis( this->m_OutputHeaderContainer.at( outputidx) );
    }
    catch( const std::exception& se)
    {
      MITK_ERROR << "STD Exception " << se.what();
    }

    MITK_INFO("diffusion.dicomreader") << "===========================================";

  }



}


bool mitk::DiffusionDICOMFileReader
::CanHandleFile(const std::string & /* filename */)
{
  //FIXME :
  return true;
}

