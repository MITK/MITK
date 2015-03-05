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

//#define MBILOG_ENABLE_DEBUG
//#define ENABLE_TIMING

#include "mitkDICOMITKSeriesGDCMReader.h"
#include "mitkITKDICOMSeriesReaderHelper.h"
#include "mitkGantryTiltInformation.h"
#include "mitkDICOMTagBasedSorter.h"
#include "mitkDICOMGDCMTagScanner.h"

#include <itkTimeProbesCollectorBase.h>

#include <gdcmUIDs.h>

mitk::DICOMITKSeriesGDCMReader
::DICOMITKSeriesGDCMReader(unsigned int decimalPlacesForOrientation)
:DICOMFileReader()
,m_FixTiltByShearing(true)
,m_DecimalPlacesForOrientation(decimalPlacesForOrientation)
{
  this->EnsureMandatorySortersArePresent(decimalPlacesForOrientation);
}

mitk::DICOMITKSeriesGDCMReader
::DICOMITKSeriesGDCMReader(const DICOMITKSeriesGDCMReader& other )
:DICOMFileReader(other)
,m_FixTiltByShearing(false)
,m_SortingResultInProgress( other.m_SortingResultInProgress )
,m_Sorter( other.m_Sorter )
,m_EquiDistantBlocksSorter( other.m_EquiDistantBlocksSorter->Clone() )
,m_NormalDirectionConsistencySorter( other.m_NormalDirectionConsistencySorter->Clone() )
,m_ReplacedCLocales( other.m_ReplacedCLocales )
,m_ReplacedCinLocales( other.m_ReplacedCinLocales )
,m_DecimalPlacesForOrientation(other.m_DecimalPlacesForOrientation)
,m_TagCache( other.m_TagCache )
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
    this->m_FixTiltByShearing = other.m_FixTiltByShearing;
    this->m_SortingResultInProgress = other.m_SortingResultInProgress;
    this->m_Sorter = other.m_Sorter; // TODO should clone the list items
    this->m_EquiDistantBlocksSorter = other.m_EquiDistantBlocksSorter->Clone();
    this->m_NormalDirectionConsistencySorter = other.m_NormalDirectionConsistencySorter->Clone();
    this->m_ReplacedCLocales = other.m_ReplacedCLocales;
    this->m_ReplacedCinLocales = other.m_ReplacedCinLocales;
    this->m_DecimalPlacesForOrientation = other.m_DecimalPlacesForOrientation;
    this->m_TagCache = other.m_TagCache;
  }
  return *this;
}

bool
mitk::DICOMITKSeriesGDCMReader
::operator==(const DICOMFileReader& other) const
{
  if (const Self* otherSelf = dynamic_cast<const Self*>(&other))
  {
    if ( this->m_FixTiltByShearing == otherSelf->m_FixTiltByShearing
      && *(this->m_EquiDistantBlocksSorter) == *(otherSelf->m_EquiDistantBlocksSorter)
      && (fabs(this->m_DecimalPlacesForOrientation - otherSelf->m_DecimalPlacesForOrientation) < eps)
       )
    {
      // test sorters for equality
      if (this->m_Sorter.size() != otherSelf->m_Sorter.size()) return false;

      SorterList::const_iterator mySorterIter = this->m_Sorter.begin();
      SorterList::const_iterator oSorterIter = otherSelf->m_Sorter.begin();
      for(; mySorterIter != this->m_Sorter.end() && oSorterIter != otherSelf->m_Sorter.end();
          ++mySorterIter, ++oSorterIter)
      {
        if ( ! (**mySorterIter == **oSorterIter ) ) return false; // this sorter differs
      }

      // nothing differs ==> all is equal
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

void
mitk::DICOMITKSeriesGDCMReader
::SetFixTiltByShearing(bool on)
{
  m_FixTiltByShearing = on;
}

bool
mitk::DICOMITKSeriesGDCMReader
::GetFixTiltByShearing() const
{
  return m_FixTiltByShearing;
}

void
mitk::DICOMITKSeriesGDCMReader
::SetAcceptTwoSlicesGroups(bool accept)
{
  m_EquiDistantBlocksSorter->SetAcceptTwoSlicesGroups(accept);
}

bool
mitk::DICOMITKSeriesGDCMReader
::GetAcceptTwoSlicesGroups() const
{
  return m_EquiDistantBlocksSorter->GetAcceptTwoSlicesGroups();
}


mitk::DICOMGDCMImageFrameList
mitk::DICOMITKSeriesGDCMReader
::FromDICOMDatasetList(const DICOMDatasetList& input)
{
  DICOMGDCMImageFrameList output;
  output.reserve(input.size());

  for(DICOMDatasetList::const_iterator inputIter = input.begin();
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
::ToDICOMDatasetList(const DICOMGDCMImageFrameList& input)
{
  DICOMDatasetList output;
  output.reserve(input.size());

  for(DICOMGDCMImageFrameList::const_iterator inputIter = input.begin();
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
::ToDICOMImageFrameList(const DICOMGDCMImageFrameList& input)
{
  DICOMImageFrameList output;
  output.reserve(input.size());

  for(DICOMGDCMImageFrameList::const_iterator inputIter = input.begin();
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
::InternalPrintConfiguration(std::ostream& os) const
{
  unsigned int sortIndex(1);
  for(SorterList::const_iterator sorterIter = m_Sorter.begin();
      sorterIter != m_Sorter.end();
      ++sortIndex, ++sorterIter)
  {
    os << "Sorting step " << sortIndex << ":" << std::endl;
    (*sorterIter)->PrintConfiguration(os, "  ");
  }

  os << "Sorting step " << sortIndex << ":" << std::endl;
  m_EquiDistantBlocksSorter->PrintConfiguration(os, "  ");
}


std::string
mitk::DICOMITKSeriesGDCMReader
::GetActiveLocale() const
{
  return setlocale(LC_NUMERIC, NULL);
}

void
mitk::DICOMITKSeriesGDCMReader
::PushLocale() const
{
  std::string currentCLocale = setlocale(LC_NUMERIC, NULL);
  m_ReplacedCLocales.push( currentCLocale );
  setlocale(LC_NUMERIC, "C");

  std::locale currentCinLocale( std::cin.getloc() );
  m_ReplacedCinLocales.push( currentCinLocale );
  std::locale l( "C" );
  std::cin.imbue(l);
}

void
mitk::DICOMITKSeriesGDCMReader
::PopLocale() const
{
  if (!m_ReplacedCLocales.empty())
  {
    setlocale(LC_NUMERIC, m_ReplacedCLocales.top().c_str());
    m_ReplacedCLocales.pop();
  }
  else
  {
    MITK_WARN << "Mismatched PopLocale on DICOMITKSeriesGDCMReader.";
  }

  if (!m_ReplacedCinLocales.empty())
  {
    std::cin.imbue( m_ReplacedCinLocales.top() );
    m_ReplacedCinLocales.pop();
  }
  else
  {
    MITK_WARN << "Mismatched PopLocale on DICOMITKSeriesGDCMReader.";
  }

}

mitk::DICOMITKSeriesGDCMReader::SortingBlockList
mitk::DICOMITKSeriesGDCMReader
::Condense3DBlocks(SortingBlockList& input)
{
  return input; // to be implemented differently by sub-classes
}

#if defined(MBILOG_ENABLE_DEBUG) || defined (ENABLE_TIMING)
  #define timeStart(part) timer.Start(part);
  #define timeStop(part) timer.Stop(part);
#else
  #define timeStart(part)
  #define timeStop(part)
#endif

void
mitk::DICOMITKSeriesGDCMReader
::AnalyzeInputFiles()
{
  itk::TimeProbesCollectorBase timer;

  timeStart("Reset");
  this->ClearOutputs();
  timeStop("Reset");

  // prepare initial sorting (== list of input files)
  StringList inputFilenames = this->GetInputFiles();
  timeStart("Check input for DCM");
  if ( inputFilenames.empty()
       ||
       !this->CanHandleFile( inputFilenames.front() ) // first
       ||
       !this->CanHandleFile( inputFilenames.back() ) // last
       ||
       !this->CanHandleFile( inputFilenames[ inputFilenames.size() / 2] ) // roughly central file
     )
  {
    // TODO a read-as-many-as-possible fallback could be implemented here
    MITK_DEBUG << "Reader unable to process files..";
    return;
  }

  timeStop("Check input for DCM");

  // scan files for sorting-relevant tags
  if (m_TagCache.IsNull())
  {
    timeStart("Tag scanning");
    DICOMGDCMTagScanner::Pointer filescanner = DICOMGDCMTagScanner::New();
    m_TagCache = filescanner.GetPointer(); // keep alive and make accessible to sub-classes

    filescanner->SetInputFiles(inputFilenames);
    filescanner->AddTags( this->GetTagsOfInterest() );

    PushLocale();
    filescanner->Scan();
    PopLocale();

    timeStop("Tag scanning");
  }
  else
  {
    // ensure that the tag cache contains our required tags AND files and has scanned!
  }

  m_SortingResultInProgress.clear();
  // TODO We should remove the following cast
  // DICOMImageFrameInfo would need to inherit DICOMDatasetAccess!
  // - then the DICOMGDCMTagScanner class could create a DICOMGDCMImageFrameList internally
  //   - and return it as a DICOMImageFrameList
  // - like this, DICOMITKSeriesGDCMReader could use the DICOMImageFrameInfoList to feed its sorters
  // - problem:
  //   - DICOMImageFrameInfo is also part of DICOMImageBlockDescriptor, which is meant
  //     to describe the scanner output, even after the reader (and its scanner) is deleted.
  //   - if DICOMImageFrameInfo now inherits DICOMDatasetAccess, it would also need to implement
  //     GetTagValueAsString().
  //     - so this could all work if we implement a default response in DICOMImageFrameInfo::GetTagValueAsString() (like in GetFilenameIfAvailable)
  //       and overwrite it in DICOMGDCMImageFrameInfo, which also knows about a specific GDCM scanner result
  //       (which again COULD (no need to?) be hidden as a point to a DICOMGDCMTagScanner class)
  //
  if ( DICOMGDCMTagScanner* tagCache = dynamic_cast<DICOMGDCMTagScanner*>(m_TagCache.GetPointer()) )
  {
    m_SortingResultInProgress.push_back( tagCache->GetFrameInfoList() );
  }
  else
  {
    throw std::logic_error("Bad implementation error: DICOMITKSeriesGDCMReader now unable to find dataset/tag information for its input.");
  }

  // sort and split blocks as configured

  timeStart("Sorting frames");
  unsigned int sorterIndex = 0;
  for(SorterList::iterator sorterIter = m_Sorter.begin();
      sorterIter != m_Sorter.end();
      ++sorterIndex, ++sorterIter)
  {
    std::stringstream ss; ss << "Sorting step " << sorterIndex;
    timeStart( ss.str().c_str() );
    m_SortingResultInProgress = this->InternalExecuteSortingStep(sorterIndex, *sorterIter, m_SortingResultInProgress);
    timeStop( ss.str().c_str() );
  }

  // a last extra-sorting step: ensure equidistant slices
  timeStart( "EquiDistantBlocksSorter" );
  m_SortingResultInProgress = this->InternalExecuteSortingStep(sorterIndex++, m_EquiDistantBlocksSorter.GetPointer(), m_SortingResultInProgress);
  timeStop( "EquiDistantBlocksSorter" );

  timeStop("Sorting frames");

  timeStart("Condensing 3D blocks");
  m_SortingResultInProgress = this->Condense3DBlocks( m_SortingResultInProgress );
  timeStop("Condensing 3D blocks");

  // provide final result as output

  timeStart("Output");
  unsigned int o = this->GetNumberOfOutputs();
  this->SetNumberOfOutputs( o + m_SortingResultInProgress.size() ); // Condense3DBlocks may already have added outputs!
  for (SortingBlockList::iterator blockIter = m_SortingResultInProgress.begin();
       blockIter != m_SortingResultInProgress.end();
       ++o, ++blockIter)
  {
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

    block.SetReaderImplementationLevel( this->GetReaderImplementationLevel( block.GetSOPClassUID() ) );

    this->SetOutput( o, block );
  }
  timeStop("Output");

#if defined(MBILOG_ENABLE_DEBUG) || defined (ENABLE_TIMING)
  std::cout << "---------------------------------------------------------------" << std::endl;
  timer.Report( std::cout );
  std::cout << "---------------------------------------------------------------" << std::endl;
#endif
}

mitk::DICOMITKSeriesGDCMReader::SortingBlockList
mitk::DICOMITKSeriesGDCMReader
::InternalExecuteSortingStep(
    unsigned int sortingStepIndex,
    DICOMDatasetSorter::Pointer sorter,
    const SortingBlockList& input)
{
  SortingBlockList nextStepSorting; // we should not modify our input list while processing it
  std::stringstream ss; ss << "Sorting step " << sortingStepIndex << " '";
#if defined(MBILOG_ENABLE_DEBUG)
  sorter->PrintConfiguration(ss);
#endif
  ss << "'";
  nextStepSorting.clear();

  MITK_DEBUG << "================================================================================";
  MITK_DEBUG << "DICOMITKSeriesGDCMReader: " << ss.str() << ": " << input.size() << " groups input";
  unsigned int groupIndex = 0;

  for(SortingBlockList::const_iterator blockIter = input.begin();
      blockIter != input.end();
      ++groupIndex, ++blockIter)
  {
    const DICOMGDCMImageFrameList& gdcmInfoFrameList = *blockIter;
    DICOMDatasetList datasetList = ToDICOMDatasetList( gdcmInfoFrameList );

    MITK_DEBUG << "--------------------------------------------------------------------------------";
    MITK_DEBUG << "DICOMITKSeriesGDCMReader: " << ss.str() << ", dataset group " << groupIndex << " (" << datasetList.size() << " datasets): ";
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

  return nextStepSorting;
}

mitk::ReaderImplementationLevel
mitk::DICOMITKSeriesGDCMReader
::GetReaderImplementationLevel(const std::string sopClassUID) const
{
  if (sopClassUID.empty())
  {
    return SOPClassUnknown;
  }

  gdcm::UIDs uidKnowledge;
  uidKnowledge.SetFromUID( sopClassUID.c_str() );

  gdcm::UIDs::TSType gdcmType = uidKnowledge;

  switch (gdcmType)
  {
    case gdcm::UIDs::CTImageStorage:
    case gdcm::UIDs::MRImageStorage:
    case gdcm::UIDs::PositronEmissionTomographyImageStorage:
    case gdcm::UIDs::ComputedRadiographyImageStorage:
    case gdcm::UIDs::DigitalXRayImageStorageForPresentation:
    case gdcm::UIDs::DigitalXRayImageStorageForProcessing:
      return SOPClassSupported;

    case gdcm::UIDs::NuclearMedicineImageStorage:
      return SOPClassPartlySupported;

    case gdcm::UIDs::SecondaryCaptureImageStorage:
      return SOPClassImplemented;

    default:
      return SOPClassUnsupported;
  }
}

// void AllocateOutputImages();

bool
mitk::DICOMITKSeriesGDCMReader
::LoadImages()
{
  bool success = true;

  unsigned int numberOfOutputs = this->GetNumberOfOutputs();
  for (unsigned int o = 0; o < numberOfOutputs; ++o)
  {
    success &= this->LoadMitkImageForOutput(o);
  }

  return success;
}

bool
mitk::DICOMITKSeriesGDCMReader
::LoadMitkImageForImageBlockDescriptor(DICOMImageBlockDescriptor& block) const
{
  PushLocale();
  const DICOMImageFrameList& frames = block.GetImageFrameList();
  const GantryTiltInformation tiltInfo = block.GetTiltInformation();
  bool hasTilt = tiltInfo.IsRegularGantryTilt();

  ITKDICOMSeriesReaderHelper::StringContainer filenames;
  for (DICOMImageFrameList::const_iterator frameIter = frames.begin();
      frameIter != frames.end();
      ++frameIter)
  {
    filenames.push_back( (*frameIter)->Filename );
  }

  mitk::ITKDICOMSeriesReaderHelper helper;
  bool success(true);
  try
  {
    mitk::Image::Pointer mitkImage = helper.Load( filenames, m_FixTiltByShearing && hasTilt, tiltInfo );
    block.SetMitkImage( mitkImage );
  }
  catch (std::exception& e)
  {
    success = false;
    MITK_ERROR << "Exception during image loading: " << e.what();
  }

  PopLocale();

  return success;
}


bool
mitk::DICOMITKSeriesGDCMReader
::LoadMitkImageForOutput(unsigned int o)
{
  DICOMImageBlockDescriptor& block = this->InternalGetOutput(o);
  return this->LoadMitkImageForImageBlockDescriptor(block);
}


bool
mitk::DICOMITKSeriesGDCMReader
::CanHandleFile(const std::string& filename)
{
  return ITKDICOMSeriesReaderHelper::CanHandleFile(filename);
}

void
mitk::DICOMITKSeriesGDCMReader
::AddSortingElement(DICOMDatasetSorter* sorter, bool atFront)
{
  assert(sorter);

  if (atFront)
  {
    m_Sorter.push_front( sorter );
  }
  else
  {
    m_Sorter.push_back( sorter );
  }
}

mitk::DICOMITKSeriesGDCMReader::ConstSorterList
mitk::DICOMITKSeriesGDCMReader
::GetFreelyConfiguredSortingElements() const
{
  std::list<DICOMDatasetSorter::ConstPointer> result;

  unsigned int sortIndex(0);
  for(SorterList::const_iterator sorterIter = m_Sorter.begin();
      sorterIter != m_Sorter.end();
      ++sortIndex, ++sorterIter)
  {
    if (sortIndex > 0) // ignore first element (see EnsureMandatorySortersArePresent)
    {
      result.push_back( (*sorterIter).GetPointer() );
    }
  }

  return result;
}

void
mitk::DICOMITKSeriesGDCMReader
::EnsureMandatorySortersArePresent(unsigned int decimalPlacesForOrientation)
{
  DICOMTagBasedSorter::Pointer splitter = DICOMTagBasedSorter::New();
  splitter->AddDistinguishingTag( DICOMTag(0x0028, 0x0010) ); // Number of Rows
  splitter->AddDistinguishingTag( DICOMTag(0x0028, 0x0011) ); // Number of Columns
  splitter->AddDistinguishingTag( DICOMTag(0x0028, 0x0030) ); // Pixel Spacing
  splitter->AddDistinguishingTag( DICOMTag(0x0018, 0x1164) ); // Imager Pixel Spacing
  splitter->AddDistinguishingTag( DICOMTag(0x0020, 0x0037), new mitk::DICOMTagBasedSorter::CutDecimalPlaces(decimalPlacesForOrientation) ); // Image Orientation (Patient)
  splitter->AddDistinguishingTag( DICOMTag(0x0018, 0x0050) ); // Slice Thickness
  splitter->AddDistinguishingTag( DICOMTag(0x0028, 0x0008) ); // Number of Frames
  this->AddSortingElement( splitter, true ); // true = at front

  if (m_EquiDistantBlocksSorter.IsNull())
  {
    m_EquiDistantBlocksSorter = mitk::EquiDistantBlocksSorter::New();
  }
  m_EquiDistantBlocksSorter->SetAcceptTilt( m_FixTiltByShearing );

  if (m_NormalDirectionConsistencySorter.IsNull())
  {
    m_NormalDirectionConsistencySorter = mitk::NormalDirectionConsistencySorter::New();
  }
}

void
mitk::DICOMITKSeriesGDCMReader
::SetToleratedOriginOffsetToAdaptive(double fractionOfInterSliceDistance)
{
  assert( m_EquiDistantBlocksSorter.IsNotNull() );
  m_EquiDistantBlocksSorter->SetToleratedOriginOffsetToAdaptive(fractionOfInterSliceDistance);
}

void
mitk::DICOMITKSeriesGDCMReader
::SetToleratedOriginOffset(double millimeters)
{
  assert( m_EquiDistantBlocksSorter.IsNotNull() );
  m_EquiDistantBlocksSorter->SetToleratedOriginOffset(millimeters);
}

double
mitk::DICOMITKSeriesGDCMReader
::GetToleratedOriginError() const
{
  assert( m_EquiDistantBlocksSorter.IsNotNull() );
  return m_EquiDistantBlocksSorter->GetToleratedOriginOffset();
}

bool
mitk::DICOMITKSeriesGDCMReader
::IsToleratedOriginOffsetAbsolute() const
{
  assert( m_EquiDistantBlocksSorter.IsNotNull() );
  return m_EquiDistantBlocksSorter->IsToleratedOriginOffsetAbsolute();
}

double
mitk::DICOMITKSeriesGDCMReader
::GetDecimalPlacesForOrientation() const
{
  return m_DecimalPlacesForOrientation;
}

mitk::DICOMTagCache::Pointer
mitk::DICOMITKSeriesGDCMReader
::GetTagCache() const
{
  return m_TagCache;
}

void
mitk::DICOMITKSeriesGDCMReader
::SetTagCache(DICOMTagCache::Pointer tagCache)
{
  m_TagCache = tagCache;
}

mitk::DICOMTagList
mitk::DICOMITKSeriesGDCMReader
::GetTagsOfInterest() const
{
  DICOMTagList completeList;

  // check all configured sorters
  for(SorterList::const_iterator sorterIter = m_Sorter.begin();
      sorterIter != m_Sorter.end();
      ++sorterIter)
  {
    assert(sorterIter->IsNotNull());

    DICOMTagList tags = (*sorterIter)->GetTagsOfInterest();
    completeList.insert( completeList.end(), tags.begin(), tags.end() );
  }

  // check our own forced sorters
  DICOMTagList tags = m_EquiDistantBlocksSorter->GetTagsOfInterest();
  completeList.insert( completeList.end(), tags.begin(), tags.end() );

  tags = m_NormalDirectionConsistencySorter->GetTagsOfInterest();
  completeList.insert( completeList.end(), tags.begin(), tags.end() );

  // add the tags for DICOMImageBlockDescriptor
  tags = DICOMImageBlockDescriptor::GetTagsOfInterest();
  completeList.insert( completeList.end(), tags.begin(), tags.end() );

  return completeList;
}
