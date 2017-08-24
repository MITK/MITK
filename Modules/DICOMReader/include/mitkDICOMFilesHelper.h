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

#ifndef MITKDICOMFILESHELPER_H
#define MITKDICOMFILESHELPER_H

#include <string>
#include <vector>

#include <MitkDICOMReaderExports.h>

namespace mitk {

typedef std::vector<std::string> DICOMFilePathList;

/** Helper functions. Searches for all files in the directory of the passed file path.
  All files will be checked if they are DICOM files. All DICOM files will be added to the result and
  returned.
  @remark The helper does no sorting of any kind.*/
DICOMFilePathList GetDICOMFilesInSameDirectory(const std::string& filePath);

/** All passed files will be checked if they are DICOM files.
All DICOM files will be added to the result and returned.
@remark The helper does no sorting of any kind.*/
DICOMFilePathList FilterForDICOMFiles(const DICOMFilePathList& fileList);
}

#endif // MITKDICOMFILESHELPER_H
