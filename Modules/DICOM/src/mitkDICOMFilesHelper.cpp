/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMFilesHelper.h"

#include <itkGDCMImageIO.h>
#include <itksys/SystemTools.hxx>
#include <gdcmDirectory.h>
#include <gdcmScanner.h>

mitk::DICOMFilePathList mitk::GetDICOMFilesInSameDirectory(const std::string& filePath)
{
  DICOMFilePathList result;

  if (!filePath.empty())
  {

    std::string dir = filePath;
    if (!itksys::SystemTools::FileIsDirectory(filePath))
    {
      dir = itksys::SystemTools::GetFilenamePath(filePath);
    }

    gdcm::Directory directoryLister;
    directoryLister.Load(dir.c_str(), false); // non-recursive
    result = FilterForDICOMFiles(directoryLister.GetFilenames());
  }

  return result;
};

mitk::DICOMFilePathList mitk::FilterForDICOMFiles(const DICOMFilePathList& fileList)
{
  mitk::DICOMFilePathList result;

  itk::GDCMImageIO::Pointer io = itk::GDCMImageIO::New();
  for (const auto &aFile : fileList)
  {
    if (io->CanReadFile(aFile.c_str()))
    {
      result.push_back(aFile);
    }
  }

  return result;
};

mitk::DICOMFilePathList mitk::FilterDICOMFilesForSameSeries(const std::string& refFilePath, const DICOMFilePathList& fileList)
{
  auto dicomFiles = FilterForDICOMFiles(fileList);

  //use the gdcm scanner directly instead of our wrapping classes, as it is a very simple task
  //and I want to spare the indirection/overhead.
  gdcm::Scanner scanner;

  const gdcm::Tag seriesInstanceUIDTag(0x0020, 0x000e);

  scanner.AddTag(seriesInstanceUIDTag); //Series Instance UID;

  scanner.Scan({ refFilePath });
  auto uid = scanner.GetValue(refFilePath.c_str(), seriesInstanceUIDTag);
  
  if (uid != nullptr)
  {
    const std::string refUID = uid;
    scanner.Scan(dicomFiles);
    return scanner.GetAllFilenamesFromTagToValue(seriesInstanceUIDTag, refUID.c_str());
  }

  return mitk::DICOMFilePathList();
}
