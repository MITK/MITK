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

namespace mitk
{

typedef itk::GDCMSeriesFileNames DcmFileNamesGeneratorType;

DataNode::Pointer DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames)
{
  DataNode::Pointer node = DataNode::New();

  if (DicomSeriesReader::LoadDicomSeries(filenames, *node))
  {
    return node;
  }
  else
  {
    return 0;
  }
}

bool DicomSeriesReader::LoadDicomSeries(const StringContainer &filenames, DataNode &node)
{
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
        DicomSeriesReader::LoadDicom<unsigned char>(filenames, node);
        return true;
      case DcmIoType::CHAR:
        DicomSeriesReader::LoadDicom<char>(filenames, node);
        return true;
      case DcmIoType::USHORT:
        DicomSeriesReader::LoadDicom<unsigned short>(filenames, node);
        return true;
      case DcmIoType::SHORT:
        DicomSeriesReader::LoadDicom<short>(filenames, node);
        return true;
      case DcmIoType::UINT:
        DicomSeriesReader::LoadDicom<unsigned int>(filenames, node);
        return true;
      case DcmIoType::INT:
        DicomSeriesReader::LoadDicom<int>(filenames, node);
        return true;
      case DcmIoType::ULONG:
        DicomSeriesReader::LoadDicom<long unsigned int>(filenames, node);
        return true;
      case DcmIoType::LONG:
        DicomSeriesReader::LoadDicom<long int>(filenames, node);
        return true;
      case DcmIoType::FLOAT:
        DicomSeriesReader::LoadDicom<float>(filenames, node);
        return true;
      case DcmIoType::DOUBLE:
        DicomSeriesReader::LoadDicom<double>(filenames, node);
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

}

