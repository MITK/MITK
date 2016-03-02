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
