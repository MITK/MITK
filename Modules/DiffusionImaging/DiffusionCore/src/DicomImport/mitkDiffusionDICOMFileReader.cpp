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
  m_ApplyRotationToGradients = true;
  m_ResolveMosaic = true;
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
  mitk::DiffusionPropertyHelper::SetOriginalGradientContainer(output_image, directions);
  mitk::DiffusionPropertyHelper::SetReferenceBValue(output_image, max_bvalue);

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

  mitk::DiffusionPropertyHelper::SetApplyMatrixToGradients(output_image, m_ApplyRotationToGradients);
  mitk::DiffusionPropertyHelper::InitializeImage(output_image);
  output_image->SetProperty("diffusion.dicom.importname", mitk::StringProperty::New( helper.GetOutputName(filenames) ) );

  block.SetMitkImage( (mitk::Image::Pointer) output_image );

  return block.GetMitkImage().IsNotNull();

}

std::vector<std::string> mitk::DiffusionDICOMFileReader::patient_ids() const
{
  return m_patient_ids;
}

std::vector<std::string> mitk::DiffusionDICOMFileReader::patient_names() const
{
  return m_patient_names;
}

std::vector<std::string> mitk::DiffusionDICOMFileReader::study_instance_uids() const
{
  return m_study_instance_uids;
}

std::vector<std::string> mitk::DiffusionDICOMFileReader::series_instance_uids() const
{
  return m_series_instance_uids;
}

std::vector<std::string> mitk::DiffusionDICOMFileReader::frame_of_reference_uids() const
{
  return m_frame_of_reference_uids;
}

std::vector<std::string> mitk::DiffusionDICOMFileReader::sop_instance_uids() const
{
  return m_sop_instance_uids;
}

void mitk::DiffusionDICOMFileReader
::AnalyzeInputFiles()
{
  m_Study_names.clear();
  m_Series_names.clear();
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

    gdcm::Tag t_sop_instance_uid(0x0008, 0x0018);
    gdcm::Tag t_frame_of_reference_uid(0x0020, 0x0052);
    gdcm::Tag t_series_instance_uid(0x0020, 0x000E);
    gdcm::Tag t_study_instance_uid(0x0020, 0x000D);
    gdcm::Tag t_patient_name(0x0010, 0x0010);
    gdcm::Tag t_patient_id(0x0010, 0x0020);

    gdcm::Tag t_vendor(0x008, 0x0070);
    gdcm::Tag t_imagetype(0x0008, 0x0008);
    gdcm::Tag t_StudyDescription(0x0008, 0x1030);
    gdcm::Tag t_SeriesDescription(0x0008, 0x103E);

    // add DICOM Tag for vendor
    gdcmScanner.AddTag( t_vendor );
    // add DICOM Tag for image type
    gdcmScanner.AddTag( t_imagetype );
    gdcmScanner.AddTag( t_StudyDescription );
    gdcmScanner.AddTag( t_SeriesDescription );

    gdcmScanner.AddTag( t_sop_instance_uid );
    gdcmScanner.AddTag( t_frame_of_reference_uid );
    gdcmScanner.AddTag( t_series_instance_uid );
    gdcmScanner.AddTag( t_study_instance_uid );
    gdcmScanner.AddTag( t_patient_name );
    gdcmScanner.AddTag( t_patient_id );

    if( gdcmScanner.Scan( inputFilename ) )
    {

      // retrieve both vendor and image type
      const char* ch_vendor = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_vendor );
      const char* ch_image_type = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_imagetype );

      const char* temp = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_sop_instance_uid );
      if (temp!=nullptr)
        m_sop_instance_uids.push_back(std::string(temp));
      else
        m_sop_instance_uids.push_back("-");

      temp = nullptr; temp = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_frame_of_reference_uid );
      if (temp!=nullptr)
        m_frame_of_reference_uids.push_back(std::string(temp));
      else
        m_frame_of_reference_uids.push_back("-");

      temp = nullptr; temp = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_series_instance_uid );
      if (temp!=nullptr)
        m_series_instance_uids.push_back(std::string(temp));
      else
        m_series_instance_uids.push_back("-");

      temp = nullptr; temp = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_study_instance_uid );
      if (temp!=nullptr)
        m_study_instance_uids.push_back(std::string(temp));
      else
        m_study_instance_uids.push_back("-");

      temp = nullptr; temp = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_patient_name );
      if (temp!=nullptr)
        m_patient_names.push_back(std::string(temp));
      else
        m_patient_names.push_back("-");

      temp = nullptr; temp = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_patient_id );
      if (temp!=nullptr)
        m_patient_ids.push_back(std::string(temp));
      else
        m_patient_ids.push_back("-");

      if( ch_vendor == nullptr || ch_image_type == nullptr )
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
      if(    vendor.find("SIEMENS") != std::string::npos || vendor.find("Siemens HealthCare GmbH") != std::string::npos )
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

      const char* ch_StudyDescription = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_StudyDescription );
      const char* ch_SeriesDescription = gdcmScanner.GetValue( frame_0->Filename.c_str(), t_SeriesDescription );
      if( ch_StudyDescription != nullptr )
        m_Study_names.push_back(ch_StudyDescription);
      else
        m_Study_names.push_back("-");

      if( ch_SeriesDescription != nullptr )
        m_Series_names.push_back(ch_SeriesDescription);
      else
        m_Series_names.push_back("-");
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
    this->SetOutput( outputidx, valid_outputs.at( outputidx ) );

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

