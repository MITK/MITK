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

#include "mitkDICOMITKSeriesGDCMReader.h"
#include "mitkITKDICOMSeriesReaderHelper.h"
#include "mitkGantryTiltInformation.h"

#include <itkTimeProbesCollectorBase.h>

#include <gdcmScanner.h>

mitk::DICOMITKSeriesGDCMReader
::DICOMITKSeriesGDCMReader()
:DICOMFileReader()
,m_FixTiltByShearing(true)
,m_Group3DplusT(false)
{
}

mitk::DICOMITKSeriesGDCMReader
::DICOMITKSeriesGDCMReader(const DICOMITKSeriesGDCMReader& other )
:DICOMFileReader(other)
,m_FixTiltByShearing(false)
,m_Group3DplusT(false)
{
}

mitk::DICOMITKSeriesGDCMReader
::~DICOMITKSeriesGDCMReader()
{
}

mitk::DICOMITKSeriesGDCMReader&
mitk::DICOMITKSeriesGDCMReader
::operator=(const DICOMITKSeriesGDCMReader& other)
{
  if (this != &other)
  {
    DICOMFileReader::operator=(other);
  }
  return *this;
}

void
mitk::DICOMITKSeriesGDCMReader
::SetFixTiltByShearing(bool on)
{
  m_FixTiltByShearing = on;
}

mitk::DICOMGDCMImageFrameList
mitk::DICOMITKSeriesGDCMReader
::FromDICOMDatasetList(DICOMDatasetList& input)
{
  DICOMGDCMImageFrameList output;
  output.reserve(input.size());

  for(DICOMDatasetList::iterator inputIter = input.begin();
      inputIter != input.end();
      ++inputIter)
  {
    DICOMGDCMImageFrameInfo* gfi = dynamic_cast<DICOMGDCMImageFrameInfo*>(*inputIter);
    assert(gfi);
    output.push_back(gfi);
  }

  return output;
}

mitk::DICOMDatasetList
mitk::DICOMITKSeriesGDCMReader
::ToDICOMDatasetList(DICOMGDCMImageFrameList& input)
{
  DICOMDatasetList output;
  output.reserve(input.size());

  for(DICOMGDCMImageFrameList::iterator inputIter = input.begin();
      inputIter != input.end();
      ++inputIter)
  {
    DICOMDatasetAccess* da = inputIter->GetPointer();
    assert(da);
    output.push_back(da);
  }

  return output;
}

mitk::DICOMImageFrameList
mitk::DICOMITKSeriesGDCMReader
::ToDICOMImageFrameList(DICOMGDCMImageFrameList& input)
{
  DICOMImageFrameList output;
  output.reserve(input.size());

  for(DICOMGDCMImageFrameList::iterator inputIter = input.begin();
      inputIter != input.end();
      ++inputIter)
  {
    DICOMImageFrameInfo::Pointer fi = (*inputIter)->GetFrameInfo();
    assert(fi.IsNotNull());
    output.push_back(fi);
  }

  return output;
}

void
mitk::DICOMITKSeriesGDCMReader
::AnalyzeInputFiles()
{
  // TODO at this point, make sure we have a sorting element at the end that splits geometrically separate blocks
  this->EnsureMandatorySortersArePresent();

  itk::TimeProbesCollectorBase timer;

  timer.Start("Reset");
  this->ClearOutputs();
  timer.Stop("Reset");

  // prepare initial sorting (== list of input files)
  StringList inputFilenames = this->GetInputFiles();

  // scan files for sorting-relevant tags

  // TODO provide tagToValueMappings to items in initialFramelist  / m_SortingResultInProgress
  timer.Start("Setup scanning");
  gdcm::Scanner gdcmScanner;
  for(SorterList::iterator sorterIter = m_Sorter.begin();
      sorterIter != m_Sorter.end();
      ++sorterIter)
  {
    assert(sorterIter->IsNotNull());

    DICOMTagList tags = (*sorterIter)->GetTagsOfInterest();
    for(DICOMTagList::const_iterator tagIter = tags.begin();
        tagIter != tags.end();
        ++tagIter)
    {
      MITK_DEBUG << "Sorting uses tag " << tagIter->GetGroup() << "," << tagIter->GetElement();
      gdcmScanner.AddTag( gdcm::Tag(tagIter->GetGroup(), tagIter->GetElement()) );
    }
  }

  // Add some of our own interest
  gdcmScanner.AddTag( gdcm::Tag(0x0018,0x1164) );
  gdcmScanner.AddTag( gdcm::Tag(0x0028,0x0030) );

  timer.Stop("Setup scanning");

  timer.Start("Tag scanning");
  gdcmScanner.Scan( inputFilenames );
  timer.Stop("Tag scanning");

  timer.Start("Setup sorting");
  DICOMGDCMImageFrameList initialFramelist;
  for (StringList::const_iterator inputIter = inputFilenames.begin();
       inputIter != inputFilenames.end();
       ++inputIter)
  {
    // TODO check DICOMness and non-multi-framedness
    initialFramelist.push_back( DICOMGDCMImageFrameInfo::New( DICOMImageFrameInfo::New(*inputIter, 0), gdcmScanner.GetMapping(inputIter->c_str()) ) );
  }
  m_SortingResultInProgress.clear();
  m_SortingResultInProgress.push_back( initialFramelist );
  timer.Stop("Setup sorting");

  // sort and split blocks as configured

  timer.Start("Sorting frames");
  SortingBlockList nextStepSorting; // we should not modify our input list while processing it
  unsigned int sorterIndex = 0;
  for(SorterList::iterator sorterIter = m_Sorter.begin();
      sorterIter != m_Sorter.end();
      ++sorterIndex, ++sorterIter)
  {
    std::stringstream ss; ss << "Sorting step " << sorterIndex;
    timer.Start( ss.str().c_str() );
    nextStepSorting.clear();
    DICOMDatasetSorter::Pointer& sorter = *sorterIter;

    MITK_DEBUG << "================================================================================";
    MITK_DEBUG << "DICOMIKTSeriesGDCMReader: " << ss.str() << ": " << m_SortingResultInProgress.size() << " groups input";
    unsigned int groupIndex = 0;

    for(SortingBlockList::iterator blockIter = m_SortingResultInProgress.begin();
        blockIter != m_SortingResultInProgress.end();
        ++groupIndex, ++blockIter)
    {
      DICOMGDCMImageFrameList& gdcmInfoFrameList = *blockIter;
      DICOMDatasetList datasetList = ToDICOMDatasetList( gdcmInfoFrameList );

      MITK_DEBUG << "--------------------------------------------------------------------------------";
      MITK_DEBUG << "DICOMIKTSeriesGDCMReader: " << ss.str() << ", dataset group " << groupIndex << " (" << datasetList.size() << " datasets): ";
      for (DICOMDatasetList::iterator oi = datasetList.begin();
           oi != datasetList.end();
           ++oi)
      {
        MITK_DEBUG << "  INPUT     : " << (*oi)->GetFilenameIfAvailable();
      }

      sorter->SetInput(datasetList);
      sorter->Sort();
      unsigned int numberOfResultingBlocks = sorter->GetNumberOfOutputs();

      for (unsigned int b = 0; b < numberOfResultingBlocks; ++b)
      {
        DICOMDatasetList blockResult = sorter->GetOutput(b);

        for (DICOMDatasetList::iterator oi = blockResult.begin();
             oi != blockResult.end();
             ++oi)
        {
          MITK_DEBUG << "  OUTPUT(" << b << ") :" << (*oi)->GetFilenameIfAvailable();
        }

        DICOMGDCMImageFrameList sortedGdcmInfoFrameList = FromDICOMDatasetList(blockResult);
        nextStepSorting.push_back( sortedGdcmInfoFrameList );
      }
    }

    m_SortingResultInProgress = nextStepSorting;
    timer.Stop( ss.str().c_str() );
  }
  timer.Stop("Sorting frames");

  // provide final result as output

  timer.Start("Output");
  this->SetNumberOfOutputs( m_SortingResultInProgress.size() );
  unsigned int o = 0;
  for (SortingBlockList::iterator blockIter = m_SortingResultInProgress.begin();
       blockIter != m_SortingResultInProgress.end();
       ++o, ++blockIter)
  {
    DICOMGDCMImageFrameList& gdcmFrameInfoList = *blockIter;
    DICOMImageFrameList frameList = ToDICOMImageFrameList( gdcmFrameInfoList );
    assert(!gdcmFrameInfoList.empty());
    assert(!frameList.empty());

    DICOMImageBlockDescriptor block;
    block.SetImageFrameList( frameList );

    bool hasTilt = false;
    const GantryTiltInformation& tiltInfo = m_EquiDistantBlocksSorter->GetTiltInformation( (gdcmFrameInfoList.front())->GetFilenameIfAvailable(), hasTilt );
    block.SetHasGantryTilt( hasTilt );
    block.SetTiltInformation( tiltInfo );

    // assume
    static const DICOMTag tagPixelSpacing(0x0028,0x0030);
    static const DICOMTag tagImagerPixelSpacing(0x0018,0x1164);
    std::string pixelSpacingString = (gdcmFrameInfoList.front())->GetTagValueAsString( tagPixelSpacing );
    std::string imagerPixelSpacingString = gdcmFrameInfoList.front()->GetTagValueAsString( tagImagerPixelSpacing );
    block.SetPixelSpacingInformation(pixelSpacingString, imagerPixelSpacingString);

    this->SetOutput( o, block );
  }
  timer.Stop("Output");

  std::cout << "---------------------------------------------------------------" << std::endl;
  timer.Report( std::cout );
  std::cout << "---------------------------------------------------------------" << std::endl;
}

// void AllocateOutputImages();

bool
mitk::DICOMITKSeriesGDCMReader
::LoadImages()
{
  mitk::ITKDICOMSeriesReaderHelper helper;

  unsigned int numberOfOutputs = this->GetNumberOfOutputs();
  for (unsigned int o = 0; o < numberOfOutputs; ++o)
  {
    DICOMImageBlockDescriptor& block = this->InternalGetOutput(o);
    const DICOMImageFrameList& frames = block.GetImageFrameList();
    const GantryTiltInformation tiltInfo = block.GetTiltInformation();
    bool hasTilt = block.HasGantryTilt();
    if (hasTilt)
    {
      MITK_DEBUG << "When loading image " << o << ": got tilt info:";
      tiltInfo.Print(std::cout);
    }
    else
    {
      MITK_DEBUG << "When loading image " << o << ": has NO info.";
    }

    ITKDICOMSeriesReaderHelper::StringContainer filenames;
    for (DICOMImageFrameList::const_iterator frameIter = frames.begin();
        frameIter != frames.end();
        ++frameIter)
    {
      filenames.push_back( (*frameIter)->Filename );
    }

    mitk::Image::Pointer mitkImage = helper.Load( filenames, m_FixTiltByShearing && hasTilt, tiltInfo ); // TODO preloaded images, caching..?

    Vector3D imageSpacing = mitkImage->GetGeometry()->GetSpacing();

    ScalarType desiredSpacingX = imageSpacing[0];
    ScalarType desiredSpacingY = imageSpacing[1];
    block.GetDesiredMITKImagePixelSpacing( desiredSpacingX, desiredSpacingY ); // prefer pixel spacing over imager pixel spacing

    MITK_DEBUG << "Loaded image with spacing " << imageSpacing[0] << ", " << imageSpacing[1];
    MITK_DEBUG << "Found correct spacing info " << desiredSpacingX << ", " << desiredSpacingY;

    imageSpacing[0] = desiredSpacingX;
    imageSpacing[1] = desiredSpacingY;
    mitkImage->GetGeometry()->SetSpacing( imageSpacing );

    block.SetMitkImage( mitkImage );
  }

  return true;
}


bool
mitk::DICOMITKSeriesGDCMReader
::CanHandleFile(const std::string& itkNotUsed(filename))
{
  return true; // can handle all
}

void
mitk::DICOMITKSeriesGDCMReader
::AddSortingElement(DICOMDatasetSorter* sorter)
{
  assert(sorter);
  m_Sorter.push_back( sorter );
}

void
mitk::DICOMITKSeriesGDCMReader
::EnsureMandatorySortersArePresent()
{
  // TODO: cols, rows, etc. are also not optional
  if (m_EquiDistantBlocksSorter.IsNull())
  {
    m_EquiDistantBlocksSorter = mitk::EquiDistantBlocksSorter::New();
  }
  m_EquiDistantBlocksSorter->SetAcceptTilt( m_FixTiltByShearing );

  // TODO CHECK
  this->AddSortingElement( m_EquiDistantBlocksSorter );
}
