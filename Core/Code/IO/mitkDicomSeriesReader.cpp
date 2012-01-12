/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// uncomment for learning more about the internal sorting mechanisms
//#define MBILOG_ENABLE_DEBUG 

#include <mitkDicomSeriesReader.h>

#include <itkGDCMSeriesFileNames.h>

#include <gdcmAttribute.h>
#include <gdcmPixmapReader.h>
#include <gdcmStringFilter.h>
#include <gdcmDirectory.h>
#include <gdcmScanner.h>

#include "mitkProperties.h"

namespace mitk
{

typedef itk::GDCMSeriesFileNames DcmFileNamesGeneratorType;

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

    initialized = true;
  }

  return dictionary;
}


DataNode::Pointer 
DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames, bool sort, bool check_4d, UpdateCallBackMethod callback)
{
  DataNode::Pointer node = DataNode::New();

  if (DicomSeriesReader::LoadDicomSeries(filenames, *node, sort, check_4d, callback))
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
DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames, DataNode &node, bool sort, bool check_4d, UpdateCallBackMethod callback)
{
  if( filenames.empty() )
  {
    MITK_WARN << "Calling LoadDicomSeries with empty filename string container. Probably invalid application logic.";
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

      switch (io->GetComponentType())
      {
      case DcmIoType::UCHAR:
        DicomSeriesReader::LoadDicom<unsigned char>(filenames, node, sort, check_4d, callback);
        return true;
      case DcmIoType::CHAR:
        DicomSeriesReader::LoadDicom<char>(filenames, node, sort, check_4d, callback);
        return true;
      case DcmIoType::USHORT:
        DicomSeriesReader::LoadDicom<unsigned short>(filenames, node, sort, check_4d, callback);
        return true;
      case DcmIoType::SHORT:
        DicomSeriesReader::LoadDicom<short>(filenames, node, sort, check_4d, callback);
        return true;
      case DcmIoType::UINT:
        DicomSeriesReader::LoadDicom<unsigned int>(filenames, node, sort, check_4d, callback);
        return true;
      case DcmIoType::INT:
        DicomSeriesReader::LoadDicom<int>(filenames, node, sort, check_4d, callback);
        return true;
      case DcmIoType::ULONG:
        DicomSeriesReader::LoadDicom<long unsigned int>(filenames, node, sort, check_4d, callback);
        return true;
      case DcmIoType::LONG:
        DicomSeriesReader::LoadDicom<long int>(filenames, node, sort, check_4d, callback);
        return true;
      case DcmIoType::FLOAT:
        DicomSeriesReader::LoadDicom<float>(filenames, node, sort, check_4d, callback);
        return true;
      case DcmIoType::DOUBLE:
        DicomSeriesReader::LoadDicom<double>(filenames, node, sort, check_4d, callback);
        return true;
      default:
        MITK_ERROR << "Found unsupported DICOM pixel type: (enum value) " << io->GetComponentType();
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
DicomSeriesReader::ReadPhilips3DDicom(const std::string &filename, mitk::Image::Pointer output_image)
{
  // Now get PhilipsSpecific Tags

  gdcm::PixmapReader reader;
  reader.SetFileName(filename.c_str());
  reader.Read();
  gdcm::DataSet &data_set = reader.GetFile().GetDataSet();
  gdcm::StringFilter sf;
  sf.SetFile(reader.GetFile());

  gdcm::Attribute<0x0028,0x0011> dimTagX; // coloumns || sagittal
  gdcm::Attribute<0x3001,0x1001, gdcm::VR::UL, gdcm::VM::VM1> dimTagZ; //I have no idea what is VM1. // (Philips specific) // transversal
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

DicomSeriesReader::TwoStringContainers
DicomSeriesReader::AnalyzeFileForITKImageSeriesReaderSpacingAssumption(
    const StringContainer& files,
    const gdcm::Scanner::MappingType& tagValueMappings_)
{
  // result.first = files that fit ITK's assumption
  // result.second = files that do not fit, should be run through AnalyzeFileForITKImageSeriesReaderSpacingAssumption() again
  TwoStringContainers result;
 
  // we const_cast here, because I could not use a map.at(), which would make the code much more readable
  gdcm::Scanner::MappingType& tagValueMappings = const_cast<gdcm::Scanner::MappingType&>(tagValueMappings_);
  const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); // Image Position (Patient)
  const gdcm::Tag    tagImageOrientation(0x0020, 0x0037); // Image Orientation

  Vector3D fromFirstToSecondOrigin; fromFirstToSecondOrigin.Fill(0.0);
  bool fromFirstToSecondOriginInitialized(false);
  Point3D thisOrigin;
  Point3D lastOrigin;
  Point3D lastDifferentOrigin;
  bool lastOriginInitialized(false);

  MITK_DEBUG << "--------------------------------------------------------------------------------";
  MITK_DEBUG << "Analyzing files for z-spacing assumption of ITK's ImageSeriesReader ";
  unsigned int fileIndex(0);
  for (StringContainer::const_iterator fileIter = files.begin();
       fileIter != files.end();
       ++fileIter, ++fileIndex)
  {
    bool fileFitsIntoPattern(false);
    std::string thisOriginString;
    // Read tag value into point3D. PLEASE replace this by appropriate GDCM code if you figure out how to do that
    const char* value = tagValueMappings[fileIter->c_str()][tagImagePositionPatient];
    if (value)
    {
      thisOriginString = value;
    }

    std::istringstream originReader(thisOriginString);
    std::string coordinate;
    unsigned int dim(0);
    while( std::getline( originReader, coordinate, '\\' ) ) thisOrigin[dim++] = atof(coordinate.c_str());

    if (dim != 3)
    {
      MITK_ERROR << "Reader implementation made wrong assumption on tag (0020,0032). Found " << dim << "instead of 3 values.";
    }

    MITK_DEBUG << "  " << fileIndex << " " << *fileIter
                       << " at "
                       << thisOriginString << "(" << thisOrigin[0] << "," << thisOrigin[1] << "," << thisOrigin[2] << ")";

    if ( lastOriginInitialized && (thisOrigin == lastOrigin) )
    {
      MITK_DEBUG << "    ==> Sort away " << *fileIter << " for separate time step"; // we already have one occupying this position
      result.second.push_back( *fileIter );
      fileFitsIntoPattern = false;
    }
    else
    {
      if (!fromFirstToSecondOriginInitialized && lastOriginInitialized) // calculate vector as soon as possible when we get a new position
      {
        fromFirstToSecondOrigin = thisOrigin - lastDifferentOrigin;
        fromFirstToSecondOriginInitialized = true;

        // Now make sure this direction is along the normal vector of the first slice
        // If this is NOT the case, then we have a data set with a TILTED GANTRY geometry,
        // which cannot be loaded into a single mitk::Image at the moment

        // Again ugly code to read tag Image Orientation into two vEctors
        Vector3D right; right.Fill(0.0);
        Vector3D up; right.Fill(0.0); // might be down as well, but it is just a name at this point
        std::string thisOrientationString;
        const char* value = tagValueMappings[fileIter->c_str()][tagImageOrientation];
        if (value)
        {
          thisOrientationString = value;
        }
        std::istringstream orientationReader(thisOrientationString);
        std::string coordinate;
        unsigned int dim(0);
        while( std::getline( orientationReader, coordinate, '\\' ) )
          if (dim<3) right[dim++] = atof(coordinate.c_str());
          else      up[dim++ - 3] = atof(coordinate.c_str());

        if (dim != 6)
        {
          MITK_ERROR << "Reader implementation made wrong assumption on tag (0020,0037). Found " << dim << "instead of 6 values.";
        }


        /* 
           Determine if line (thisOrigin + l * normal) contains lastDifferentOrigin.

           Done by calculating the distance of lastDifferentOrigin from line (thisOrigin + l *normal)

           E.g. http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html

           squared distance = | (pointAlongNormal - thisOrign) x (thisOrigin - lastDifferentOrigin) | ^ 2
                              /
                              |pointAlongNormal - thisOrigin| ^ 2

           ( x meaning the cross product )

        MITK_DEBUG << "Tilt check: right vector (" << right[0] << "," << right[1] << "," << right[2] << "), "
                                     "up vector (" << up[0] << "," << up[1] << "," << up[2] << ")";
        */

        Vector3D normal = itk::CrossProduct(right, up);
        Point3D pointAlongNormal = thisOrigin + normal;
        
        double numerator = itk::CrossProduct( pointAlongNormal - thisOrigin , thisOrigin - lastDifferentOrigin ).GetSquaredNorm();
        double denominator = (pointAlongNormal - thisOrigin).GetSquaredNorm();

        double distance = sqrt(numerator / denominator);

        if (distance > 0.001) // mitk::eps is too small; 1/1000 of a mm should be enough to detect tilt
        {
          MITK_DEBUG << "  Series might contain a tilted geometry";
          MITK_DEBUG << "  Distance of expected slice origin from actual slice origin: " << distance;
          MITK_DEBUG  << "    ==> Sort away " << *fileIter << " for later analysis";

          /* Pessimistic approach: split block right here

          result.first.assign( files.begin(), fileIter );
          result.second.insert( result.second.end(), fileIter, files.end() );

          return result; // stop processing with first split
          */

          /* optimistic approach: save file for later, check all further files */
          
          result.second.push_back(*fileIter);
          fileFitsIntoPattern = false;
        }
        else
        {
          result.first.push_back(*fileIter); // this file is good for current block
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
          
          /*
             Pessimistic approach: split right here:

          result.first.assign( files.begin(), fileIter );
          result.second.insert( result.second.end(), fileIter, files.end() );

          return result; // stop processing with first split
          */

          /* Optimistic approach: check if any of the remaining slices fits in */
          result.second.push_back( *fileIter ); // sort away for further analysis
          fileFitsIntoPattern = false;
        }
        else
        {
          result.first.push_back(*fileIter); // this file is good for current block
          fileFitsIntoPattern = true;
        }
      }
      else // this should be the very first slice
      {
        result.first.push_back(*fileIter); // this file is good for current block
        fileFitsIntoPattern = true;
      }
    }

    // recored current origin for reference in later iterations
    if ( !lastOriginInitialized || ( fileFitsIntoPattern && (thisOrigin != lastOrigin) ) )
    {
      lastDifferentOrigin = thisOrigin;
    }

    lastOrigin = thisOrigin; 
    lastOriginInitialized = true;
  }
  
  return result;
}

DicomSeriesReader::UidFileNamesMap 
DicomSeriesReader::GetSeries(const StringContainer& files, const StringContainer &restrictions)
{
  return GetSeries(files, true, restrictions);
}
  
DicomSeriesReader::UidFileNamesMap 
DicomSeriesReader::GetSeries(const StringContainer& files, bool sortTo3DPlust, const StringContainer& /*restrictions*/)
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

  UidFileNamesMap groupsOfSimilarImages; // preliminary result, refined into the final result mapOf3DPlusTBlocks

  // use GDCM directly, itk::GDCMSeriesFileNames does not work with GDCM 2

  // PART I: scan files for sorting relevant DICOM tags, 
  //         separate images that differ in any of those 
  //         attributes (they cannot possibly form a 3D block)

  // scan for relevant tags in dicom files
  gdcm::Scanner scanner;
  const gdcm::Tag tagSeriesInstanceUID(0x0020,0x000e); // Series Instance UID
    scanner.AddTag( tagSeriesInstanceUID );

  const gdcm::Tag tagImageOrientation(0x0020, 0x0037); // image orientation
    scanner.AddTag( tagImageOrientation );

  const gdcm::Tag tagPixelSpacing(0x0028, 0x0030); // pixel spacing
    scanner.AddTag( tagPixelSpacing );
    
  const gdcm::Tag tagSliceThickness(0x0018, 0x0050); // slice thickness
    scanner.AddTag( tagSliceThickness );

  const gdcm::Tag tagNumberOfRows(0x0028, 0x0010); // number rows
    scanner.AddTag( tagNumberOfRows );

  const gdcm::Tag tagNumberOfColumns(0x0028, 0x0011); // number cols
    scanner.AddTag( tagNumberOfColumns );
  
  // additional tags read in this scan to allow later analysis
  // THESE tag are not used for initial separating of files
  const gdcm::Tag tagImagePositionPatient(0x0020,0x0032); // Image Position (Patient)
    scanner.AddTag( tagImagePositionPatient );

  // TODO add further restrictions from arguments

  // let GDCM scan files
  if ( !scanner.Scan( files ) )
  {
    MITK_ERROR << "gdcm::Scanner failed when scanning " << files.size() << " input files.";
    return groupsOfSimilarImages;
  }

  // assign files IDs that will separate them for loading into image blocks
  for (gdcm::Scanner::ConstIterator fileIter = scanner.Begin();
       fileIter != scanner.End();
       ++fileIter)
  {
    //MITK_DEBUG << "Scan file " << fileIter->first << std::endl;
    if ( std::string(fileIter->first).empty() ) continue; // TODO understand why Scanner has empty string entries

    // we const_cast here, because I could not use a map.at() function in CreateMoreUniqueSeriesIdentifier.
    // doing the same thing with find would make the code less readable. Since we forget the Scanner results
    // anyway after this function, we can simply tolerate empty map entries introduced by bad operator[] access
    std::string moreUniqueSeriesId = CreateMoreUniqueSeriesIdentifier( const_cast<gdcm::Scanner::TagToValue&>(fileIter->second) );
    groupsOfSimilarImages [ moreUniqueSeriesId ].push_back( fileIter->first );
  }
  
  // PART III: sort slices spatially

  for ( UidFileNamesMap::const_iterator groupIter = groupsOfSimilarImages.begin(); groupIter != groupsOfSimilarImages.end(); ++groupIter )
  {
    try
    {
    groupsOfSimilarImages[ groupIter->first ] = SortSeriesSlices( groupIter->second  ); // sort each slice group spatially
    } catch(...)
    {
       MITK_ERROR << "Catched something.";
    }
  }

  // PART II: analyze pre-sorted images for valid blocks (i.e. blocks of equal z-spacing), 
  //          separate into multiple blocks if necessary.
  //          
  //          Analysis performs the following steps:
  //            * imitate itk::ImageSeriesReader: use the distance between the first two images as z-spacing
  //            * check what images actually fulfill ITK's z-spacing assumption
  //            * separate all images that fail the test into new blocks, re-iterate analysis for these blocks

  UidFileNamesMap mapOf3DPlusTBlocks; // final result of this function
  for ( UidFileNamesMap::const_iterator groupIter = groupsOfSimilarImages.begin(); groupIter != groupsOfSimilarImages.end(); ++groupIter )
  {
    UidFileNamesMap mapOf3DBlocks;      // intermediate result for only this group(!)
    StringContainer filesStillToAnalyze = groupIter->second;
    std::string groupUID = groupIter->first;
    unsigned int subgroup(0);
    MITK_DEBUG << "Analyze group " << groupUID;

    while (!filesStillToAnalyze.empty()) // repeat until all files are grouped somehow 
    {
      TwoStringContainers analysisResult = AnalyzeFileForITKImageSeriesReaderSpacingAssumption( filesStillToAnalyze, scanner.GetMappings() );

      // enhance the UID for additional groups
      std::stringstream newGroupUID;
      newGroupUID << groupUID << '.' << subgroup;
      mapOf3DBlocks[ newGroupUID.str() ] = analysisResult.first;
      MITK_DEBUG << "Result: sorted 3D group " << newGroupUID.str() << " with " << mapOf3DBlocks[ newGroupUID.str() ].size() << " files";
      
      ++subgroup;
        
      filesStillToAnalyze = analysisResult.second; // remember what needs further analysis
    }

    // end of grouping, now post-process groups
  
    // PART IV: attempt to group blocks to 3D+t blocks if requested
    //           inspect entries of mapOf3DBlocks
    //            - if number of files is identical to previous entry, collect for 3D+t block
    //            - as soon as number of files changes from previous entry, record collected blocks as 3D+t block, start a new one, continue

    // decide whether or not to group 3D blocks into 3D+t blocks where possible
    if ( !sortTo3DPlust )
    {
      // copy 3D blocks to output
      // TODO avoid collisions (or prove impossibility)
      mapOf3DPlusTBlocks.insert( mapOf3DBlocks.begin(), mapOf3DBlocks.end() );
    }
    else
    {
      // sort 3D+t (as described in "PART IV")
      
      MITK_DEBUG << "================================================================================";
      MITK_DEBUG << "3D+t analysis:";

      unsigned int numberOfFilesInPreviousBlock(0);
      std::string previousBlockKey;

      for ( UidFileNamesMap::const_iterator block3DIter = mapOf3DBlocks.begin();
            block3DIter != mapOf3DBlocks.end();
            ++block3DIter )
      {
        unsigned int numberOfFilesInThisBlock = block3DIter->second.size();
        std::string thisBlockKey = block3DIter->first;
        
        if (numberOfFilesInPreviousBlock == 0)
        {
          numberOfFilesInPreviousBlock = numberOfFilesInThisBlock;
          mapOf3DPlusTBlocks[thisBlockKey].insert( mapOf3DPlusTBlocks[thisBlockKey].end(), 
                                                   block3DIter->second.begin(), 
                                                   block3DIter->second.end() );
          MITK_DEBUG << "  3D+t group " << thisBlockKey << " started";
          previousBlockKey = thisBlockKey;
        }
        else
        {
          bool identicalOrigins;
          try {
            // check whether this and the previous block share a comon origin
            // TODO should be safe, but a little try/catch or other error handling wouldn't hurt
            std::string thisOriginString = scanner.GetValue( mapOf3DBlocks[thisBlockKey].front().c_str(), tagImagePositionPatient );
            std::string previousOriginString = scanner.GetValue( mapOf3DBlocks[previousBlockKey].front().c_str(), tagImagePositionPatient );

            // also compare last origin, because this might differ if z-spacing is different
            std::string thisDestinationString = scanner.GetValue( mapOf3DBlocks[thisBlockKey].back().c_str(), tagImagePositionPatient );
            std::string previousDestinationString = scanner.GetValue( mapOf3DBlocks[previousBlockKey].back().c_str(), tagImagePositionPatient );

            identicalOrigins =  ( (thisOriginString == previousOriginString) && (thisDestinationString == previousDestinationString) );

          } catch(...)
          {
            identicalOrigins = false;
          }

          if (identicalOrigins && (numberOfFilesInPreviousBlock == numberOfFilesInThisBlock))
          {
            // group with previous block
            mapOf3DPlusTBlocks[previousBlockKey].insert( mapOf3DPlusTBlocks[previousBlockKey].end(), 
                                                         block3DIter->second.begin(), 
                                                         block3DIter->second.end() );
            MITK_DEBUG << "  --> group enhanced with another timestep";
          }
          else
          {
            // start a new block
            mapOf3DPlusTBlocks[thisBlockKey].insert( mapOf3DPlusTBlocks[thisBlockKey].end(), 
                                                     block3DIter->second.begin(), 
                                                     block3DIter->second.end() );
            MITK_DEBUG << "  ==> group closed with " << mapOf3DPlusTBlocks[previousBlockKey].size() / numberOfFilesInPreviousBlock << " time steps";
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
  for ( UidFileNamesMap::const_iterator groupIter = mapOf3DPlusTBlocks.begin(); groupIter != mapOf3DPlusTBlocks.end(); ++groupIter )
  {
    MITK_DEBUG << "  Image volume " << groupIter->first << " with " << groupIter->second.size() << " files";
  }
  MITK_DEBUG << "Done. ";
  MITK_DEBUG << "================================================================================";

  return mapOf3DPlusTBlocks;
}

DicomSeriesReader::UidFileNamesMap 
DicomSeriesReader::GetSeries(const std::string &dir, const StringContainer &restrictions)
{
  gdcm::Directory directoryLister;
  directoryLister.Load( dir.c_str(), false ); // non-recursive
  return GetSeries(directoryLister.GetFilenames(), restrictions);
}

std::string
DicomSeriesReader::CreateSeriesIdentifierPart( gdcm::Scanner::TagToValue& tagValueMap, const gdcm::Tag& tag )
{
  std::string result;
  try
  {
    result = IDifyTagValue( tagValueMap[ tag ] ? tagValueMap[ tag ] : std::string("") );
  }
  catch (std::exception& e)
  {
    MITK_WARN << "Could not access tag " << tag << ": " << e.what();
  }
   
  return result;
}

std::string 
DicomSeriesReader::CreateMoreUniqueSeriesIdentifier( gdcm::Scanner::TagToValue& tagValueMap )
{
  const gdcm::Tag tagSeriesInstanceUID(0x0020,0x000e); // Series Instance UID
  const gdcm::Tag tagImageOrientation(0x0020, 0x0037); // image orientation
  const gdcm::Tag tagPixelSpacing(0x0028, 0x0030); // pixel spacing
  const gdcm::Tag tagSliceThickness(0x0018, 0x0050); // slice thickness
  const gdcm::Tag tagNumberOfRows(0x0028, 0x0010); // number rows
  const gdcm::Tag tagNumberOfColumns(0x0028, 0x0011); // number cols
    
  std::string constructedID;
 
  try
  {
    constructedID = tagValueMap[ tagSeriesInstanceUID ];
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "CreateMoreUniqueSeriesIdentifier() could not access series instance UID. Something is seriously wrong with this image.";
    MITK_ERROR << "Error from exception: " << e.what();
  }
 
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagNumberOfRows );
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagNumberOfColumns );
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagPixelSpacing );
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagSliceThickness );
  constructedID += CreateSeriesIdentifierPart( tagValueMap, tagImageOrientation );

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
DicomSeriesReader::GetSeries(const std::string &dir, const std::string &series_uid, const StringContainer &restrictions)
{
  UidFileNamesMap allSeries = GetSeries(dir, restrictions);
  StringContainer resultingFileList;

  for ( UidFileNamesMap::const_iterator idIter = allSeries.begin(); 
        idIter != allSeries.end(); 
        ++idIter )
  {
    if ( idIter->first.find( series_uid ) == 0 ) // this ID starts with given series_uid
    {
      resultingFileList.insert( resultingFileList.end(), idIter->second.begin(), idIter->second.end() ); // append
    }
  }

  return resultingFileList;
}

DicomSeriesReader::StringContainer 
DicomSeriesReader::SortSeriesSlices(const StringContainer &unsortedFilenames)
{
  gdcm::Sorter sorter;

  sorter.SetSortFunction(DicomSeriesReader::GdcmSortFunction);
  try
  {
    sorter.Sort(unsortedFilenames);
    return sorter.GetFilenames();
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
  // make sure we habe Image Position and Orientation
  if ( ! ( 
      ds1.FindDataElement(gdcm::Tag(0x0020,0x0032)) &&
      ds1.FindDataElement(gdcm::Tag(0x0020,0x0037)) &&
      ds2.FindDataElement(gdcm::Tag(0x0020,0x0032)) &&
      ds2.FindDataElement(gdcm::Tag(0x0020,0x0037)) 
        )
      )
      {
        MITK_WARN << "Dicom images are missing attributes for a meaningful sorting.";
        throw std::logic_error("Dicom images are missing attributes for a meaningful sorting.");
      }
  
  gdcm::Attribute<0x0020,0x0032> image_pos1; // Image Position (Patient)
  gdcm::Attribute<0x0020,0x0037> image_orientation1; // Image Orientation (Patient)

  image_pos1.Set(ds1);
  image_orientation1.Set(ds1);

  gdcm::Attribute<0x0020,0x0032> image_pos2;
  gdcm::Attribute<0x0020,0x0037> image_orientation2;

  image_pos2.Set(ds2);
  image_orientation2.Set(ds2);

  if (image_orientation1 != image_orientation2)
  {
    MITK_ERROR << "Dicom images have different orientations.";
    throw std::logic_error("Dicom images have different orientations. Call GetSeries() first to separate images.");
  }

  double normal[3];

  normal[0] = image_orientation1[1] * image_orientation1[5] - image_orientation1[2] * image_orientation1[4];
  normal[1] = image_orientation1[2] * image_orientation1[3] - image_orientation1[0] * image_orientation1[5];
  normal[2] = image_orientation1[0] * image_orientation1[4] - image_orientation1[1] * image_orientation1[3];

  double
      dist1 = 0.0,
      dist2 = 0.0;

  for (unsigned char i = 0u; i < 3u; ++i)
  {
    dist1 += normal[i] * image_pos1[i];
    dist2 += normal[i] * image_pos2[i];
  }

  if ( fabs(dist1 - dist2) < mitk::eps)
  {
    gdcm::Attribute<0x0008,0x0032> acq_time1; // Acquisition time (may be missing, so we check existence first)
    gdcm::Attribute<0x0008,0x0032> acq_time2;

    if (ds1.FindDataElement(gdcm::Tag(0x0008,0x0032)))
      acq_time1.Set(ds1);

    if (ds2.FindDataElement(gdcm::Tag(0x0008,0x0032)))
      acq_time2.Set(ds2);

    // TODO this could lead to comparison of unset times (does Attribute initialize to good defaults?)
    // exception: same position: compare by acquisition time
    return acq_time1 < acq_time2;
  }
  else
  {
    // default: compare position
    return dist1 < dist2;
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

void DicomSeriesReader::CopyMetaDataToImageProperties(StringContainer filenames, const gdcm::Scanner::MappingType &tagValueMappings_, DcmIoType *io, Image *image)
{
  std::list<StringContainer> imageBlock;
  imageBlock.push_back(filenames);
  CopyMetaDataToImageProperties(imageBlock, tagValueMappings_, io, image);
}
  
void DicomSeriesReader::CopyMetaDataToImageProperties( std::list<StringContainer> imageBlock, const gdcm::Scanner::MappingType& tagValueMappings_,  DcmIoType* io, Image* image)
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
      propertyKeySliceLocation.append(".t" + timeStep);
      propertyKeyInstanceNumber.append(".t" + timeStep);
      propertyKeySOPInstanceNumber.append(".t" + timeStep);
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
    MITK_DEBUG << "Key " << dictIter->first;
    std::string value;
    if ( itk::ExposeMetaData<std::string>( dict, dictIter->first, value ) )
    {
      MITK_DEBUG << "Value " << value;

      TagToPropertyMapType::const_iterator valuePosition = propertyLookup.find( dictIter->first );
      if ( valuePosition != propertyLookup.end() )
      {
        std::string propertyKey = valuePosition->second;
        MITK_DEBUG << "--> " << propertyKey;
        
        image->SetProperty( propertyKey.c_str(), StringProperty::New(value) );
      }
    }
    else
    {
      MITK_WARN << "Tag " << dictIter->first << " not read as string as expected. Ignoring..." ;
    }
    ++dictIter;
  }
}
  

} // end namespace mitk

#include <mitkDicomSeriesReader.txx>

