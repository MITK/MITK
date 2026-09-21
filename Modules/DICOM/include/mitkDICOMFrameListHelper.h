/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMFrameListHelper_h
#define mitkDICOMFrameListHelper_h

#include <mitkDICOMImageFrameInfo.h>
#include <MitkDICOMExports.h>

#include <string>
#include <vector>

namespace mitk
{
  /**
   * \ingroup DICOMModule
   * \brief Filenames of the passed frames in list order, consecutive duplicates
   *        collapsed, so that the frames of one multi-frame file yield one filename.
   *
   * Frames of one file are always consecutive in a block's frame list, so
   * collapsing consecutive duplicates is enough and keeps the order the sorters
   * established.
   */
  MITKDICOM_EXPORT std::vector<std::string> DistinctFilesInOrder(const DICOMImageFrameList& frames);
}

#endif
