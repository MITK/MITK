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

mitk::DICOMFilePathList mitk::GetDICOMFilesInSameDirectory(const std::string& filePath)
{
  DICOMFilePathList result;

  if (!filePath.empty())
  {
    std::string dir = itksys::SystemTools::GetFilenamePath(filePath);

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
  for (auto aFile : fileList)
  {
    if (io->CanReadFile(aFile.c_str()))
    {
      result.push_back(aFile);
    }
  }

  return result;
};
