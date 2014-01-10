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
::ThreeDnTDICOMSeriesReader()
:DICOMITKSeriesGDCMReader()
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

void
mitk::ThreeDnTDICOMSeriesReader
::SetGroup3DandT(bool on)
{
  m_Group3DandT = on;
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

  // TODO we should provide this tag as needed via a function
  // (however, we currently know that the superclass will use this tag)
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
    DICOMGDCMImageFrameList& gdcmFrameInfoList = *blockIter;
    DICOMImageFrameList frameList = ToDICOMImageFrameList( gdcmFrameInfoList );
    assert(!gdcmFrameInfoList.empty());
    assert(!frameList.empty());

    DICOMImageBlockDescriptor block;
    block.SetTagCache( this ); // important: this must be before SetImageFrameList(), because SetImageFrameList will trigger reading of lots of interesting tags!
    block.SetImageFrameList( frameList );

    // bad copy&paste code, should be handled in a better way

    const GantryTiltInformation& tiltInfo = m_EquiDistantBlocksSorter->GetTiltInformation( (gdcmFrameInfoList.front())->GetFilenameIfAvailable() );
    block.SetTiltInformation( tiltInfo );

    // assume
    static const DICOMTag tagPixelSpacing(0x0028,0x0030);
    static const DICOMTag tagImagerPixelSpacing(0x0018,0x1164);
    std::string pixelSpacingString = (gdcmFrameInfoList.front())->GetTagValueAsString( tagPixelSpacing );
    std::string imagerPixelSpacingString = gdcmFrameInfoList.front()->GetTagValueAsString( tagImagerPixelSpacing );
    block.SetPixelSpacingTagValues(pixelSpacingString, imagerPixelSpacingString);

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
  bool hasTilt = block.GetFlag("gantryTilt", false);

  int numberOfTimesteps = block.GetIntProperty("timesteps", 1);
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
  mitk::Image::Pointer mitkImage = helper.Load3DnT( filenamesPerTimestep, m_FixTiltByShearing && hasTilt, tiltInfo ); // TODO preloaded images, caching..?

  block.SetMitkImage( mitkImage );

  PopLocale();

  return true;
}
