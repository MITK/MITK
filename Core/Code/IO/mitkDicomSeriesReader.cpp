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

#include <mitkDicomSeriesReader.h>

#include <itkGDCMSeriesFileNames.h>

#if GDCM_MAJOR_VERSION >= 2
  #include <gdcmAttribute.h>
  #include <gdcmPixmapReader.h>
  #include <gdcmStringFilter.h>
#endif

namespace mitk
{

typedef itk::GDCMSeriesFileNames DcmFileNamesGeneratorType;

DataNode::Pointer DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames, UpdateCallBackMethod callback)
{
  DataNode::Pointer node = DataNode::New();

  if (DicomSeriesReader::LoadDicomSeries(filenames, *node, callback))
  {
    return node;
  }
  else
  {
    return 0;
  }
}

bool DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames, DataNode &node, UpdateCallBackMethod callback)
{
  if(filenames.size() == 0)
  {
    MITK_ERROR << "Calling LoadDicomSeries with empty filename string container. Aborting.";
    return false;
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
        DicomSeriesReader::LoadDicom<unsigned char>(filenames, node, callback);
        return true;
      case DcmIoType::CHAR:
        DicomSeriesReader::LoadDicom<char>(filenames, node, callback);
        return true;
      case DcmIoType::USHORT:
        DicomSeriesReader::LoadDicom<unsigned short>(filenames, node, callback);
        return true;
      case DcmIoType::SHORT:
        DicomSeriesReader::LoadDicom<short>(filenames, node, callback);
        return true;
      case DcmIoType::UINT:
        DicomSeriesReader::LoadDicom<unsigned int>(filenames, node, callback);
        return true;
      case DcmIoType::INT:
        DicomSeriesReader::LoadDicom<int>(filenames, node, callback);
        return true;
      case DcmIoType::ULONG:
        DicomSeriesReader::LoadDicom<long unsigned int>(filenames, node, callback);
        return true;
      case DcmIoType::LONG:
        DicomSeriesReader::LoadDicom<long int>(filenames, node, callback);
        return true;
      case DcmIoType::FLOAT:
        DicomSeriesReader::LoadDicom<float>(filenames, node, callback);
        return true;
      case DcmIoType::DOUBLE:
        DicomSeriesReader::LoadDicom<double>(filenames, node, callback);
        return true;
      default:
        MITK_ERROR << "Unknown pixel type!";
      }
    }
  }
  catch(itk::MemoryAllocationError e)
  {
    MITK_ERROR << "Memory allocation!";
  }
  catch(...)
  {
    MITK_ERROR << "Unknown!";
  }

  return false;
}


bool DicomSeriesReader::IsDicom(const std::string &filename)
{
  DcmIoType::Pointer io = DcmIoType::New();

  return io->CanReadFile(filename.c_str());
}

#if GDCM_MAJOR_VERSION >= 2
bool DicomSeriesReader::IsPhilips3DDicom(const std::string &filename)
{
  DcmIoType::Pointer io = DcmIoType::New();

  if (io->CanReadFile(filename.c_str()))
  {
    //Look at header Tag 3001,0010 if it is "Philips3D"
    gdcm::PixmapReader reader;
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

bool DicomSeriesReader::ReadPhilips3DDicom(const std::string &filename, mitk::Image::Pointer output_image)
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
  gdcm::Attribute<0x0028,0x0008> dimTagT;
  gdcm::Attribute<0x0018,0x602c> spaceTagX;
  gdcm::Attribute<0x0018,0x602e> spaceTagY;
  gdcm::Attribute<0x3001,0x1003, gdcm::VR::FD, gdcm::VM::VM1> spaceTagZ; // (Philips specific)
  gdcm::Attribute<0x0018,0x6024> physicalTagX;
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
  typedef itk::Image<UCHAR, 4> ImageType;   //Pixeltype might be different sometimes? Maybe read it out from header
  ImageType::RegionType myRegion;
  ImageType::SizeType mySize;
  ImageType::IndexType myIndex;
  ImageType::SpacingType mySpacing;
  ImageType::Pointer imageItk = ImageType::New();

  mySpacing[0] = spaceX;
  mySpacing[1] = spaceY;
  mySpacing[2] = spaceZ;
  mySpacing[3] = 1;
  myIndex[0] = physicalX;
  myIndex[1] = physicalY;
  myIndex[2] = physicalZ;
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
    iterator++;

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
  /*
  // this works as well, but then the picture is upside down..   transversal slice[max] should be transversal slice[0] and so on.
  while (!iterator.IsAtEnd())
  {
  iterator.Set( new_pixels[pixCount] );
  pixCount++;             
  iterator++;
  }                
  mitk::CastToMitkImage(imageItk, image);                    
  */        
  return true; // actually never returns false yet.. but exception possible
}
#endif

DicomSeriesReader::UidFileNamesMap DicomSeriesReader::GetSeries(const std::string &dir, bool additional_criteria, const StringContainer &restrictions)
{
  DcmFileNamesGeneratorType::Pointer name_generator = DcmFileNamesGeneratorType::New();

  name_generator->SetUseSeriesDetails(additional_criteria);
  name_generator->SetDirectory(dir.c_str());

  const StringContainer::const_iterator restrictions_end = restrictions.end();

  for(StringContainer::const_iterator it = restrictions.begin(); it != restrictions_end; ++it)
  {
    name_generator->AddSeriesRestriction(*it);
  }

  UidFileNamesMap map;
  const StringContainer &series_uids = name_generator->GetSeriesUIDs();
  const StringContainer::const_iterator series_end = series_uids.end();

  for(StringContainer::const_iterator it = series_uids.begin(); it != series_end; ++it)
  {
    const std::string &uid = *it;

    map[uid] = name_generator->GetFileNames(uid);
  }

  return map;
}

DicomSeriesReader::StringContainer DicomSeriesReader::GetSeries(const std::string &dir, const std::string &series_uid, bool additional_criteria,
    const StringContainer &restrictions)
{
  DcmFileNamesGeneratorType::Pointer name_generator = DcmFileNamesGeneratorType::New();

  name_generator->SetUseSeriesDetails(additional_criteria);
  name_generator->SetDirectory(dir.c_str());

  const StringContainer::const_iterator restrictions_end = restrictions.end();

  for(StringContainer::const_iterator it = restrictions.begin(); it != restrictions_end; ++it)
  {
    name_generator->AddSeriesRestriction(*it);
  }

  return name_generator->GetFileNames(series_uid);
}

#if GDCM_MAJOR_VERSION >= 2
bool DicomSeriesReader::GdcmSortFunction(const gdcm::DataSet &ds1, const gdcm::DataSet &ds2)
{
  gdcm::Attribute<0x0008,0x0032> acq_time1; // Acquisition time
  gdcm::Attribute<0x0020,0x0032> image_pos1; // Image Position (Patient)

  acq_time1.Set(ds1);
  image_pos1.Set(ds1);

  gdcm::Attribute<0x0008,0x0032> acq_time2;
  gdcm::Attribute<0x0020,0x0032> image_pos2;

  acq_time2.Set(ds2);
  image_pos2.Set(ds2);

  if (acq_time1 == acq_time2)
  {
    return image_pos1 < image_pos2;
  }

  return acq_time1 < acq_time2;
}
#endif

}

#include <mitkDicomSeriesReader.txx>
