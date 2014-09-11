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

#include "mitkThreeDnTDICOMSeriesReader.h"
#include "mitkITKDICOMSeriesReaderHelper.h"

mitk::ThreeDnTDICOMSeriesReader
::ThreeDnTDICOMSeriesReader(unsigned int decimalPlacesForOrientation)
:DICOMITKSeriesGDCMReader(decimalPlacesForOrientation)
,m_Group3DandT(true)
{
}

mitk::ThreeDnTDICOMSeriesReader
::ThreeDnTDICOMSeriesReader(const ThreeDnTDICOMSeriesReader& other )
:DICOMITKSeriesGDCMReader(other)
,m_Group3DandT(true)
{
}

mitk::ThreeDnTDICOMSeriesReader
::~ThreeDnTDICOMSeriesReader()
{
}

mitk::ThreeDnTDICOMSeriesReader&
mitk::ThreeDnTDICOMSeriesReader
::operator=(const ThreeDnTDICOMSeriesReader& other)
{
  if (this != &other)
  {
    DICOMITKSeriesGDCMReader::operator=(other);
    this->m_Group3DandT = other.m_Group3DandT;
  }
  return *this;
}

bool
mitk::ThreeDnTDICOMSeriesReader
::operator==(const DICOMFileReader& other) const
{
  if (const Self* otherSelf = dynamic_cast<const Self*>(&other))
  {
    return
       DICOMITKSeriesGDCMReader::operator==(other)
    && this->m_Group3DandT == otherSelf->m_Group3DandT;
  }
  else
  {
    return false;
  }
}

void
mitk::ThreeDnTDICOMSeriesReader
::SetGroup3DandT(bool on)
{
  m_Group3DandT = on;
}

bool
mitk::ThreeDnTDICOMSeriesReader
::GetGroup3DandT() const
{
  return m_Group3DandT;
}

mitk::DICOMITKSeriesGDCMReader::SortingBlockList
mitk::ThreeDnTDICOMSeriesReader
::Condense3DBlocks(SortingBlockList& resultOf3DGrouping)
{
  if (!m_Group3DandT)
  {
    return resultOf3DGrouping; // don't work if nobody asks us to
  }

  SortingBlockList remainingBlocks = resultOf3DGrouping;

  SortingBlockList non3DnTBlocks;
  SortingBlockList true3DnTBlocks;
  std::vector<unsigned int> true3DnTBlocksTimeStepCount;

  // we should describe our need for this tag as needed via a function
  // (however, we currently know that the superclass will always need this tag)
  const DICOMTag tagImagePositionPatient(0x0020, 0x0032);

  while (!remainingBlocks.empty())
  {
    // new block to fill up
    DICOMGDCMImageFrameList& firstBlock = remainingBlocks.front();
    DICOMGDCMImageFrameList current3DnTBlock = firstBlock;
    int current3DnTBlockNumberOfTimeSteps = 1;

    // get block characteristics of first block
    unsigned int currentBlockNumberOfSlices = firstBlock.size();
    std::string currentBlockFirstOrigin = firstBlock.front()->GetTagValueAsString( tagImagePositionPatient );
    std::string currentBlockLastOrigin  =  firstBlock.back()->GetTagValueAsString( tagImagePositionPatient );

    remainingBlocks.pop_front();

    // compare all other blocks against the first one
    for (SortingBlockList::iterator otherBlockIter = remainingBlocks.begin();
         otherBlockIter != remainingBlocks.end();
         /*++otherBlockIter*/) // <-- inside loop
    {
      // get block characteristics from first block
      DICOMGDCMImageFrameList& otherBlock = *otherBlockIter;

      unsigned int otherBlockNumberOfSlices = otherBlock.size();
      std::string otherBlockFirstOrigin = otherBlock.front()->GetTagValueAsString( tagImagePositionPatient );
      std::string otherBlockLastOrigin  =  otherBlock.back()->GetTagValueAsString( tagImagePositionPatient );

      // add matching blocks to current3DnTBlock
      // keep other blocks for later
      if (   otherBlockNumberOfSlices == currentBlockNumberOfSlices
          && otherBlockFirstOrigin == currentBlockFirstOrigin
          && otherBlockLastOrigin == currentBlockLastOrigin
          )
      { // matching block
        ++current3DnTBlockNumberOfTimeSteps;
        current3DnTBlock.insert( current3DnTBlock.end(), otherBlock.begin(), otherBlock.end() ); // append
        // remove this block from remainingBlocks
        otherBlockIter = remainingBlocks.erase(otherBlockIter); // make sure iterator otherBlockIter is valid afterwards
      }
      else
      {
        ++otherBlockIter;
      }
    }

    // in any case, we now now all about the first block of our list ...
    // ... and we wither call it 3D o 3D+t
    if (current3DnTBlockNumberOfTimeSteps > 1)
    {
      true3DnTBlocks.push_back(current3DnTBlock);
      true3DnTBlocksTimeStepCount.push_back(current3DnTBlockNumberOfTimeSteps);
    }
    else
    {
      non3DnTBlocks.push_back(current3DnTBlock);
    }
  }

  // create output for real 3D+t blocks (other outputs will be created by superclass)
  // set 3D+t flag on output block
  this->SetNumberOfOutputs( true3DnTBlocks.size() );
  unsigned int o = 0;
  for (SortingBlockList::iterator blockIter = true3DnTBlocks.begin();
       blockIter != true3DnTBlocks.end();
       ++o, ++blockIter)
  {
    // bad copy&paste code from DICOMITKSeriesGDCMReader, should be handled in a better way
    DICOMGDCMImageFrameList& gdcmFrameInfoList = *blockIter;
    assert(!gdcmFrameInfoList.empty());

    // reverse frames if necessary
    // update tilt information from absolute last sorting
    DICOMDatasetList datasetList = ToDICOMDatasetList( gdcmFrameInfoList );
    m_NormalDirectionConsistencySorter->SetInput( datasetList );
    m_NormalDirectionConsistencySorter->Sort();
    DICOMGDCMImageFrameList sortedGdcmInfoFrameList = FromDICOMDatasetList( m_NormalDirectionConsistencySorter->GetOutput(0) );
    const GantryTiltInformation& tiltInfo = m_NormalDirectionConsistencySorter->GetTiltInformation();

    // set frame list for current block
    DICOMImageFrameList frameList = ToDICOMImageFrameList( sortedGdcmInfoFrameList );
    assert(!frameList.empty());

    DICOMImageBlockDescriptor block;
    block.SetTagCache( this->GetTagCache() ); // important: this must be before SetImageFrameList(), because SetImageFrameList will trigger reading of lots of interesting tags!
    block.SetImageFrameList( frameList );
    block.SetTiltInformation( tiltInfo );

    block.SetFlag("3D+t", true);
    block.SetIntProperty("timesteps", true3DnTBlocksTimeStepCount[o]);
    MITK_DEBUG << "Found " << true3DnTBlocksTimeStepCount[o] << " timesteps";

    this->SetOutput( o, block );
  }

  return non3DnTBlocks;
}

bool
mitk::ThreeDnTDICOMSeriesReader
::LoadImages()
{
  bool success = true;

  unsigned int numberOfOutputs = this->GetNumberOfOutputs();
  for (unsigned int o = 0; o < numberOfOutputs; ++o)
  {
    DICOMImageBlockDescriptor& block = this->InternalGetOutput(o);

    if (block.GetFlag("3D+t", false))
    {
      success &= this->LoadMitkImageForOutput(o);
    }
    else
    {
      success &= DICOMITKSeriesGDCMReader::LoadMitkImageForOutput(o); // let superclass handle non-3D+t
    }
  }

  return success;
}

bool
mitk::ThreeDnTDICOMSeriesReader
::LoadMitkImageForImageBlockDescriptor(DICOMImageBlockDescriptor& block) const
{
  PushLocale();
  const DICOMImageFrameList& frames = block.GetImageFrameList();
  const GantryTiltInformation tiltInfo = block.GetTiltInformation();
  bool hasTilt = tiltInfo.IsRegularGantryTilt();

  int numberOfTimesteps = block.GetIntProperty("timesteps", 1);

  if (numberOfTimesteps == 1)
  {
    return DICOMITKSeriesGDCMReader::LoadMitkImageForImageBlockDescriptor(block);
  }

  int numberOfFramesPerTimestep = frames.size() / numberOfTimesteps;
  assert( int(double((double)frames.size() / (double)numberOfTimesteps ))
       == numberOfFramesPerTimestep ); // this should hold

  ITKDICOMSeriesReaderHelper::StringContainerList filenamesPerTimestep;
  for (int timeStep = 0; timeStep<numberOfTimesteps; ++timeStep)
  {
    // use numberOfFramesPerTimestep frames for a new item in filenamesPerTimestep
    ITKDICOMSeriesReaderHelper::StringContainer filenamesOfThisTimeStep;
    DICOMImageFrameList::const_iterator timeStepStart = frames.begin() + timeStep * numberOfFramesPerTimestep;
    DICOMImageFrameList::const_iterator timeStepEnd   = frames.begin() + (timeStep+1) * numberOfFramesPerTimestep;
    for (DICOMImageFrameList::const_iterator frameIter = timeStepStart;
        frameIter != timeStepEnd;
        ++frameIter)
    {
      filenamesOfThisTimeStep.push_back( (*frameIter)->Filename );
    }
    filenamesPerTimestep.push_back( filenamesOfThisTimeStep );
  }

  mitk::ITKDICOMSeriesReaderHelper helper;
  mitk::Image::Pointer mitkImage = helper.Load3DnT( filenamesPerTimestep, m_FixTiltByShearing && hasTilt, tiltInfo );

  block.SetMitkImage( mitkImage );

  PopLocale();

  return true;
}
