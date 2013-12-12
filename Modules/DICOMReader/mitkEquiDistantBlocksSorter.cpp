/*=================================================================== The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkEquiDistantBlocksSorter.h"

mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::SliceGroupingAnalysisResult()
:m_GantryTilt(false)
{
}

mitk::DICOMDatasetList
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::GetBlockFilenames()
{
  return m_GroupedFiles;
}

mitk::DICOMDatasetList
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::GetUnsortedFilenames()
{
  return m_UnsortedFiles;
}

bool
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::ContainsGantryTilt()
{
  return m_GantryTilt;
}

void
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::AddFileToSortedBlock(DICOMDatasetAccess* dataset)
{
  m_GroupedFiles.push_back( dataset );
}

void
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::AddFileToUnsortedBlock(DICOMDatasetAccess* dataset)
{
  m_UnsortedFiles.push_back( dataset );
}

void
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::AddFilesToUnsortedBlock(const DICOMDatasetList& datasets)
{
  m_UnsortedFiles.insert( m_UnsortedFiles.end(), datasets.begin(), datasets.end() );
}

void
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::SetFirstFilenameOfBlock(const std::string& filename)
{
  m_FirstFilenameOfBlock = filename;
}

std::string
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::GetFirstFilenameOfBlock() const
{
  return m_FirstFilenameOfBlock;
}

void
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::FlagGantryTilt(const GantryTiltInformation& tiltInfo)
{
  m_GantryTilt = true;
  m_TiltInfo = tiltInfo;
}

const mitk::GantryTiltInformation&
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::GetTiltInfo() const
{
  return m_TiltInfo;
}

void
mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
::UndoPrematureGrouping()
{
  assert( !m_GroupedFiles.empty() );
  m_UnsortedFiles.insert( m_UnsortedFiles.begin(), m_GroupedFiles.back() );
  m_GroupedFiles.pop_back();
}

// ------------------------ end helper class

mitk::EquiDistantBlocksSorter
::EquiDistantBlocksSorter()
:DICOMDatasetSorter()
,m_AcceptTilt(false)
{
}

mitk::EquiDistantBlocksSorter
::EquiDistantBlocksSorter(const EquiDistantBlocksSorter& other )
:DICOMDatasetSorter(other)
,m_AcceptTilt(false)
{
}

mitk::EquiDistantBlocksSorter
::~EquiDistantBlocksSorter()
{
}

void
mitk::EquiDistantBlocksSorter
::SetAcceptTilt(bool accept)
{
  m_AcceptTilt = accept;
}


mitk::EquiDistantBlocksSorter&
mitk::EquiDistantBlocksSorter
::operator=(const EquiDistantBlocksSorter& other)
{
  if (this != &other)
  {
    DICOMDatasetSorter::operator=(other);
  }
  return *this;
}

mitk::DICOMTagList
mitk::EquiDistantBlocksSorter
::GetTagsOfInterest()
{
  DICOMTagList tags;
  tags.push_back( DICOMTag(0x0020, 0x0032) ); // ImagePositionPatient
  tags.push_back( DICOMTag(0x0020, 0x0037) ); // ImageOrientationPatient
  tags.push_back( DICOMTag(0x0018, 0x1120) ); // GantryTilt

  return tags;
}

void
mitk::EquiDistantBlocksSorter
::Sort()
{
  DICOMDatasetList remainingInput = GetInput(); // copy

  typedef std::list<DICOMDatasetList> OutputListType;
  OutputListType outputs;

  m_SliceGroupingResults.clear();

  while (!remainingInput.empty()) // repeat until all files are grouped somehow
  {
    SliceGroupingAnalysisResult regularBlock = this->AnalyzeFileForITKImageSeriesReaderSpacingAssumption( remainingInput, m_AcceptTilt );

    DICOMDatasetList inBlock = regularBlock.GetBlockFilenames();
    DICOMDatasetList laterBlock = regularBlock.GetUnsortedFilenames();
    MITK_DEBUG << "Result: sorted 3D group with " << inBlock.size() << " files";
    for (DICOMDatasetList::const_iterator diter = inBlock.begin(); diter != inBlock.end(); ++diter)
      MITK_DEBUG << "  IN  " << (*diter)->GetFilenameIfAvailable();
    for (DICOMDatasetList::const_iterator diter = laterBlock.begin(); diter != laterBlock.end(); ++diter)
      MITK_DEBUG << " OUT  " << (*diter)->GetFilenameIfAvailable();

    outputs.push_back( regularBlock.GetBlockFilenames() );
    m_SliceGroupingResults.push_back( regularBlock );
    remainingInput = regularBlock.GetUnsortedFilenames();
  }

  unsigned int numberOfOutputs = outputs.size();
  this->SetNumberOfOutputs(numberOfOutputs);

  unsigned int outputIndex(0);
  for (OutputListType::iterator oIter = outputs.begin();
       oIter != outputs.end();
       ++outputIndex, ++oIter)
  {
    this->SetOutput(outputIndex, *oIter);
  }
}

const mitk::GantryTiltInformation
mitk::EquiDistantBlocksSorter
::GetTiltInformation(const std::string& filename, bool& hasTiltInfo)
{
  for (ResultsList::iterator ri = m_SliceGroupingResults.begin();
       ri != m_SliceGroupingResults.end();
       ++ri)
  {
    SliceGroupingAnalysisResult& result = *ri;

    if (filename == result.GetFirstFilenameOfBlock())
    {
      hasTiltInfo = result.ContainsGantryTilt(); // this is a returning statement, don't remove
      if (hasTiltInfo)
      {
        return result.GetTiltInfo();
      }
    }
  }

  return GantryTiltInformation(); // empty
}

std::string
mitk::EquiDistantBlocksSorter
::ConstCharStarToString(const char* s)
{
  return s ?  std::string(s) : std::string();
}

mitk::EquiDistantBlocksSorter::SliceGroupingAnalysisResult
mitk::EquiDistantBlocksSorter
::AnalyzeFileForITKImageSeriesReaderSpacingAssumption(
  const DICOMDatasetList& datasets,
  bool groupImagesWithGantryTilt)
{
  // result.first = files that fit ITK's assumption
  // result.second = files that do not fit, should be run through AnalyzeFileForITKImageSeriesReaderSpacingAssumption() again
  SliceGroupingAnalysisResult result;

  // we const_cast here, because I could not use a map.at(), which would make the code much more readable
  const DICOMTag tagImagePositionPatient = DICOMTag(0x0020,0x0032); // Image Position (Patient)
  const DICOMTag    tagImageOrientation = DICOMTag(0x0020, 0x0037); // Image Orientation
  const DICOMTag          tagGantryTilt = DICOMTag(0x0018, 0x1120); // gantry tilt

  Vector3D fromFirstToSecondOrigin; fromFirstToSecondOrigin.Fill(0.0);
  bool fromFirstToSecondOriginInitialized(false);
  Point3D thisOrigin;
  thisOrigin.Fill(0.0f);
  Point3D lastOrigin;
  lastOrigin.Fill(0.0f);
  Point3D lastDifferentOrigin;
  lastDifferentOrigin.Fill(0.0f);

  bool lastOriginInitialized(false);

  MITK_DEBUG << "--------------------------------------------------------------------------------";
  MITK_DEBUG << "Analyzing " << datasets.size() << " files for z-spacing assumption of ITK's ImageSeriesReader (group tilted: " << groupImagesWithGantryTilt << ")";
  unsigned int fileIndex(0);
  for (DICOMDatasetList::const_iterator dsIter = datasets.begin();
       dsIter != datasets.end();
       ++dsIter, ++fileIndex)
  {
    bool fileFitsIntoPattern(false);
    std::string thisOriginString;
    // Read tag value into point3D. PLEASE replace this by appropriate GDCM code if you figure out how to do that
    thisOriginString = (*dsIter)->GetTagValueAsString( tagImagePositionPatient );

    if (thisOriginString.empty())
    {
      // don't let such files be in a common group. Everything without position information will be loaded as a single slice:
      // with standard DICOM files this can happen to: CR, DX, SC
      MITK_DEBUG << "    ==> Sort away " << *dsIter << " for later analysis (no position information)"; // we already have one occupying this position

      if ( result.GetBlockFilenames().empty() ) // nothing WITH position information yet
      {
        // ==> this is a group of its own, stop processing, come back later
        result.AddFileToSortedBlock( *dsIter );

        DICOMDatasetList remainingFiles;
        remainingFiles.insert( remainingFiles.end(), dsIter+1, datasets.end() );
        result.AddFilesToUnsortedBlock( remainingFiles );

        fileFitsIntoPattern = false;
        break; // no files anymore
      }
      else
      {
        // ==> this does not match, consider later
        result.AddFileToUnsortedBlock( *dsIter ); // sort away for further analysis
        fileFitsIntoPattern = false;
        continue; // next file
      }
    }

    bool ignoredConversionError(-42); // hard to get here, no graceful way to react
    thisOrigin = DICOMStringToPoint3D( thisOriginString, ignoredConversionError );

    MITK_DEBUG << "  " << fileIndex << " " << (*dsIter)->GetFilenameIfAvailable()
                       << " at "
                       /* << thisOriginString */ << "(" << thisOrigin[0] << "," << thisOrigin[1] << "," << thisOrigin[2] << ")";

    if ( lastOriginInitialized && (thisOrigin == lastOrigin) )
    {
      MITK_DEBUG << "    ==> Sort away " << *dsIter << " for separate time step"; // we already have one occupying this position
      result.AddFileToUnsortedBlock( *dsIter ); // sort away for further analysis
      fileFitsIntoPattern = false;
    }
    else
    {
      if (!fromFirstToSecondOriginInitialized && lastOriginInitialized) // calculate vector as soon as possible when we get a new position
      {
        fromFirstToSecondOrigin = thisOrigin - lastDifferentOrigin;
        fromFirstToSecondOriginInitialized = true;

        // Here we calculate if this slice and the previous one are well aligned,
        // i.e. we test if the previous origin is on a line through the current
        // origin, directed into the normal direction of the current slice.

        // If this is NOT the case, then we have a data set with a TILTED GANTRY geometry,
        // which cannot be simply loaded into a single mitk::Image at the moment.
        // For this case, we flag this finding in the result and DicomSeriesReader
        // can correct for that later.

        Vector3D right; right.Fill(0.0);
        Vector3D up; right.Fill(0.0); // might be down as well, but it is just a name at this point
        std::string orientationValue = (*dsIter)->GetTagValueAsString( tagImageOrientation );
        DICOMStringToOrientationVectors( orientationValue, right, up, ignoredConversionError );

        GantryTiltInformation tiltInfo( lastDifferentOrigin, thisOrigin, right, up, 1 );

        if ( tiltInfo.IsSheared() ) // mitk::eps is too small; 1/1000 of a mm should be enough to detect tilt
        {
          /* optimistic approach, accepting gantry tilt: save file for later, check all further files */

          // at this point we have TWO slices analyzed! if they are the only two files, we still split, because there is no third to verify our tilting assumption.
          // later with a third being available, we must check if the initial tilting vector is still valid. if yes, continue.
          // if NO, we need to split the already sorted part (result.first) and the currently analyzed file (*dsIter)

          // tell apart gantry tilt from overall skewedness
          // sort out irregularly sheared slices, that IS NOT tilting

          if ( groupImagesWithGantryTilt && tiltInfo.IsRegularGantryTilt() )
          {
            // check if this is at least roughly the same angle as recorded in DICOM tags
            double angle = 0.0;
            std::string tiltStr = (*dsIter)->GetTagValueAsString( tagGantryTilt );
            const char* convertInput = tiltStr.c_str();
            char* convertEnd(NULL);
            if (!tiltStr.empty())
            {
              // read value, compare to calculated angle
              angle = strtod(convertInput, &convertEnd); // TODO check errors!
            }

            if (convertEnd != convertInput)
            {
              MITK_DEBUG << "Comparing recorded tilt angle " << angle << " against calculated value " << tiltInfo.GetTiltAngleInDegrees();
              // TODO we probably want the signs correct, too (that depends: this is just a rough check, nothing serious)
              if ( fabs(angle) - tiltInfo.GetTiltAngleInDegrees() > 0.25)
              {
                result.AddFileToUnsortedBlock( *dsIter ); // sort away for further analysis
                fileFitsIntoPattern = false;
              }
              else  // tilt angle from header is less than 0.25 degrees different from what we calculated, assume this is fine
              {
                assert(!datasets.empty());

                result.FlagGantryTilt(tiltInfo);
                result.AddFileToSortedBlock( *dsIter ); // this file is good for current block
                result.SetFirstFilenameOfBlock( datasets.front()->GetFilenameIfAvailable() );
                fileFitsIntoPattern = true;
              }
            }
            else // we cannot check the calculated tilt angle against the one from the dicom header (so we assume we are right)
            {
              assert(!datasets.empty());

              result.FlagGantryTilt(tiltInfo);
              result.AddFileToSortedBlock( *dsIter ); // this file is good for current block
              result.SetFirstFilenameOfBlock( datasets.front()->GetFilenameIfAvailable() );
              fileFitsIntoPattern = true;
            }
          }
          else // caller does not want tilt compensation OR shearing is more complicated than tilt
          {
            result.AddFileToUnsortedBlock( *dsIter ); // sort away for further analysis
            fileFitsIntoPattern = false;
          }
        }
        else // not sheared
        {
          result.AddFileToSortedBlock( *dsIter ); // this file is good for current block
          fileFitsIntoPattern = true;
        }
      }
      else if (fromFirstToSecondOriginInitialized) // we already know the offset between slices
      {
        Point3D assumedOrigin = lastDifferentOrigin + fromFirstToSecondOrigin;

        Vector3D originError = assumedOrigin - thisOrigin;
        double norm = originError.GetNorm();
        double toleratedError(0.005); // max. 1/10mm error when measurement crosses 20 slices in z direction

        if (norm > toleratedError)
        {
          MITK_DEBUG << "  File does not fit into the inter-slice distance pattern (diff = "
                               << norm << ", allowed "
                               << toleratedError << ").";
          MITK_DEBUG << "  Expected position (" << assumedOrigin[0] << ","
                                            << assumedOrigin[1] << ","
                                            << assumedOrigin[2] << "), got position ("
                                            << thisOrigin[0] << ","
                                            << thisOrigin[1] << ","
                                            << thisOrigin[2] << ")";
          MITK_DEBUG  << "    ==> Sort away " << *dsIter << " for later analysis";

          // At this point we know we deviated from the expectation of ITK's ImageSeriesReader
          // We split the input file list at this point, i.e. all files up to this one (excluding it)
          // are returned as group 1, the remaining files (including the faulty one) are group 2

          /* Optimistic approach: check if any of the remaining slices fits in */
          result.AddFileToUnsortedBlock( *dsIter ); // sort away for further analysis
          fileFitsIntoPattern = false;
        }
        else
        {
          result.AddFileToSortedBlock( *dsIter ); // this file is good for current block
          fileFitsIntoPattern = true;
        }
      }
      else // this should be the very first slice
      {
        result.AddFileToSortedBlock( *dsIter ); // this file is good for current block
        fileFitsIntoPattern = true;
      }
    }

    // record current origin for reference in later iterations
    if ( !lastOriginInitialized || ( fileFitsIntoPattern && (thisOrigin != lastOrigin) ) )
    {
      lastDifferentOrigin = thisOrigin;
    }

    lastOrigin = thisOrigin;
    lastOriginInitialized = true;
  }

  if ( result.ContainsGantryTilt() )
  {
    // check here how many files were grouped.
    // IF it was only two files AND we assume tiltedness (e.g. save "distance")
    // THEN we would want to also split the two previous files (simple) because
    // we don't have any reason to assume they belong together

    if ( result.GetBlockFilenames().size() == 2 )
    {
      result.UndoPrematureGrouping();
    }
  }

  return result;
}
