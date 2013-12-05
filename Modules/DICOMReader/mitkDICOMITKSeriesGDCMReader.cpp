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

mitk::DICOMITKSeriesGDCMReader
::DICOMITKSeriesGDCMReader()
:DICOMFileReader()
,m_FixTiltByShearing(false)
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
::AnalyzeInputFiles()
{
  this->ClearOutputs();

  // prepare initial sorting (== list of input files)

  StringList inputFilenames = this->GetInputFiles();
  DICOMImageFrameList initialFramelist;
  for (StringList::const_iterator inputIter = inputFilenames.begin();
       inputIter != inputFilenames.end();
       ++inputIter)
  {
    // TODO check DICOMness and non-multi-framedness
    initialFramelist.push_back( DICOMImageFrameInfo::New(*inputIter, 0) );
  }
  m_SortingResultInProgress.push_back( initialFramelist );

  /*
     PSEUDO CODE

     // scan files for sorting-relevant tags

     gdcm::Scanner gdcmScanner;

     foreach(DICOMDataSetSorter sorter, m_Sorter)
     {
       TagList tags = sorter->GetTagsOfInterest();
       foreach(Tag, tags)
       {
         gdcmScanner.AddTag( tag );
       }
     }

     // sort and split blocks as configured

     DICOMImageFrameInfo nextStepSorting; // we should not modify our input list while processing it
     foreach(DICOMDataSetSorter sorter, m_Sorter)
     {
       nextStepSorting.clear();

       foreach(DICOMImageFrameList framelist, m_SortingResultInProgress)
       {
         sorter->SetInput(framelist);
         sorter->Sort();
         unsigned int numberOfResultingBlocks = sorter->GetNumberOfOutputs();
         for (unsigned int b = 0; b < numberOfResultingBlocks; ++b)
         {
           DICOMImageFrameList blockResult = sorter->GetOutput(b);
           nextStepSorting.push_back( blockResult );
         }
       }

       m_SortingResultInProgress = nextStepSorting;
     }

  */

  // provide final result as output

  this->SetNumberOfOutputs( m_SortingResultInProgress.size() );
  unsigned int o = 0;
  for (SortingBlockList::iterator blockIter = m_SortingResultInProgress.begin();
       blockIter != m_SortingResultInProgress.end();
       ++o, ++blockIter)
  {
    DICOMImageBlockDescriptor block;
    block.SetImageFrameList( *blockIter );
    this->SetOutput( o, block );
  }
}

// void AllocateOutputImages();

bool
mitk::DICOMITKSeriesGDCMReader
::LoadImages()
{
  // does nothing
  return true;
}

bool
mitk::DICOMITKSeriesGDCMReader
::CanHandleFile(const std::string& itkNotUsed(filename))
{
  return true; // can handle all
}
