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

// uncomment for learning more about the internal sorting mechanisms
//#define MBILOG_ENABLE_DEBUG

#include <mitkDicomSeriesReader.h>
#include <mitkImage.h>
#include <mitkImageCast.h>

#include <itkGDCMSeriesFileNames.h>

#include <gdcmSorter.h>
#include <gdcmRAWCodec.h>
#include <gdcmAttribute.h>
#include <gdcmPixmapReader.h>
#include <gdcmStringFilter.h>
#include <gdcmDirectory.h>
#include <gdcmScanner.h>
#include <gdcmUIDs.h>

#include "mitkProperties.h"

namespace mitk
{
std::string DicomSeriesReader::ReaderImplementationLevelToString( const ReaderImplementationLevel& enumValue )
{
  switch (enumValue)
  {
    case ReaderImplementationLevel_Supported: return "Supported";
    case ReaderImplementationLevel_PartlySupported: return "PartlySupported";
    case ReaderImplementationLevel_Implemented: return "Implemented";
    case ReaderImplementationLevel_Unsupported: return "Unsupported";
    default: return "<unknown value of enum ReaderImplementationLevel>";
  };
}

std::string DicomSeriesReader::PixelSpacingInterpretationToString( const PixelSpacingInterpretation& enumValue )
{
  switch (enumValue)
  {
    case PixelSpacingInterpretation_SpacingInPatient: return "In Patient";
    case PixelSpacingInterpretation_SpacingAtDetector: return "At Detector";
    case PixelSpacingInterpretation_SpacingUnknown: return "Unknown spacing";
    default: return "<unknown value of enum PixelSpacingInterpretation>";
  };
}

const DicomSeriesReader::TagToPropertyMapType& DicomSeriesReader::GetDICOMTagsToMITKPropertyMap()
{
  static bool initialized = false;
  static TagToPropertyMapType dictionary;
  if (!initialized)
  {
    /*
       Selection criteria:
       - no sequences because we cannot represent that
       - nothing animal related (specied, breed registration number), MITK focusses on human medical image processing.
       - only general attributes so far

       When extending this, we should make use of a real dictionary (GDCM/DCMTK and lookup the tag names there)
    */

    // Patient module
    dictionary["0010|0010"] = "dicom.patient.PatientsName";
    dictionary["0010|0020"] = "dicom.patient.PatientID";
    dictionary["0010|0030"] = "dicom.patient.PatientsBirthDate";
    dictionary["0010|0040"] = "dicom.patient.PatientsSex";
    dictionary["0010|0032"] = "dicom.patient.PatientsBirthTime";
    dictionary["0010|1000"] = "dicom.patient.OtherPatientIDs";
    dictionary["0010|1001"] = "dicom.patient.OtherPatientNames";
    dictionary["0010|2160"] = "dicom.patient.EthnicGroup";
    dictionary["0010|4000"] = "dicom.patient.PatientComments";
    dictionary["0012|0062"] = "dicom.patient.PatientIdentityRemoved";
    dictionary["0012|0063"] = "dicom.patient.DeIdentificationMethod";

    // General Study module
    dictionary["0020|000d"] = "dicom.study.StudyInstanceUID";
    dictionary["0008|0020"] = "dicom.study.StudyDate";
    dictionary["0008|0030"] = "dicom.study.StudyTime";
    dictionary["0008|0090"] = "dicom.study.ReferringPhysiciansName";
    dictionary["0020|0010"] = "dicom.study.StudyID";
    dictionary["0008|0050"] = "dicom.study.AccessionNumber";
    dictionary["0008|1030"] = "dicom.study.StudyDescription";
    dictionary["0008|1048"] = "dicom.study.PhysiciansOfRecord";
    dictionary["0008|1060"] = "dicom.study.NameOfPhysicianReadingStudy";

    // General Series module
    dictionary["0008|0060"] = "dicom.series.Modality";
    dictionary["0020|000e"] = "dicom.series.SeriesInstanceUID";
    dictionary["0020|0011"] = "dicom.series.SeriesNumber";
    dictionary["0020|0060"] = "dicom.series.Laterality";
    dictionary["0008|0021"] = "dicom.series.SeriesDate";
    dictionary["0008|0031"] = "dicom.series.SeriesTime";
    dictionary["0008|1050"] = "dicom.series.PerformingPhysiciansName";
    dictionary["0018|1030"] = "dicom.series.ProtocolName";
    dictionary["0008|103e"] = "dicom.series.SeriesDescription";
    dictionary["0008|1070"] = "dicom.series.OperatorsName";
    dictionary["0018|0015"] = "dicom.series.BodyPartExamined";
    dictionary["0018|5100"] = "dicom.series.PatientPosition";
    dictionary["0028|0108"] = "dicom.series.SmallestPixelValueInSeries";
    dictionary["0028|0109"] = "dicom.series.LargestPixelValueInSeries";

    // VOI LUT module
    dictionary["0028|1050"] = "dicom.voilut.WindowCenter";
    dictionary["0028|1051"] = "dicom.voilut.WindowWidth";
    dictionary["0028|1055"] = "dicom.voilut.WindowCenterAndWidthExplanation";

    // Image Pixel module
    dictionary["0028|0004"] = "dicom.pixel.PhotometricInterpretation";
    dictionary["0028|0010"] = "dicom.pixel.Rows";
    dictionary["0028|0011"] = "dicom.pixel.Columns";

    // Image Plane module
    dictionary["0028|0030"] = "dicom.PixelSpacing";
    dictionary["0018|1164"] = "dicom.ImagerPixelSpacing";

    initialized = true;
  }

  return dictionary;
}


DataNode::Pointer
DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames, bool sort, bool check_4d, bool correctTilt, UpdateCallBackMethod callback, Image::Pointer preLoadedImageBlock)
{
  DataNode::Pointer node = DataNode::New();

  if (DicomSeriesReader::LoadDicomSeries(filenames, *node, sort, check_4d, correctTilt, callback, preLoadedImageBlock))
  {
    if( filenames.empty() )
    {
      return NULL;
    }

    return node;
  }
  else
  {
    return NULL;
  }
}

bool
DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames,
    DataNode &node,
    bool sort,
    bool check_4d,
    bool correctTilt,
    UpdateCallBackMethod callback,
    itk::SmartPointer<Image> preLoadedImageBlock)
{
  if( filenames.empty() )
  {
    MITK_DEBUG << "Calling LoadDicomSeries with empty filename string container. Probably invalid application logic.";
    node.SetData(NULL);
    return true; // this is not actually an error but the result is very simple
  }

  DcmIoType::Pointer io = DcmIoType::New();

  try
  {
    if (io->CanReadFile(filenames.front().c_str()))
    {
      io->SetFileName(filenames.front().c_str());
      io->ReadImageInformation();

      if (io->GetPixelType() == itk::ImageIOBase::SCALAR ||
          io->GetPixelType() == itk::ImageIOBase::RGB)
      {
        LoadDicom(filenames, node, sort, check_4d, correctTilt, callback, preLoadedImageBlock);
      }

      if (node.GetData())
      {
        return true;
      }
    }
  }
  catch(itk::MemoryAllocationError& e)
  {
    MITK_ERROR << "Out of memory. Cannot load DICOM series: " << e.what();
  }
  catch(std::exception& e)
  {
    MITK_ERROR << "Error encountered when loading DICOM series:" << e.what();
  }
  catch(...)
  {
    MITK_ERROR << "Unspecified error encountered when loading DICOM series.";
  }

  return false;
}


bool
DicomSeriesReader::IsDicom(const std::string &filename)
{
  DcmIoType::Pointer io = DcmIoType::New();

  return io->CanReadFile(filename.c_str());
}

bool
DicomSeriesReader::IsPhilips3DDicom(const std::string &filename)
{
  DcmIoType::Pointer io = DcmIoType::New();

  if (io->CanReadFile(filename.c_str()))
  {
    //Look at header Tag 3001,0010 if it is "Philips3D"
    gdcm::Reader reader;
    reader.SetFileName(filename.c_str());
    reader.Read();
    gdcm::DataSet &data_set = reader.GetFile().GetDataSet();
    gdcm::StringFilter sf;
    sf.SetFile(reader.GetFile());

    if (data_set.FindDataElement(gdcm::Tag(0x3001, 0x0010)) &&
      (sf.ToString(gdcm::Tag(0x3001, 0x0010)) == "Philips3D "))
    {
      return true;
    }
  }
  return false;
}

bool
DicomSeriesReader::ReadPhilips3DDicom(const std::string &filename, itk::SmartPointer<Image> output_image)
{
  // Now get PhilipsSpecific Tags

  gdcm::PixmapReader reader;
  reader.SetFileName(filename.c_str());
  reader.Read();
  gdcm::DataSet &data_set = reader.GetFile().GetDataSet();
  gdcm::StringFilter sf;
  sf.SetFile(reader.GetFile());

  gdcm::Attribute<0x0028,0x0011> dimTagX; // coloumns || sagittal
  gdcm::Attribute<0x3001,0x1001, gdcm::VR::UL, gdcm::VM::VM1> dimTagZ; //I have no idea what is VM1. // (Philips specific) // axial
  gdcm::Attribute<0x0028,0x0010> dimTagY; // rows || coronal
  gdcm::Attribute<0x0028,0x0008> dimTagT; // how many frames
  gdcm::Attribute<0x0018,0x602c> spaceTagX; // Spacing in X , unit is "physicalTagx" (usually centimeter)
  gdcm::Attribute<0x0018,0x602e> spaceTagY;
  gdcm::Attribute<0x3001,0x1003, gdcm::VR::FD, gdcm::VM::VM1> spaceTagZ; // (Philips specific)
  gdcm::Attribute<0x0018,0x6024> physicalTagX; // if 3, then spacing params are centimeter
  gdcm::Attribute<0x0018,0x6026> physicalTagY;
  gdcm::Attribute<0x3001,0x1002, gdcm::VR::US, gdcm::VM::VM1> physicalTagZ; // (Philips specific)

  dimTagX.Set(data_set);
  dimTagY.Set(data_set);
  dimTagZ.Set(data_set);
  dimTagT.Set(data_set);
  spaceTagX.Set(data_set);
  spaceTagY.Set(data_set);
  spaceTagZ.Set(data_set);
  physicalTagX.Set(data_set);
  physicalTagY.Set(data_set);
  physicalTagZ.Set(data_set);

  unsigned int
    dimX = dimTagX.GetValue(),
    dimY = dimTagY.GetValue(),
    dimZ = dimTagZ.GetValue(),
    dimT = dimTagT.GetValue(),
    physicalX = physicalTagX.GetValue(),
    physicalY = physicalTagY.GetValue(),
    physicalZ = physicalTagZ.GetValue();

  float
    spaceX = spaceTagX.GetValue(),
    spaceY = spaceTagY.GetValue(),
    spaceZ = spaceTagZ.GetValue();

  if (physicalX == 3) // spacing parameter in cm, have to convert it to mm.
    spaceX = spaceX * 10;

  if (physicalY == 3) // spacing parameter in cm, have to convert it to mm.
    spaceY = spaceY * 10;

  if (physicalZ == 3) // spacing parameter in cm, have to convert it to mm.
    spaceZ = spaceZ * 10;

  // Ok, got all necessary Tags!
  // Now read Pixeldata (7fe0,0010) X x Y x Z x T Elements

  const gdcm::Pixmap &pixels = reader.GetPixmap();
  gdcm::RAWCodec codec;

  codec.SetPhotometricInterpretation(gdcm::PhotometricInterpretation::MONOCHROME2);
  codec.SetPixelFormat(pixels.GetPixelFormat());
  codec.SetPlanarConfiguration(0);

  gdcm::DataElement out;
  codec.Decode(data_set.GetDataElement(gdcm::Tag(0x7fe0, 0x0010)), out);

  const gdcm::ByteValue *bv = out.GetByteValue();
  const char *new_pixels = bv->GetPointer();

  // Create MITK Image + Geometry
  typedef itk::Image<unsigned char, 4> ImageType;   //Pixeltype might be different sometimes? Maybe read it out from header
  ImageType::RegionType myRegion;
  ImageType::SizeType mySize;
  ImageType::IndexType myIndex;
  ImageType::SpacingType mySpacing;
  ImageType::Pointer imageItk = ImageType::New();

  mySpacing[0] = spaceX;
  mySpacing[1] = spaceY;
  mySpacing[2] = spaceZ;
  mySpacing[3] = 1;
  myIndex[0] = 0;
  myIndex[1] = 0;
  myIndex[2] = 0;
  myIndex[3] = 0;
  mySize[0] = dimX;
  mySize[1] = dimY;
  mySize[2] = dimZ;
  mySize[3] = dimT;
  myRegion.SetSize( mySize);
  myRegion.SetIndex( myIndex );
  imageItk->SetSpacing(mySpacing);
  imageItk->SetRegions( myRegion);
  imageItk->Allocate();
  imageItk->FillBuffer(0);

  itk::ImageRegionIterator<ImageType>  iterator(imageItk, imageItk->GetLargestPossibleRegion());
  iterator.GoToBegin();
  unsigned long pixCount = 0;
  unsigned long planeSize = dimX*dimY;
  unsigned long planeCount = 0;
  unsigned long timeCount = 0;
  unsigned long numberOfSlices = dimZ;

  while (!iterator.IsAtEnd())
  {
    unsigned long adressedPixel =
      pixCount
      + (numberOfSlices-1-planeCount)*planeSize // add offset to adress the first pixel of current plane
      + timeCount*numberOfSlices*planeSize; // add time offset

    iterator.Set( new_pixels[ adressedPixel ] );
    pixCount++;
    ++iterator;

    if (pixCount == planeSize)
    {
      pixCount = 0;
      planeCount++;
    }
    if (planeCount == numberOfSlices)
    {
      planeCount = 0;
      timeCount++;
    }
    if (timeCount == dimT)
    {
      break;
    }
  }
  mitk::CastToMitkImage(imageItk, output_image);
  return true; // actually never returns false yet.. but exception possible
}

std::string
DicomSeriesReader::ConstCharStarToString(const char* s)
{
  return s ?  std::string(s) : std::string();
}

bool
DicomSeriesReader::DICOMStringToSpacing(const std::string& s, ScalarType& spacingX, ScalarType& spacingY)
{
  bool successful = false;

  std::istringstream spacingReader(s);
  std::string spacing;
  if ( std::getline( spacingReader, spacing, '\\' ) )
  {
    spacingY = atof( spacing.c_str() );

    if ( std::getline( spacingReader, spacing, '\\' ) )
    {
      spacingX = atof( spacing.c_str() );

      successful = true;
    }
  }

  return successful;
}

Point3D
DicomSeriesReader::DICOMStringToPoint3D(const std::string& s, bool& successful)
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
DicomSeriesReader::DICOMStringToOrientationVectors(const std::string& s, Vector3D& right, Vector3D& up, bool& successful)
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


DicomSeriesReader::SliceGroupingAnalysisResult
DicomSeriesReader::AnalyzeFileForITKImageSeriesReaderSpacingAssumption(
    const StringContainer& files,
    bool groupImagesWithGantryTilt,
    const gdcm::Scanner::MappingType& tagValueMappings_)
{
  // result.first = files that fit ITK's assumption
  // result.second = files that do not fit, should be run through AnalyzeFileForITKImageSeriesReaderSpacingAssumption() again
  SliceGroupingAnalysisResult result;

  // we const_cast here, because I could not use a map.at(), which would make the code much more readable
  gdcm::Scanner::MappingType& tagValueMappings = const_cast<gdcm::Scanner::MappingType&>(tagValueMappings_);
  const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); // Image Position (Patient)
  const gdcm::Tag    tagImageOrientation(0x0020, 0x0037); // Image Orientation
  const gdcm::Tag          tagGantryTilt(0x0018, 0x1120); // gantry tilt

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
  for (StringContainer::const_iterator fileIter = files.begin();
       fileIter != files.end();
       ++fileIter, ++fileIndex)
  {
    bool fileFitsIntoPattern(false);
    std::string thisOriginString;
    // Read tag value into point3D. PLEASE replace this by appropriate GDCM code if you figure out how to do that
    thisOriginString = ConstCharStarToString( tagValueMappings[fileIter->c_str()][tagImagePositionPatient] );

    if (thisOriginString.empty())
    {
      // don't let such files be in a common group. Everything without position information will be loaded as a single slice:
      // with standard DICOM files this can happen to: CR, DX, SC
      MITK_DEBUG << "    ==> Sort away " << *fileIter << " for later analysis (no position information)"; // we already have one occupying this position

      if ( result.GetBlockFilenames().empty() ) // nothing WITH position information yet
      {
        // ==> this is a group of its own, stop processing, come back later
        result.AddFileToSortedBlock( *fileIter );

        StringContainer remainingFiles;
        remainingFiles.insert( remainingFiles.end(), fileIter+1, files.end() );
        result.AddFilesToUnsortedBlock( remainingFiles );

        fileFitsIntoPattern = false;
        break; // no files anymore
      }
      else
      {
        // ==> this does not match, consider later
        result.AddFileToUnsortedBlock( *fileIter );
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
      result.AddFileToUnsortedBlock( *fileIter );
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
        DICOMStringToOrientationVectors( tagValueMappings[fileIter->c_str()][tagImageOrientation], right, up, ignoredConversionError );

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
            // check if this is at least roughly the same angle as recorded in DICOM tags
            if ( tagValueMappings[fileIter->c_str()].find(tagGantryTilt) != tagValueMappings[fileIter->c_str()].end() )
            {
              // read value, compare to calculated angle
              std::string tiltStr = ConstCharStarToString( tagValueMappings[fileIter->c_str()][tagGantryTilt] );
              double angle = atof(tiltStr.c_str());

              MITK_DEBUG << "Comparing recorded tilt angle " << angle << " against calculated value " << tiltInfo.GetTiltAngleInDegrees();
              // TODO we probably want the signs correct, too (that depends: this is just a rough check, nothing serious)
              // TODO TODO TODO when angle -27 and tiltangle 63, this will never trigger the if-clause... useless check in this case! old bug..?!
              if ( fabs(angle) - tiltInfo.GetTiltAngleInDegrees() > 0.25)
              {
                result.AddFileToUnsortedBlock( *fileIter ); // sort away for further analysis
                fileFitsIntoPattern = false;
              }
              else  // tilt angle from header is less than 0.25 degrees different from what we calculated, assume this is fine
              {
                result.FlagGantryTilt();
                result.AddFileToSortedBlock(*fileIter); // this file is good for current block
                fileFitsIntoPattern = true;
              }
            }
            else // we cannot check the calculated tilt angle against the one from the dicom header (so we assume we are right)
            {
              result.FlagGantryTilt();
              result.AddFileToSortedBlock(*fileIter); // this file is good for current block
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
          result.AddFileToSortedBlock(*fileIter); // this file is good for current block
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
          result.AddFileToSortedBlock(*fileIter); // this file is good for current block
          fileFitsIntoPattern = true;
        }
      }
      else // this should be the very first slice
      {
        result.AddFileToSortedBlock(*fileIter); // this file is good for current block
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

DicomSeriesReader::FileNamesGrouping
DicomSeriesReader::GetSeries(const StringContainer& files, bool groupImagesWithGantryTilt, const StringContainer &restrictions)
{
  return GetSeries(files, true, groupImagesWithGantryTilt, restrictions);
}

DicomSeriesReader::FileNamesGrouping
DicomSeriesReader::GetSeries(const StringContainer& files, bool sortTo3DPlust, bool groupImagesWithGantryTilt, const StringContainer& /*restrictions*/)
{
  /**
    assumption about this method:
      returns a map of uid-like-key --> list(filename)
      each entry should contain filenames that have images of same
        - series instance uid (automatically done by GDCMSeriesFileNames
        - 0020,0037 image orientation (patient)
        - 0028,0030 pixel spacing (x,y)
        - 0018,0050 slice thickness
  */

  // use GDCM directly, itk::GDCMSeriesFileNames does not work with GDCM 2

  // PART I: scan files for sorting relevant DICOM tags,
  //         separate images that differ in any of those
  //         attributes (they cannot possibly form a 3D block)

  // scan for relevant tags in dicom files
  gdcm::Scanner scanner;
  const gdcm::Tag tagSOPClassUID(0x0008, 0x0016); // SOP class UID
    scanner.AddTag( tagSOPClassUID );

  const gdcm::Tag tagSeriesInstanceUID(0x0020,0x000e); // Series Instance UID
    scanner.AddTag( tagSeriesInstanceUID );

  const gdcm::Tag tagImageOrientation(0x0020, 0x0037); // image orientation
    scanner.AddTag( tagImageOrientation );

  const gdcm::Tag tagPixelSpacing(0x0028, 0x0030); // pixel spacing
    scanner.AddTag( tagPixelSpacing );

  const gdcm::Tag tagImagerPixelSpacing(0x0018, 0x1164); // imager pixel spacing
    scanner.AddTag( tagImagerPixelSpacing );

  const gdcm::Tag tagSliceThickness(0x0018, 0x0050); // slice thickness
    scanner.AddTag( tagSliceThickness );

  const gdcm::Tag tagNumberOfRows(0x0028, 0x0010); // number rows
    scanner.AddTag( tagNumberOfRows );

  const gdcm::Tag tagNumberOfColumns(0x0028, 0x0011); // number cols
    scanner.AddTag( tagNumberOfColumns );

  const gdcm::Tag tagGantryTilt(0x0018, 0x1120); // gantry tilt
    scanner.AddTag( tagGantryTilt );

  const gdcm::Tag tagModality(0x0008, 0x0060); // modality
    scanner.AddTag( tagModality );

  const gdcm::Tag tagNumberOfFrames(0x0028, 0x0008); // number of frames
    scanner.AddTag( tagNumberOfFrames );

  // additional tags read in this scan to allow later analysis
  // THESE tag are not used for initial separating of files
  const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); // Image Position (Patient)
    scanner.AddTag( tagImagePositionPatient );

  // TODO add further restrictions from arguments (when anybody asks for it)

  FileNamesGrouping result;

  // let GDCM scan files
  if ( !scanner.Scan( files ) )
  {
    MITK_ERROR << "gdcm::Scanner failed when scanning " << files.size() << " input files.";
    return result;
  }

  // assign files IDs that will separate them for loading into image blocks
  for (gdcm::Scanner::ConstIterator fileIter = scanner.Begin();
       fileIter != scanner.End();
       ++fileIter)
  {
    if ( std::string(fileIter->first).empty() ) continue; // TODO understand why Scanner has empty string entries
    if ( std::string(fileIter->first) == std::string("DICOMDIR") ) continue;

    /* sort out multi-frame
    if ( scanner.GetValue( fileIter->first , tagNumberOfFrames ) )
    {
      MITK_INFO << "Ignoring " << fileIter->first << " because we cannot handle multi-frame images.";
      continue;
    }
    */

    // we const_cast here, because I could not use a map.at() function in CreateMoreUniqueSeriesIdentifier.
    // doing the same thing with find would make the code less readable. Since we forget the Scanner results
    // anyway after this function, we can simply tolerate empty map entries introduced by bad operator[] access
    std::string moreUniqueSeriesId = CreateMoreUniqueSeriesIdentifier( const_cast<gdcm::Scanner::TagToValue&>(fileIter->second) );
    result[ moreUniqueSeriesId ].AddFile( fileIter->first );
  }

  // PART II: sort slices spatially (or at least consistently if this is NOT possible, see method)

  for ( FileNamesGrouping::const_iterator groupIter = result.begin();
        groupIter != result.end();
        ++groupIter )
  {
    try
    {
      result[ groupIter->first ] = ImageBlockDescriptor( SortSeriesSlices( groupIter->second.GetFilenames() ) ); // sort each slice group spatially
    } catch(...)
    {
       MITK_ERROR << "Caught something.";
    }
  }

  // PART III: analyze pre-sorted images for valid blocks (i.e. blocks of equal z-spacing),
  //          separate into multiple blocks if necessary.
  //
  //          Analysis performs the following steps:
  //            * imitate itk::ImageSeriesReader: use the distance between the first two images as z-spacing
  //            * check what images actually fulfill ITK's z-spacing assumption
  //            * separate all images that fail the test into new blocks, re-iterate analysis for these blocks
  //              * this includes images which DO NOT PROVIDE spatial information, i.e. all images w/o ImagePositionPatient will be loaded separately

  FileNamesGrouping groupsOf3DPlusTBlocks; // final result of this function
  for ( FileNamesGrouping::const_iterator groupIter = result.begin();
        groupIter != result.end();
        ++groupIter )
  {
    FileNamesGrouping groupsOf3DBlocks; // intermediate result for only this group(!)
    std::map<std::string, SliceGroupingAnalysisResult> mapOf3DBlockAnalysisResults;
    StringContainer filesStillToAnalyze = groupIter->second.GetFilenames();
    std::string groupUID = groupIter->first;
    unsigned int subgroup(0);
    MITK_DEBUG << "Analyze group " << groupUID << " of " << groupIter->second.GetFilenames().size() << " files";

    while (!filesStillToAnalyze.empty()) // repeat until all files are grouped somehow
    {
      SliceGroupingAnalysisResult analysisResult =
        AnalyzeFileForITKImageSeriesReaderSpacingAssumption( filesStillToAnalyze,
                                                             groupImagesWithGantryTilt,
                                                             scanner.GetMappings() );

      // enhance the UID for additional groups
      std::stringstream newGroupUID;
      newGroupUID << groupUID << '.' << subgroup;

      ImageBlockDescriptor thisBlock( analysisResult.GetBlockFilenames() );

      std::string firstFileInBlock = thisBlock.GetFilenames().front();

      thisBlock.SetImageBlockUID( newGroupUID.str() );
      thisBlock.SetSeriesInstanceUID( DicomSeriesReader::ConstCharStarToString( scanner.GetValue( firstFileInBlock.c_str(), tagSeriesInstanceUID ) ) );
      thisBlock.SetHasGantryTiltCorrected( analysisResult.ContainsGantryTilt() );
      thisBlock.SetSOPClassUID( DicomSeriesReader::ConstCharStarToString( scanner.GetValue( firstFileInBlock.c_str(), tagSOPClassUID ) ) );
      thisBlock.SetNumberOfFrames( ConstCharStarToString( scanner.GetValue( firstFileInBlock.c_str(), tagNumberOfFrames ) ) );
      thisBlock.SetModality( DicomSeriesReader::ConstCharStarToString( scanner.GetValue( firstFileInBlock.c_str(), tagModality ) ) );
      thisBlock.SetPixelSpacingInformation( DicomSeriesReader::ConstCharStarToString( scanner.GetValue( firstFileInBlock.c_str(), tagPixelSpacing ) ),
                                            DicomSeriesReader::ConstCharStarToString( scanner.GetValue( firstFileInBlock.c_str(), tagImagerPixelSpacing ) ) );
      thisBlock.SetHasMultipleTimePoints( false );

      groupsOf3DBlocks[ newGroupUID.str() ] = thisBlock;

      //MITK_DEBUG << "Result: sorted 3D group " << newGroupUID.str() << " with " << groupsOf3DBlocks[ newGroupUID.str() ].GetFilenames().size() << " files";
      MITK_DEBUG << "Result: sorted 3D group with " << groupsOf3DBlocks[ newGroupUID.str() ].GetFilenames().size() << " files";
      StringContainer debugOutputFiles = analysisResult.GetBlockFilenames();
      for (StringContainer::const_iterator siter = debugOutputFiles.begin(); siter != debugOutputFiles.end(); ++siter)
        MITK_DEBUG << "  IN  " << *siter;

      ++subgroup;

      filesStillToAnalyze = analysisResult.GetUnsortedFilenames(); // remember what needs further analysis
      for (StringContainer::const_iterator siter = filesStillToAnalyze.begin(); siter != filesStillToAnalyze.end(); ++siter)
        MITK_DEBUG << " OUT  " << *siter;
    }

    // end of grouping, now post-process groups

    // PART IV: attempt to group blocks to 3D+t blocks if requested
    //           inspect entries of groupsOf3DBlocks
    //            - if number of files is identical to previous entry, collect for 3D+t block
    //            - as soon as number of files changes from previous entry, record collected blocks as 3D+t block, start a new one, continue

    // decide whether or not to group 3D blocks into 3D+t blocks where possible
    if ( !sortTo3DPlust )
    {
      // copy 3D blocks to output
      groupsOf3DPlusTBlocks.insert( groupsOf3DBlocks.begin(), groupsOf3DBlocks.end() );
    }
    else
    {
      // sort 3D+t (as described in "PART IV")

      MITK_DEBUG << "================================================================================";
      MITK_DEBUG << "3D+t analysis:";

      unsigned int numberOfFilesInPreviousBlock(0);
      std::string previousBlockKey;

      for ( FileNamesGrouping::const_iterator block3DIter = groupsOf3DBlocks.begin();
            block3DIter != groupsOf3DBlocks.end();
            ++block3DIter )
      {
        unsigned int numberOfFilesInThisBlock = block3DIter->second.GetFilenames().size();
        std::string thisBlockKey = block3DIter->first;

        if (numberOfFilesInPreviousBlock == 0)
        {
          numberOfFilesInPreviousBlock = numberOfFilesInThisBlock;
          groupsOf3DPlusTBlocks[thisBlockKey] = block3DIter->second;
          MITK_DEBUG << "  3D+t group " << thisBlockKey;
          previousBlockKey = thisBlockKey;
        }
        else
        {
          bool identicalOrigins;
          try {
            // check whether this and the previous block share a comon origin
            // TODO should be safe, but a little try/catch or other error handling wouldn't hurt

            const char
                *origin_value = scanner.GetValue( groupsOf3DBlocks[thisBlockKey].GetFilenames().front().c_str(), tagImagePositionPatient ),
                *previous_origin_value = scanner.GetValue( groupsOf3DBlocks[previousBlockKey].GetFilenames().front().c_str(), tagImagePositionPatient ),
                *destination_value = scanner.GetValue( groupsOf3DBlocks[thisBlockKey].GetFilenames().back().c_str(), tagImagePositionPatient ),
                *previous_destination_value = scanner.GetValue( groupsOf3DBlocks[previousBlockKey].GetFilenames().back().c_str(), tagImagePositionPatient );

            if (!origin_value || !previous_origin_value || !destination_value || !previous_destination_value)
            {
              identicalOrigins = false;
            }
            else
            {
              std::string thisOriginString = ConstCharStarToString( origin_value );
              std::string previousOriginString = ConstCharStarToString( previous_origin_value );

              // also compare last origin, because this might differ if z-spacing is different
              std::string thisDestinationString = ConstCharStarToString( destination_value );
              std::string previousDestinationString = ConstCharStarToString( previous_destination_value );

              identicalOrigins =  ( (thisOriginString == previousOriginString) && (thisDestinationString == previousDestinationString) );
            }
          } catch(...)
          {
            identicalOrigins = false;
          }

          if (identicalOrigins && (numberOfFilesInPreviousBlock == numberOfFilesInThisBlock))
          {
            // group with previous block
            groupsOf3DPlusTBlocks[previousBlockKey].AddFiles( block3DIter->second.GetFilenames() );
            groupsOf3DPlusTBlocks[previousBlockKey].SetHasMultipleTimePoints(true);
            MITK_DEBUG << "  --> group enhanced with another timestep";
          }
          else
          {
            // start a new block
            groupsOf3DPlusTBlocks[thisBlockKey] = block3DIter->second;
            int numberOfTimeSteps = groupsOf3DPlusTBlocks[previousBlockKey].GetFilenames().size() / numberOfFilesInPreviousBlock;
            MITK_DEBUG << "  ==> group closed with " << numberOfTimeSteps << " time steps";
            previousBlockKey = thisBlockKey;
            MITK_DEBUG << "  3D+t group " << thisBlockKey << " started";
          }
        }

        numberOfFilesInPreviousBlock = numberOfFilesInThisBlock;
      }
    }
  }

  MITK_DEBUG << "================================================================================";
  MITK_DEBUG << "Summary: ";
  for ( FileNamesGrouping::const_iterator groupIter = groupsOf3DPlusTBlocks.begin(); groupIter != groupsOf3DPlusTBlocks.end(); ++groupIter )
  {
    ImageBlockDescriptor block = groupIter->second;
    MITK_DEBUG << "  " << block.GetFilenames().size() << " '" << block.GetModality() << "' images (" << block.GetSOPClassUIDAsString() << ") in volume " << block.GetImageBlockUID();
    MITK_DEBUG << "    (gantry tilt : " << (block.HasGantryTiltCorrected()?"Yes":"No") << "; "
                       "pixel spacing : " << PixelSpacingInterpretationToString( block.GetPixelSpacingType() ) << "; "
                       "3D+t: " << (block.HasMultipleTimePoints()?"Yes":"No") << "; "
                       "reader support: " << ReaderImplementationLevelToString( block.GetReaderImplementationLevel() ) << ")";
    StringContainer debugOutputFiles = block.GetFilenames();
    for (StringContainer::const_iterator siter = debugOutputFiles.begin(); siter != debugOutputFiles.end(); ++siter)
      MITK_DEBUG << "  F " << *siter;
  }
  MITK_DEBUG << "================================================================================";

  return groupsOf3DPlusTBlocks;
}

DicomSeriesReader::FileNamesGrouping
DicomSeriesReader::GetSeries(const std::string &dir, bool groupImagesWithGantryTilt, const StringContainer &restrictions)
{
  gdcm::Directory directoryLister;
  directoryLister.Load( dir.c_str(), false ); // non-recursive
  return GetSeries(directoryLister.GetFilenames(), groupImagesWithGantryTilt, restrictions);
}

std::string
DicomSeriesReader::CreateSeriesIdentifierPart( gdcm::Scanner::TagToValue& tagValueMap, const gdcm::Tag& tag )
{
  std::string result;
  try
  {
    result = IDifyTagValue( tagValueMap[ tag ] ? tagValueMap[ tag ] : std::string("") );
  }
  catch (std::exception&)
  {
    // we are happy with even nothing, this will just group images of a series
    //MITK_WARN << "Could not access tag " << tag << ": " << e.what();
  }

  return result;
}

std::string
DicomSeriesReader::CreateMoreUniqueSeriesIdentifier( gdcm::Scanner::TagToValue& tagValueMap )
{
  const gdcm::Tag tagSeriesInstanceUID(0x0020,0x000e); // Series Instance UID
  const gdcm::Tag tagImageOrientation(0x0020, 0x0037); // image orientation
  const gdcm::Tag tagPixelSpacing(0x0028, 0x0030); // pixel spacing
  const gdcm::Tag tagImagerPixelSpacing(0x0018, 0x1164); // imager pixel spacing
  const gdcm::Tag tagSliceThickness(0x0018, 0x0050); // slice thickness
  const gdcm::Tag tagNumberOfRows(0x0028, 0x0010); // number rows
  const gdcm::Tag tagNumberOfColumns(0x0028, 0x0011); // number cols
  const gdcm::Tag tagNumberOfFrames(0x0028, 0x0008); // number of frames

  const char* tagSeriesInstanceUid = tagValueMap[tagSeriesInstanceUID];
  if (!tagSeriesInstanceUid)
  {
    mitkThrow() << "CreateMoreUniqueSeriesIdentifier() could not access series instance UID. Something is seriously wrong with this image, so stopping here.";
  }
  std::string constructedID = tagSeriesInstanceUid;

  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagNumberOfRows );
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagNumberOfColumns );
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagPixelSpacing );
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagImagerPixelSpacing );
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagSliceThickness );
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagNumberOfFrames );

  // be a bit tolerant for orienatation, let only the first few digits matter (http://bugs.mitk.org/show_bug.cgi?id=12263)
  // NOT constructedID += CreateSeriesIdentifierPart( tagValueMap, tagImageOrientation );
  if (tagValueMap.find(tagImageOrientation) != tagValueMap.end())
  {
    bool conversionError(false);
    Vector3D right; right.Fill(0.0);
    Vector3D up; right.Fill(0.0);
    DICOMStringToOrientationVectors( tagValueMap[tagImageOrientation], right, up, conversionError );
    //string  newstring sprintf(simplifiedOrientationString, "%.3f\\%.3f\\%.3f\\%.3f\\%.3f\\%.3f", right[0], right[1], right[2], up[0], up[1], up[2]);
    std::ostringstream ss;
    ss.setf(std::ios::fixed, std::ios::floatfield);
    ss.precision(5);
    ss << right[0] << "\\"
       << right[1] << "\\"
       << right[2] << "\\"
       << up[0] << "\\"
       << up[1] << "\\"
       << up[2];
    std::string simplifiedOrientationString(ss.str());

    constructedID += IDifyTagValue( simplifiedOrientationString );
  }

  constructedID.resize( constructedID.length() - 1 ); // cut of trailing '.'

  return constructedID;
}

std::string
DicomSeriesReader::IDifyTagValue(const std::string& value)
{
  std::string IDifiedValue( value );
  if (value.empty()) throw std::logic_error("IDifyTagValue() illegaly called with empty tag value");

  // Eliminate non-alnum characters, including whitespace...
  //   that may have been introduced by concats.
  for(std::size_t i=0; i<IDifiedValue.size(); i++)
  {
    while(i<IDifiedValue.size()
      && !( IDifiedValue[i] == '.'
        || (IDifiedValue[i] >= 'a' && IDifiedValue[i] <= 'z')
        || (IDifiedValue[i] >= '0' && IDifiedValue[i] <= '9')
        || (IDifiedValue[i] >= 'A' && IDifiedValue[i] <= 'Z')))
    {
      IDifiedValue.erase(i, 1);
    }
  }


  IDifiedValue += ".";
  return IDifiedValue;
}

DicomSeriesReader::StringContainer
DicomSeriesReader::GetSeries(const std::string &dir, const std::string &series_uid, bool groupImagesWithGantryTilt, const StringContainer &restrictions)
{
  FileNamesGrouping allSeries = GetSeries(dir, groupImagesWithGantryTilt, restrictions);
  StringContainer resultingFileList;

  for ( FileNamesGrouping::const_iterator idIter = allSeries.begin();
        idIter != allSeries.end();
        ++idIter )
  {
    if ( idIter->first.find( series_uid ) == 0 ) // this ID starts with given series_uid
    {
      return idIter->second.GetFilenames();
    }
  }

  return resultingFileList;
}

DicomSeriesReader::StringContainer
DicomSeriesReader::SortSeriesSlices(const StringContainer &unsortedFilenames)
{
  /* we CAN expect a group of equal
     - series instance uid
     - image orientation
     - pixel spacing
     - imager pixel spacing
     - slice thickness
     - number of rows/columns

     (each piece of information except the rows/columns might be missing)

     sorting with GdcmSortFunction tries its best by sorting by spatial position
     and more hints (acquisition number, acquisition time, trigger time) but will
     always produce a sorting by falling back to SOP Instance UID.
  */
  gdcm::Sorter sorter;

  sorter.SetSortFunction(DicomSeriesReader::GdcmSortFunction);
  try
  {
    if (sorter.Sort(unsortedFilenames))
    {
      return sorter.GetFilenames();
    }
    else
    {
      MITK_WARN << "Sorting error. Leaving series unsorted.";
      return unsortedFilenames;
    }
  }
  catch(std::logic_error&)
  {
    MITK_WARN << "Sorting error. Leaving series unsorted.";
    return unsortedFilenames;
  }
}

bool
DicomSeriesReader::GdcmSortFunction(const gdcm::DataSet &ds1, const gdcm::DataSet &ds2)
{
  // This method MUST accept missing location and position information (and all else, too)
  // because we cannot rely on anything
  // (restriction on the sentence before: we have to provide consistent sorting, so we
  // rely on the minimum information all DICOM files need to provide: SOP Instance UID)

  /* we CAN expect a group of equal
     - series instance uid
     - image orientation
     - pixel spacing
     - imager pixel spacing
     - slice thickness
     - number of rows/columns
  */
  static const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); // Image Position (Patient)
  static const gdcm::Tag    tagImageOrientation(0x0020, 0x0037); // Image Orientation

  // see if we have Image Position and Orientation
  if ( ds1.FindDataElement(tagImagePositionPatient) && ds1.FindDataElement(tagImageOrientation) &&
       ds2.FindDataElement(tagImagePositionPatient) && ds2.FindDataElement(tagImageOrientation) )
  {
    gdcm::Attribute<0x0020,0x0032> image_pos1; // Image Position (Patient)
    gdcm::Attribute<0x0020,0x0037> image_orientation1; // Image Orientation (Patient)

    image_pos1.Set(ds1);
    image_orientation1.Set(ds1);

    gdcm::Attribute<0x0020,0x0032> image_pos2;
    gdcm::Attribute<0x0020,0x0037> image_orientation2;

    image_pos2.Set(ds2);
    image_orientation2.Set(ds2);

    /*
       we tolerate very small differences in image orientation, since we got to know about
       acquisitions where these values change across a single series (7th decimal digit)
       (http://bugs.mitk.org/show_bug.cgi?id=12263)

       still, we want to check if our assumption of 'almost equal' orientations is valid
     */
    for (unsigned int dim = 0; dim < 6; ++dim)
    {
      if ( fabs(image_orientation2[dim] - image_orientation1[dim]) > 0.0001 )
      {
        MITK_ERROR << "Dicom images have different orientations.";
        throw std::logic_error("Dicom images have different orientations. Call GetSeries() first to separate images.");
      }
    }

    double normal[3];

    normal[0] = image_orientation1[1] * image_orientation1[5] - image_orientation1[2] * image_orientation1[4];
    normal[1] = image_orientation1[2] * image_orientation1[3] - image_orientation1[0] * image_orientation1[5];
    normal[2] = image_orientation1[0] * image_orientation1[4] - image_orientation1[1] * image_orientation1[3];

    double
      dist1 = 0.0,
      dist2 = 0.0;

    // this computes the distance from world origin (0,0,0) ALONG THE NORMAL of the image planes
    for (unsigned char i = 0u; i < 3u; ++i)
    {
      dist1 += normal[i] * image_pos1[i];
      dist2 += normal[i] * image_pos2[i];
    }

    // if we can sort by just comparing the distance, we do exactly that
    if ( fabs(dist1 - dist2) >= mitk::eps)
    {
      // default: compare position
      return dist1 < dist2;
    }
    else // we need to check more properties to distinguish slices
    {
      // try to sort by Acquisition Number
      static const gdcm::Tag tagAcquisitionNumber(0x0020, 0x0012);
      if (ds1.FindDataElement(tagAcquisitionNumber) && ds2.FindDataElement(tagAcquisitionNumber))
      {
        gdcm::Attribute<0x0020,0x0012> acquisition_number1; // Acquisition number
        gdcm::Attribute<0x0020,0x0012> acquisition_number2;

        acquisition_number1.Set(ds1);
        acquisition_number2.Set(ds2);

        if (acquisition_number1 != acquisition_number2)
        {
          return acquisition_number1 < acquisition_number2;
        }
        else // neither position nor acquisition number are good for sorting, so check more
        {
          // try to sort by Acquisition Time
          static const gdcm::Tag tagAcquisitionTime(0x0008, 0x0032);
          if (ds1.FindDataElement(tagAcquisitionTime) && ds2.FindDataElement(tagAcquisitionTime))
          {
            gdcm::Attribute<0x0008,0x0032> acquisition_time1; // Acquisition time
            gdcm::Attribute<0x0008,0x0032> acquisition_time2;

            acquisition_time1.Set(ds1);
            acquisition_time2.Set(ds2);

            if (acquisition_time1 != acquisition_time2)
            {
              return acquisition_time1 < acquisition_time2;
            }
            else // we gave up on image position, acquisition number and acquisition time now
            {
              // let's try trigger time
              static const gdcm::Tag tagTriggerTime(0x0018, 0x1060);
              if (ds1.FindDataElement(tagTriggerTime) && ds2.FindDataElement(tagTriggerTime))
              {
                gdcm::Attribute<0x0018,0x1060> trigger_time1; // Trigger time
                gdcm::Attribute<0x0018,0x1060> trigger_time2;

                trigger_time1.Set(ds1);
                trigger_time2.Set(ds2);

                if (trigger_time1 != trigger_time2)
                {
                  return trigger_time1 < trigger_time2;
                }
                // ELSE!
                // for this and many previous ifs we fall through if nothing lets us sort
              } // .
            }   // .
          }     // .
        }
      }
    }
  }             // .

  // LAST RESORT: all valuable information for sorting is missing.
  // Sort by some meaningless but unique identifiers to satisfy the sort function
  static const gdcm::Tag tagSOPInstanceUID(0x0008, 0x0018);
  if (ds1.FindDataElement(tagSOPInstanceUID) && ds2.FindDataElement(tagSOPInstanceUID))
  {
    MITK_DEBUG << "Dicom images are missing attributes for a meaningful sorting, falling back to SOP instance UID comparison.";
    gdcm::Attribute<0x0008,0x0018> SOPInstanceUID1;   // SOP instance UID is mandatory and unique
    gdcm::Attribute<0x0008,0x0018> SOPInstanceUID2;

    SOPInstanceUID1.Set(ds1);
    SOPInstanceUID2.Set(ds2);

    return SOPInstanceUID1 < SOPInstanceUID2;
  }
  else
  {
    // no DICOM file should really come down here, this should only be reached with unskillful and unlucky manipulation of files
    std::string error_message("Malformed DICOM images, which do not even contain a SOP Instance UID.");
    MITK_ERROR << error_message;
    throw std::logic_error( error_message );
  }
}

std::string DicomSeriesReader::GetConfigurationString()
{
  std::stringstream configuration;
  configuration << "MITK_USE_GDCMIO: ";
  configuration << "true";
  configuration << "\n";

  configuration << "GDCM_VERSION: ";
#ifdef GDCM_MAJOR_VERSION
  configuration << GDCM_VERSION;
#endif
  //configuration << "\n";

  return configuration.str();
}

void DicomSeriesReader::CopyMetaDataToImageProperties(StringContainer filenames, const gdcm::Scanner::MappingType &tagValueMappings_, DcmIoType *io, const ImageBlockDescriptor& blockInfo, Image *image)
{
  std::list<StringContainer> imageBlock;
  imageBlock.push_back(filenames);
  CopyMetaDataToImageProperties(imageBlock, tagValueMappings_, io, blockInfo, image);
}

void DicomSeriesReader::CopyMetaDataToImageProperties( std::list<StringContainer> imageBlock, const gdcm::Scanner::MappingType& tagValueMappings_,  DcmIoType* io, const ImageBlockDescriptor& blockInfo, Image* image)
{
  if (!io || !image) return;

  StringLookupTable filesForSlices;
  StringLookupTable sliceLocationForSlices;
  StringLookupTable instanceNumberForSlices;
  StringLookupTable SOPInstanceNumberForSlices;

  gdcm::Scanner::MappingType& tagValueMappings = const_cast<gdcm::Scanner::MappingType&>(tagValueMappings_);

  //DICOM tags which should be added to the image properties
  const gdcm::Tag tagSliceLocation(0x0020, 0x1041); // slice location

  const gdcm::Tag tagInstanceNumber(0x0020, 0x0013); // (image) instance number

  const gdcm::Tag tagSOPInstanceNumber(0x0008, 0x0018); // SOP instance number
  unsigned int timeStep(0);

  std::string propertyKeySliceLocation = "dicom.image.0020.1041";
  std::string propertyKeyInstanceNumber = "dicom.image.0020.0013";
  std::string propertyKeySOPInstanceNumber = "dicom.image.0008.0018";

  // tags for each image
  for ( std::list<StringContainer>::iterator i = imageBlock.begin(); i != imageBlock.end(); i++, timeStep++ )
  {
    const StringContainer& files = (*i);
    unsigned int slice(0);
    for ( StringContainer::const_iterator fIter = files.begin();
          fIter != files.end();
          ++fIter, ++slice )
    {
      filesForSlices.SetTableValue( slice, *fIter );
      gdcm::Scanner::TagToValue tagValueMapForFile = tagValueMappings[fIter->c_str()];
      if(tagValueMapForFile.find(tagSliceLocation) != tagValueMapForFile.end())
        sliceLocationForSlices.SetTableValue(slice, tagValueMapForFile[tagSliceLocation]);
      if(tagValueMapForFile.find(tagInstanceNumber) != tagValueMapForFile.end())
        instanceNumberForSlices.SetTableValue(slice, tagValueMapForFile[tagInstanceNumber]);
      if(tagValueMapForFile.find(tagSOPInstanceNumber) != tagValueMapForFile.end())
        SOPInstanceNumberForSlices.SetTableValue(slice, tagValueMapForFile[tagSOPInstanceNumber]);
    }

    image->SetProperty( "files", StringLookupTableProperty::New( filesForSlices ) );

    //If more than one time step add postfix ".t" + timestep
    if(timeStep != 0)
    {
      std::ostringstream postfix;
      postfix << ".t" << timeStep;

      propertyKeySliceLocation.append(postfix.str());
      propertyKeyInstanceNumber.append(postfix.str());
      propertyKeySOPInstanceNumber.append(postfix.str());
    }
    image->SetProperty( propertyKeySliceLocation.c_str(), StringLookupTableProperty::New( sliceLocationForSlices ) );
    image->SetProperty( propertyKeyInstanceNumber.c_str(), StringLookupTableProperty::New( instanceNumberForSlices ) );
    image->SetProperty( propertyKeySOPInstanceNumber.c_str(), StringLookupTableProperty::New( SOPInstanceNumberForSlices ) );
  }

  // Copy tags for series, study, patient level (leave interpretation to application).
  // These properties will be copied to the DataNode by DicomSeriesReader.

  // tags for the series (we just use the one that ITK copied to its dictionary (proably that of the last slice)
  const itk::MetaDataDictionary& dict = io->GetMetaDataDictionary();
  const TagToPropertyMapType& propertyLookup = DicomSeriesReader::GetDICOMTagsToMITKPropertyMap();

  itk::MetaDataDictionary::ConstIterator dictIter = dict.Begin();
  while ( dictIter != dict.End() )
  {
    //MITK_DEBUG << "Key " << dictIter->first;
    std::string value;
    if ( itk::ExposeMetaData<std::string>( dict, dictIter->first, value ) )
    {
      //MITK_DEBUG << "Value " << value;

      TagToPropertyMapType::const_iterator valuePosition = propertyLookup.find( dictIter->first );
      if ( valuePosition != propertyLookup.end() )
      {
        std::string propertyKey = valuePosition->second;
        //MITK_DEBUG << "--> " << propertyKey;

        image->SetProperty( propertyKey.c_str(), StringProperty::New(value) );
      }
    }
    else
    {
      MITK_WARN << "Tag " << dictIter->first << " not read as string as expected. Ignoring..." ;
    }
    ++dictIter;
  }

  // copy imageblockdescriptor as properties
  image->SetProperty("dicomseriesreader.SOPClass", StringProperty::New(blockInfo.GetSOPClassUIDAsString()));
  image->SetProperty("dicomseriesreader.ReaderImplementationLevelString", StringProperty::New(ReaderImplementationLevelToString( blockInfo.GetReaderImplementationLevel() )));
  image->SetProperty("dicomseriesreader.ReaderImplementationLevel", GenericProperty<ReaderImplementationLevel>::New( blockInfo.GetReaderImplementationLevel() ));
  image->SetProperty("dicomseriesreader.PixelSpacingInterpretationString", StringProperty::New(PixelSpacingInterpretationToString( blockInfo.GetPixelSpacingType() )));
  image->SetProperty("dicomseriesreader.PixelSpacingInterpretation", GenericProperty<PixelSpacingInterpretation>::New(blockInfo.GetPixelSpacingType()));
  image->SetProperty("dicomseriesreader.MultiFrameImage", BoolProperty::New(blockInfo.IsMultiFrameImage()));
  image->SetProperty("dicomseriesreader.GantyTiltCorrected", BoolProperty::New(blockInfo.HasGantryTiltCorrected()));
  image->SetProperty("dicomseriesreader.3D+t", BoolProperty::New(blockInfo.HasMultipleTimePoints()));
}

void DicomSeriesReader::FixSpacingInformation( mitk::Image* image, const ImageBlockDescriptor& imageBlockDescriptor )
{
  // spacing provided by ITK/GDCM
  Vector3D imageSpacing = image->GetGeometry()->GetSpacing();
  ScalarType imageSpacingX = imageSpacing[0];
  ScalarType imageSpacingY = imageSpacing[1];

  // spacing as desired by MITK (preference for "in patient", else "on detector", or "1.0/1.0")
  ScalarType desiredSpacingX = imageSpacingX;
  ScalarType desiredSpacingY = imageSpacingY;
  imageBlockDescriptor.GetDesiredMITKImagePixelSpacing( desiredSpacingX, desiredSpacingY );

  MITK_DEBUG << "Loaded spacing: " << imageSpacingX << "/" << imageSpacingY;
  MITK_DEBUG << "Corrected spacing: " << desiredSpacingX << "/" << desiredSpacingY;

  imageSpacing[0] = desiredSpacingX;
  imageSpacing[1] = desiredSpacingY;
  image->GetGeometry()->SetSpacing( imageSpacing );
}

void DicomSeriesReader::LoadDicom(const StringContainer &filenames, DataNode &node, bool sort, bool load4D, bool correctTilt, UpdateCallBackMethod callback, Image::Pointer preLoadedImageBlock)
{
  const char* previousCLocale = setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  std::locale previousCppLocale( std::cin.getloc() );
  std::locale l( "C" );
  std::cin.imbue(l);

  ImageBlockDescriptor imageBlockDescriptor;

  const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); // Image Position (Patient)
  const gdcm::Tag    tagImageOrientation(0x0020, 0x0037); // Image Orientation
  const gdcm::Tag tagSeriesInstanceUID(0x0020, 0x000e); // Series Instance UID
  const gdcm::Tag tagSOPClassUID(0x0008, 0x0016); // SOP class UID
  const gdcm::Tag tagModality(0x0008, 0x0060); // modality
  const gdcm::Tag tagPixelSpacing(0x0028, 0x0030); // pixel spacing
  const gdcm::Tag tagImagerPixelSpacing(0x0018, 0x1164); // imager pixel spacing
  const gdcm::Tag tagNumberOfFrames(0x0028, 0x0008); // number of frames

  try
  {
    Image::Pointer image = preLoadedImageBlock.IsNull() ? Image::New() : preLoadedImageBlock;
    CallbackCommand *command = callback ? new CallbackCommand(callback) : 0;
    bool initialize_node = false;

    /* special case for Philips 3D+t ultrasound images */
    if ( DicomSeriesReader::IsPhilips3DDicom(filenames.front().c_str())  )
    {
      // TODO what about imageBlockDescriptor?
      // TODO what about preLoadedImageBlock?
      ReadPhilips3DDicom(filenames.front().c_str(), image);
      initialize_node = true;
    }
    else
    {
      /* default case: assume "normal" image blocks, possibly 3D+t */
      bool canLoadAs4D(true);
      gdcm::Scanner scanner;
      ScanForSliceInformation(filenames, scanner);

      // need non-const access for map
      gdcm::Scanner::MappingType& tagValueMappings = const_cast<gdcm::Scanner::MappingType&>(scanner.GetMappings());

      std::list<StringContainer> imageBlocks = SortIntoBlocksFor3DplusT( filenames, tagValueMappings, sort, canLoadAs4D );
      unsigned int volume_count = imageBlocks.size();

      imageBlockDescriptor.SetSeriesInstanceUID( DicomSeriesReader::ConstCharStarToString( scanner.GetValue( filenames.front().c_str(), tagSeriesInstanceUID ) ) );
      imageBlockDescriptor.SetSOPClassUID( DicomSeriesReader::ConstCharStarToString( scanner.GetValue( filenames.front().c_str(), tagSOPClassUID ) ) );
      imageBlockDescriptor.SetModality( DicomSeriesReader::ConstCharStarToString( scanner.GetValue( filenames.front().c_str(), tagModality ) ) );
      imageBlockDescriptor.SetNumberOfFrames( ConstCharStarToString( scanner.GetValue( filenames.front().c_str(), tagNumberOfFrames ) ) );
      imageBlockDescriptor.SetPixelSpacingInformation( ConstCharStarToString( scanner.GetValue( filenames.front().c_str(), tagPixelSpacing ) ),
                                                       ConstCharStarToString( scanner.GetValue( filenames.front().c_str(), tagImagerPixelSpacing ) ) );

      GantryTiltInformation tiltInfo;

      // check possibility of a single slice with many timesteps. In this case, don't check for tilt, no second slice possible
      if ( !imageBlocks.empty() && imageBlocks.front().size() > 1 && correctTilt)
      {
        // check tiltedness here, potentially fixup ITK's loading result by shifting slice contents
        // check first and last position slice from tags, make some calculations to detect tilt

        std::string firstFilename(imageBlocks.front().front());
        // calculate from first and last slice to minimize rounding errors
        std::string secondFilename(imageBlocks.front().back());

        std::string imagePosition1(    ConstCharStarToString( tagValueMappings[ firstFilename.c_str() ][ tagImagePositionPatient ] ) );
        std::string imageOrientation( ConstCharStarToString( tagValueMappings[ firstFilename.c_str() ][ tagImageOrientation ] ) );
        std::string imagePosition2(    ConstCharStarToString( tagValueMappings[secondFilename.c_str() ][ tagImagePositionPatient ] ) );

        bool ignoredConversionError(-42); // hard to get here, no graceful way to react
        Point3D origin1( DICOMStringToPoint3D( imagePosition1, ignoredConversionError ) );
        Point3D origin2( DICOMStringToPoint3D( imagePosition2, ignoredConversionError ) );

        Vector3D right; right.Fill(0.0);
        Vector3D up; right.Fill(0.0); // might be down as well, but it is just a name at this point
        DICOMStringToOrientationVectors( imageOrientation, right, up, ignoredConversionError );

        tiltInfo = GantryTiltInformation ( origin1, origin2, right, up, filenames.size()-1 );
        correctTilt = tiltInfo.IsSheared() && tiltInfo.IsRegularGantryTilt();
      }
      else
      {
        correctTilt = false; // we CANNOT do that
      }

      imageBlockDescriptor.SetHasGantryTiltCorrected( correctTilt );

      if (volume_count == 1 || !canLoadAs4D || !load4D)
      {
        DcmIoType::Pointer io;
        image = MultiplexLoadDICOMByITK( imageBlocks.front(), correctTilt, tiltInfo, io, command, preLoadedImageBlock ); // load first 3D block

        imageBlockDescriptor.AddFiles(imageBlocks.front()); // only the first part is loaded
        imageBlockDescriptor.SetHasMultipleTimePoints( false );

        FixSpacingInformation( image, imageBlockDescriptor );
        CopyMetaDataToImageProperties( imageBlocks.front(), scanner.GetMappings(), io, imageBlockDescriptor, image);

        initialize_node = true;
      }
      else if (volume_count > 1)
      {
        imageBlockDescriptor.AddFiles(filenames); // all is loaded
        imageBlockDescriptor.SetHasMultipleTimePoints( true );

        DcmIoType::Pointer io;
        image = MultiplexLoadDICOMByITK4D( imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock );

        initialize_node = true;
      }
    }

    if (initialize_node)
    {
      // forward some image properties to node
      node.GetPropertyList()->ConcatenatePropertyList( image->GetPropertyList(), true );

      std::string patientName = "NoName";
      if(node.GetProperty("dicom.patient.PatientsName"))
        patientName = node.GetProperty("dicom.patient.PatientsName")->GetValueAsString();

      node.SetData( image );
      node.SetName(patientName);
      setlocale(LC_NUMERIC, previousCLocale);
      std::cin.imbue(previousCppLocale);
    }

    MITK_DEBUG << "--------------------------------------------------------------------------------";
    MITK_DEBUG << "DICOM files loaded (from series UID " << imageBlockDescriptor.GetSeriesInstanceUID() << "):";
    MITK_DEBUG << "  " << imageBlockDescriptor.GetFilenames().size() << " '" << imageBlockDescriptor.GetModality() << "' files (" << imageBlockDescriptor.GetSOPClassUIDAsString() << ") loaded into 1 mitk::Image";
    MITK_DEBUG << "  multi-frame: " << (imageBlockDescriptor.IsMultiFrameImage()?"Yes":"No");
    MITK_DEBUG << "  reader support: " << ReaderImplementationLevelToString(imageBlockDescriptor.GetReaderImplementationLevel());
    MITK_DEBUG << "  pixel spacing type: " << PixelSpacingInterpretationToString( imageBlockDescriptor.GetPixelSpacingType() ) << " " << image->GetGeometry()->GetSpacing()[0] << "/" << image->GetGeometry()->GetSpacing()[0];
    MITK_DEBUG << "  gantry tilt corrected: " << (imageBlockDescriptor.HasGantryTiltCorrected()?"Yes":"No");
    MITK_DEBUG << "  3D+t: " << (imageBlockDescriptor.HasMultipleTimePoints()?"Yes":"No");
    MITK_DEBUG << "--------------------------------------------------------------------------------";
  }
  catch (std::exception& e)
  {
    // reset locale then throw up
    setlocale(LC_NUMERIC, previousCLocale);
    std::cin.imbue(previousCppLocale);

    MITK_DEBUG << "Caught exception in DicomSeriesReader::LoadDicom";

    throw e;
  }
}

void
DicomSeriesReader::ScanForSliceInformation(const StringContainer &filenames, gdcm::Scanner& scanner)
{
  const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); //Image position (Patient)
  scanner.AddTag(tagImagePositionPatient);

  const gdcm::Tag tagSeriesInstanceUID(0x0020, 0x000e); // Series Instance UID
  scanner.AddTag(tagSeriesInstanceUID);

  const gdcm::Tag tagImageOrientation(0x0020,0x0037); //Image orientation
  scanner.AddTag(tagImageOrientation);

  const gdcm::Tag tagSliceLocation(0x0020, 0x1041); // slice location
  scanner.AddTag( tagSliceLocation );

  const gdcm::Tag tagInstanceNumber(0x0020, 0x0013); // (image) instance number
  scanner.AddTag( tagInstanceNumber );

  const gdcm::Tag tagSOPInstanceNumber(0x0008, 0x0018); // SOP instance number
  scanner.AddTag( tagSOPInstanceNumber );

  const gdcm::Tag tagPixelSpacing(0x0028, 0x0030); // Pixel Spacing
  scanner.AddTag( tagPixelSpacing );

  const gdcm::Tag tagImagerPixelSpacing(0x0018, 0x1164); // Imager Pixel Spacing
  scanner.AddTag( tagImagerPixelSpacing );

  const gdcm::Tag tagModality(0x0008, 0x0060); // Modality
  scanner.AddTag( tagModality );

  const gdcm::Tag tagSOPClassUID(0x0008, 0x0016); // SOP Class UID
  scanner.AddTag( tagSOPClassUID );

  const gdcm::Tag tagNumberOfFrames(0x0028, 0x0008); // number of frames
    scanner.AddTag( tagNumberOfFrames );

  scanner.Scan(filenames); // make available image information for each file
}

std::list<DicomSeriesReader::StringContainer>
DicomSeriesReader::SortIntoBlocksFor3DplusT(
    const StringContainer& presortedFilenames,
    const gdcm::Scanner::MappingType& tagValueMappings,
    bool /*sort*/,
    bool& canLoadAs4D )
{
  std::list<StringContainer> imageBlocks;

  // ignore sort request, because most likely re-sorting is now needed due to changes in GetSeries(bug #8022)
  StringContainer sorted_filenames = DicomSeriesReader::SortSeriesSlices(presortedFilenames);

  std::string firstPosition;
  unsigned int numberOfBlocks(0); // number of 3D image blocks

  static const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); //Image position (Patient)
  const gdcm::Tag tagModality(0x0008,0x0060);

  // loop files to determine number of image blocks
  for (StringContainer::const_iterator fileIter = sorted_filenames.begin();
       fileIter != sorted_filenames.end();
       ++fileIter)
  {
    gdcm::Scanner::TagToValue tagToValueMap = tagValueMappings.find( fileIter->c_str() )->second;

    if(tagToValueMap.find(tagImagePositionPatient) == tagToValueMap.end())
    {
      const std::string& modality = tagToValueMap.find(tagModality)->second;
      if ( modality.compare("RTIMAGE ") == 0 || modality.compare("RTIMAGE") == 0 )
      {
        MITK_WARN << "Modality "<< modality <<" is not fully supported yet.";
        numberOfBlocks = 1;
        break;
      } else {
        // we expect to get images w/ missing position information ONLY as separated blocks.
        assert( presortedFilenames.size() == 1 );
        numberOfBlocks = 1;
        break;
      }
    }

    std::string position = tagToValueMap.find(tagImagePositionPatient)->second;
    MITK_DEBUG << "  " << *fileIter << " at " << position;
    if (firstPosition.empty())
    {
      firstPosition = position;
    }

    if ( position == firstPosition )
    {
      ++numberOfBlocks;
    }
    else
    {
      break; // enough information to know the number of image blocks
    }
  }

  MITK_DEBUG << "  ==> Assuming " << numberOfBlocks << " time steps";

  if (numberOfBlocks == 0) return imageBlocks; // only possible if called with no files


  // loop files to sort them into image blocks
  unsigned int numberOfExpectedSlices(0);
  for (unsigned int block = 0; block < numberOfBlocks; ++block)
  {
    StringContainer filesOfCurrentBlock;

    for ( StringContainer::const_iterator fileIter = sorted_filenames.begin() + block;
         fileIter != sorted_filenames.end();
         //fileIter += numberOfBlocks) // TODO shouldn't this work? give invalid iterators on first attempts
         )
    {
      filesOfCurrentBlock.push_back( *fileIter );
      for (unsigned int b = 0; b < numberOfBlocks; ++b)
      {
        if (fileIter != sorted_filenames.end())
          ++fileIter;
      }
    }

    imageBlocks.push_back(filesOfCurrentBlock);

    if (block == 0)
    {
      numberOfExpectedSlices = filesOfCurrentBlock.size();
    }
    else
    {
      if (filesOfCurrentBlock.size() != numberOfExpectedSlices)
      {
        MITK_WARN << "DicomSeriesReader expected " << numberOfBlocks
                  << " image blocks of "
                  << numberOfExpectedSlices
                  << " images each. Block "
                  << block
                  << " got "
                  << filesOfCurrentBlock.size()
                  << " instead. Cannot load this as 3D+t"; // TODO implement recovery (load as many slices 3D+t as much as possible)
        canLoadAs4D = false;
      }
    }
  }

  return imageBlocks;
}

Image::Pointer
DicomSeriesReader
::MultiplexLoadDICOMByITK(const StringContainer& filenames, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, Image::Pointer preLoadedImageBlock)
{
  io = DcmIoType::New();
  io->SetFileName(filenames.front().c_str());
  io->ReadImageInformation();

  if (io->GetPixelType() == itk::ImageIOBase::SCALAR)
  {
    return MultiplexLoadDICOMByITKScalar(filenames, correctTilt, tiltInfo, io, command ,preLoadedImageBlock);
  }
  else if (io->GetPixelType() == itk::ImageIOBase::RGB)
  {
    return MultiplexLoadDICOMByITKRGBPixel(filenames, correctTilt, tiltInfo, io, command ,preLoadedImageBlock);
  }
  else
  {
    return NULL;
  }
}

Image::Pointer
DicomSeriesReader
::MultiplexLoadDICOMByITK4D( std::list<StringContainer>& imageBlocks, ImageBlockDescriptor imageBlockDescriptor, bool correctTilt, const GantryTiltInformation& tiltInfo, DcmIoType::Pointer& io, CallbackCommand* command, Image::Pointer preLoadedImageBlock)
{
  io = DcmIoType::New();
  io->SetFileName(imageBlocks.front().front().c_str());
  io->ReadImageInformation();

  if (io->GetPixelType() == itk::ImageIOBase::SCALAR)
  {
    return MultiplexLoadDICOMByITK4DScalar(imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command ,preLoadedImageBlock);
  }
  else if (io->GetPixelType() == itk::ImageIOBase::RGB)
  {
    return MultiplexLoadDICOMByITK4DRGBPixel(imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command ,preLoadedImageBlock);
  }
  else
  {
    return NULL;
  }
}
} // end namespace mitk
