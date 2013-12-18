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
#include "mitkDICOMTagBasedSorter.h"

#include <itkTimeProbesCollectorBase.h>

#include <gdcmScanner.h>
#include <gdcmUIDs.h>

mitk::DICOMITKSeriesGDCMReader
::DICOMITKSeriesGDCMReader()
:DICOMFileReader()
,m_FixTiltByShearing(true)
{
}

mitk::DICOMITKSeriesGDCMReader
::DICOMITKSeriesGDCMReader(const DICOMITKSeriesGDCMReader& other )
:DICOMFileReader(other)
,m_FixTiltByShearing(false)
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
    this->m_Sorter = other.m_Sorter; // TODO should clone the list items
    this->m_EquiDistantBlocksSorter = other.m_EquiDistantBlocksSorter->Clone();
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

std::string
mitk::DICOMITKSeriesGDCMReader
::GetActiveLocale() const
{
  return setlocale(LC_NUMERIC, NULL);
}

void
mitk::DICOMITKSeriesGDCMReader
::PushLocale()
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
::PopLocale()
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

void
mitk::DICOMITKSeriesGDCMReader
::AnalyzeInputFiles()
{
  // at this point, make sure we have a sorting element at the end that splits geometrically separate blocks
  this->EnsureMandatorySortersArePresent();

  itk::TimeProbesCollectorBase timer;

  timer.Start("Reset");
  this->ClearOutputs();
  m_InputFrameList.clear();
  m_GDCMScanner.ClearTags();
  timer.Stop("Reset");

  // prepare initial sorting (== list of input files)
  StringList inputFilenames = this->GetInputFiles();

  // scan files for sorting-relevant tags

  timer.Start("Setup scanning");
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
      m_GDCMScanner.AddTag( gdcm::Tag(tagIter->GetGroup(), tagIter->GetElement()) );
    }
  }

  // Add some of our own interest
  // TODO all tags that are needed in DICOMImageBlockDescriptor should be added by DICOMFileReader (this code location here should query all superclasses for tags)
  m_GDCMScanner.AddTag( gdcm::Tag(0x0018,0x1164) ); // pixel spacing
  m_GDCMScanner.AddTag( gdcm::Tag(0x0028,0x0030) ); // imager pixel spacing

  m_GDCMScanner.AddTag( gdcm::Tag(0x0020,0x1041) ); // slice location
  m_GDCMScanner.AddTag( gdcm::Tag(0x0020,0x0013) ); // instance number
  m_GDCMScanner.AddTag( gdcm::Tag(0x0008,0x0016) ); // sop class UID
  m_GDCMScanner.AddTag( gdcm::Tag(0x0008,0x0018) ); // sop instance UID

  m_GDCMScanner.AddTag( gdcm::Tag(0x0008,0x0060) ); // modality
  m_GDCMScanner.AddTag( gdcm::Tag(0x0020,0x0012) ); // acquisition number
  m_GDCMScanner.AddTag( gdcm::Tag(0x0018,0x0024) ); // sequence name
  m_GDCMScanner.AddTag( gdcm::Tag(0x0020,0x0037) ); // image orientation
  m_GDCMScanner.AddTag( gdcm::Tag(0x0020,0x0032) ); // ipp

  timer.Stop("Setup scanning");

  timer.Start("Tag scanning");
  PushLocale();
  m_GDCMScanner.Scan( inputFilenames );
  PopLocale();
  timer.Stop("Tag scanning");

  timer.Start("Setup sorting");
  for (StringList::const_iterator inputIter = inputFilenames.begin();
       inputIter != inputFilenames.end();
       ++inputIter)
  {
    m_InputFrameList.push_back( DICOMGDCMImageFrameInfo::New( DICOMImageFrameInfo::New(*inputIter, 0), m_GDCMScanner.GetMapping(inputIter->c_str()) ) );
  }
  m_SortingResultInProgress.clear();
  m_SortingResultInProgress.push_back( m_InputFrameList );
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
    MITK_DEBUG << "DICOMITKSeriesGDCMReader: " << ss.str() << ": " << m_SortingResultInProgress.size() << " groups input";
    unsigned int groupIndex = 0;

    for(SortingBlockList::iterator blockIter = m_SortingResultInProgress.begin();
        blockIter != m_SortingResultInProgress.end();
        ++groupIndex, ++blockIter)
    {
      DICOMGDCMImageFrameList& gdcmInfoFrameList = *blockIter;
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

    m_SortingResultInProgress = nextStepSorting;
    timer.Stop( ss.str().c_str() );
  }
  timer.Stop("Sorting frames");

  timer.Start("Condensing 3D blocks (3D+t or vector values)");
  m_SortingResultInProgress = this->Condense3DBlocks( m_SortingResultInProgress );
  timer.Stop("Condensing 3D blocks (3D+t or vector values)");

  // provide final result as output

  timer.Start("Output");
  unsigned int o = this->GetNumberOfOutputs();
  this->SetNumberOfOutputs( o + m_SortingResultInProgress.size() ); // Condense3DBlocks may already have added outputs!
  for (SortingBlockList::iterator blockIter = m_SortingResultInProgress.begin();
       blockIter != m_SortingResultInProgress.end();
       ++o, ++blockIter)
  {
    DICOMGDCMImageFrameList& gdcmFrameInfoList = *blockIter;
    DICOMImageFrameList frameList = ToDICOMImageFrameList( gdcmFrameInfoList );
    assert(!gdcmFrameInfoList.empty());
    assert(!frameList.empty());

    DICOMImageBlockDescriptor block;
    block.SetTagCache( this ); // important: this must be before SetImageFrameList(), because SetImageFrameList will trigger reading of lots of interesting tags!
    block.SetImageFrameList( frameList );

    const GantryTiltInformation& tiltInfo = m_EquiDistantBlocksSorter->GetTiltInformation( (gdcmFrameInfoList.front())->GetFilenameIfAvailable() );
    block.SetFlag("gantryTilt", tiltInfo.IsRegularGantryTilt());
    block.SetTiltInformation( tiltInfo );

    static const DICOMTag tagPixelSpacing(0x0028,0x0030);
    static const DICOMTag tagImagerPixelSpacing(0x0018,0x1164);
    std::string pixelSpacingString = (gdcmFrameInfoList.front())->GetTagValueAsString( tagPixelSpacing );
    std::string imagerPixelSpacingString = gdcmFrameInfoList.front()->GetTagValueAsString( tagImagerPixelSpacing );
    block.SetPixelSpacingInformation(pixelSpacingString, imagerPixelSpacingString);

    static const DICOMTag tagSOPClassUID(0x0008,0x0016);
    std::string sopClassUID = (gdcmFrameInfoList.front())->GetTagValueAsString( tagSOPClassUID );
    block.SetSOPClassUID(sopClassUID);

    block.SetReaderImplementationLevel( this->GetReaderImplementationLevel(sopClassUID) );

    this->SetOutput( o, block );
  }
  timer.Stop("Output");

#ifdef MBILOG_ENABLE_DEBUG
  std::cout << "---------------------------------------------------------------" << std::endl;
  timer.Report( std::cout );
  std::cout << "---------------------------------------------------------------" << std::endl;
#endif
}

mitk::ReaderImplementationLevel
mitk::DICOMITKSeriesGDCMReader
::GetReaderImplementationLevel(const std::string uid) const
{
  if (uid.empty())
  {
    return SOPClassUnknown;
  }

  gdcm::UIDs uidKnowledge;
  uidKnowledge.SetFromUID( uid.c_str() );

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
::LoadMitkImageForOutput(unsigned int o)
{
  PushLocale();
  DICOMImageBlockDescriptor& block = this->InternalGetOutput(o);
  const DICOMImageFrameList& frames = block.GetImageFrameList();
  const GantryTiltInformation tiltInfo = block.GetTiltInformation();
  bool hasTilt = block.GetFlag("gantryTilt", false);
  if (hasTilt)
  {
    MITK_DEBUG << "When loading image " << o << ": got tilt info:";
    //tiltInfo.Print(std::cout);
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

  mitk::ITKDICOMSeriesReaderHelper helper;
  mitk::Image::Pointer mitkImage = helper.Load( filenames, m_FixTiltByShearing && hasTilt, tiltInfo ); // TODO preloaded images, caching..?

  block.SetMitkImage( mitkImage );
  PopLocale();

  return true;
}


bool
mitk::DICOMITKSeriesGDCMReader
::CanHandleFile(const std::string& itkNotUsed(filename))
{
  return true; // can handle all
  // peek into file, check DCM
  // nice-to-have: check multi-framedness
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

void
mitk::DICOMITKSeriesGDCMReader
::EnsureMandatorySortersArePresent()
{
  // TODO do just once!
  DICOMTagBasedSorter::Pointer splitter = DICOMTagBasedSorter::New();
  splitter->AddDistinguishingTag( DICOMTag(0x0028, 0x0010) ); // Number of Rows
  splitter->AddDistinguishingTag( DICOMTag(0x0028, 0x0011) ); // Number of Columns
  splitter->AddDistinguishingTag( DICOMTag(0x0028, 0x0030) ); // Pixel Spacing
  splitter->AddDistinguishingTag( DICOMTag(0x0018, 0x1164) ); // Imager Pixel Spacing
  splitter->AddDistinguishingTag( DICOMTag(0x0020, 0x0037), new mitk::DICOMTagBasedSorter::CutDecimalPlaces(5) ); // Image Orientation (Patient) // TODO: configurable!
  splitter->AddDistinguishingTag( DICOMTag(0x0018, 0x0050) ); // Slice Thickness
  splitter->AddDistinguishingTag( DICOMTag(0x0028, 0x0008) ); // Number of Frames
  this->AddSortingElement( splitter, true ); // true = at front

  if (m_EquiDistantBlocksSorter.IsNull())
  {
    m_EquiDistantBlocksSorter = mitk::EquiDistantBlocksSorter::New();
  }
  m_EquiDistantBlocksSorter->SetAcceptTilt( m_FixTiltByShearing );

  this->AddSortingElement( m_EquiDistantBlocksSorter );
}

std::string
mitk::DICOMITKSeriesGDCMReader
::GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const
{
  for(DICOMGDCMImageFrameList::const_iterator frameIter = m_InputFrameList.begin();
      frameIter != m_InputFrameList.end();
      ++frameIter)
  {
    if ( (*frameIter)->GetFrameInfo().IsNotNull() &&
         (*((*frameIter)->GetFrameInfo()) == *frame )
       )
    {
      return (*frameIter)->GetTagValueAsString(tag);
    }

  }

  return "";
}
