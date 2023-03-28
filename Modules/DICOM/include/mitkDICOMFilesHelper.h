/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMFilesHelper_h
#define mitkDICOMFilesHelper_h

#include <string>
#include <vector>

#include <MitkDICOMExports.h>

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

/** Returns all DICOM files passed with fileList that have the same series instance UID then the passed refFilePath.
@pre refFilePath must point to a valid DICOM file.*/
DICOMFilePathList FilterDICOMFilesForSameSeries(const std::string& refFilePath, const DICOMFilePathList& fileList);
}

#endif
