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
#include "mitkGantryTiltInformation.h"

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
::FlagGantryTilt()
{
  m_GantryTilt = true;
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
{
}

mitk::EquiDistantBlocksSorter
::~EquiDistantBlocksSorter()
{
}

mitk::EquiDistantBlocksSorter
::EquiDistantBlocksSorter(const EquiDistantBlocksSorter& other )
:DICOMDatasetSorter(other)
{
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
  tags.push_back( std::make_pair(0x0020, 0x0032) ); // ImagePositionPatient
  tags.push_back( std::make_pair(0x0020, 0x0037) ); // ImageOrientationPatient

  return tags;
}

void
mitk::EquiDistantBlocksSorter
::Sort()
{
  DICOMDatasetList remainingInput = GetInput(); // copy

  bool acceptGantryTilt = false; // TODO

  typedef std::list<DICOMDatasetList> OutputListType;
  OutputListType outputs;

  while (!remainingInput.empty()) // repeat until all files are grouped somehow
  {
    SliceGroupingAnalysisResult regularBlock = this->AnalyzeFileForITKImageSeriesReaderSpacingAssumption( remainingInput, acceptGantryTilt );

    outputs.push_back( regularBlock.GetBlockFilenames() );
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

std::string
mitk::EquiDistantBlocksSorter
::ConstCharStarToString(const char* s)
{
  return s ?  std::string(s) : std::string();
}

mitk::Point3D
mitk::EquiDistantBlocksSorter
::DICOMStringToPoint3D(const std::string& s, bool& successful)
{
  Point3D p;
  successful = true;

  std::istringstream originReader(s);
  std::string coordinate;
  unsigned int dim(0);
  while( std::getline( originReader, coordinate, '\\' ) && dim < 3)
  {
    p[dim++]= atof(coordinate.c_str());
  }

  if (dim && dim != 3)
  {
    successful = false;
    MITK_ERROR << "Reader implementation made wrong assumption on tag (0020,0032). Found " << dim << " instead of 3 values.";
  }
  else if (dim == 0)
  {
    successful = false;
    p.Fill(0.0); // assume default (0,0,0)
  }

  return p;
}


void
mitk::EquiDistantBlocksSorter
::DICOMStringToOrientationVectors(const std::string& s, Vector3D& right, Vector3D& up, bool& successful)
{
  successful = true;

  std::istringstream orientationReader(s);
  std::string coordinate;
  unsigned int dim(0);
  while( std::getline( orientationReader, coordinate, '\\' ) && dim < 6 )
  {
    if (dim<3)
    {
      right[dim++] = atof(coordinate.c_str());
    }
    else
    {
      up[dim++ - 3] = atof(coordinate.c_str());
    }
  }

  if (dim && dim != 6)
  {
    successful = false;
    MITK_ERROR << "Reader implementation made wrong assumption on tag (0020,0037). Found " << dim << " instead of 6 values.";
  }
  else if (dim == 0)
  {
    // fill with defaults
    right.Fill(0.0);
    right[0] = 1.0;

    up.Fill(0.0);
    up[1] = 1.0;

    successful = false;
  }
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
  const DICOMTag tagImagePositionPatient = std::make_pair(0x0020,0x0032); // Image Position (Patient)
  const DICOMTag    tagImageOrientation = std::make_pair(0x0020, 0x0037); // Image Orientation
  const DICOMTag          tagGantryTilt = std::make_pair(0x0018, 0x1120); // gantry tilt

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
  MITK_DEBUG << "Analyzing files for z-spacing assumption of ITK's ImageSeriesReader (group tilted: " << groupImagesWithGantryTilt << ")";
  unsigned int fileIndex(0);
  for (DICOMDatasetList::const_iterator fileIter = datasets.begin();
       fileIter != datasets.end();
       ++fileIter, ++fileIndex)
  {
    bool fileFitsIntoPattern(false);
    std::string thisOriginString;
    // Read tag value into point3D. PLEASE replace this by appropriate GDCM code if you figure out how to do that
    thisOriginString = (*fileIter)->GetTagValueAsString( tagImagePositionPatient );

    if (thisOriginString.empty())
    {
      // don't let such files be in a common group. Everything without position information will be loaded as a single slice:
      // with standard DICOM files this can happen to: CR, DX, SC
      MITK_DEBUG << "    ==> Sort away " << *fileIter << " for later analysis (no position information)"; // we already have one occupying this position

      if ( result.GetBlockFilenames().empty() ) // nothing WITH position information yet
      {
        // ==> this is a group of its own, stop processing, come back later
        result.AddFileToSortedBlock( *fileIter ); // TODO

        // TODO change to DICOMDatasetList!
        DICOMDatasetList remainingFiles;
        remainingFiles.insert( remainingFiles.end(), fileIter+1, datasets.end() );
        result.AddFilesToUnsortedBlock( remainingFiles );

        fileFitsIntoPattern = false;
        break; // no files anymore
      }
      else
      {
        // ==> this does not match, consider later
        result.AddFileToUnsortedBlock( *fileIter ); // sort away for further analysis
        fileFitsIntoPattern = false;
        continue; // next file
      }
    }

    bool ignoredConversionError(-42); // hard to get here, no graceful way to react
    thisOrigin = DICOMStringToPoint3D( thisOriginString, ignoredConversionError );

    MITK_DEBUG << "  " << fileIndex << " " << *fileIter
                       << " at "
                       /* << thisOriginString */ << "(" << thisOrigin[0] << "," << thisOrigin[1] << "," << thisOrigin[2] << ")";

    if ( lastOriginInitialized && (thisOrigin == lastOrigin) )
    {
      MITK_DEBUG << "    ==> Sort away " << *fileIter << " for separate time step"; // we already have one occupying this position
      result.AddFileToUnsortedBlock( *fileIter ); // sort away for further analysis
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
        std::string orientationValue = (*fileIter)->GetTagValueAsString( tagImageOrientation );
        DICOMStringToOrientationVectors( orientationValue, right, up, ignoredConversionError );

        GantryTiltInformation tiltInfo( lastDifferentOrigin, thisOrigin, right, up, 1 );

        if ( tiltInfo.IsSheared() ) // mitk::eps is too small; 1/1000 of a mm should be enough to detect tilt
        {
          /* optimistic approach, accepting gantry tilt: save file for later, check all further files */

          // at this point we have TWO slices analyzed! if they are the only two files, we still split, because there is no third to verify our tilting assumption.
          // later with a third being available, we must check if the initial tilting vector is still valid. if yes, continue.
          // if NO, we need to split the already sorted part (result.first) and the currently analyzed file (*fileIter)

          // tell apart gantry tilt from overall skewedness
          // sort out irregularly sheared slices, that IS NOT tilting

          if ( groupImagesWithGantryTilt && tiltInfo.IsRegularGantryTilt() )
          {
            bool todoIsDone(false); // TODO add GantryTilt reading
            // check if this is at least roughly the same angle as recorded in DICOM tags
            if ( todoIsDone /*&& tagValueMappings[fileIter->c_str()].find(tagGantryTilt) != tagValueMappings[fileIter->c_str()].end() */)
            {
              // read value, compare to calculated angle
              std::string tiltStr = (*fileIter)->GetTagValueAsString( tagGantryTilt );
              double angle = atof(tiltStr.c_str());

              MITK_DEBUG << "Comparing recorded tilt angle " << angle << " against calculated value " << tiltInfo.GetTiltAngleInDegrees();
              // TODO we probably want the signs correct, too (that depends: this is just a rough check, nothing serious)
              if ( fabs(angle) - tiltInfo.GetTiltAngleInDegrees() > 0.25)
              {
                result.AddFileToUnsortedBlock( *fileIter ); // sort away for further analysis
                fileFitsIntoPattern = false;
              }
              else  // tilt angle from header is less than 0.25 degrees different from what we calculated, assume this is fine
              {
                result.FlagGantryTilt();
                result.AddFileToSortedBlock( *fileIter ); // this file is good for current block
                fileFitsIntoPattern = true;
              }
            }
            else // we cannot check the calculated tilt angle against the one from the dicom header (so we assume we are right)
            {
              result.FlagGantryTilt();
              result.AddFileToSortedBlock( *fileIter ); // this file is good for current block
              fileFitsIntoPattern = true;
            }
          }
          else // caller does not want tilt compensation OR shearing is more complicated than tilt
          {
            result.AddFileToUnsortedBlock( *fileIter ); // sort away for further analysis
            fileFitsIntoPattern = false;
          }
        }
        else // not sheared
        {
          result.AddFileToSortedBlock( *fileIter ); // this file is good for current block
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
          MITK_DEBUG  << "    ==> Sort away " << *fileIter << " for later analysis";

          // At this point we know we deviated from the expectation of ITK's ImageSeriesReader
          // We split the input file list at this point, i.e. all files up to this one (excluding it)
          // are returned as group 1, the remaining files (including the faulty one) are group 2

          /* Optimistic approach: check if any of the remaining slices fits in */
          result.AddFileToUnsortedBlock( *fileIter ); // sort away for further analysis
          fileFitsIntoPattern = false;
        }
        else
        {
          result.AddFileToSortedBlock( *fileIter ); // this file is good for current block
          fileFitsIntoPattern = true;
        }
      }
      else // this should be the very first slice
      {
        result.AddFileToSortedBlock( *fileIter ); // this file is good for current block
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
