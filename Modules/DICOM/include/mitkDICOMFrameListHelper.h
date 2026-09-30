/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMFrameListHelper_h
#define mitkDICOMFrameListHelper_h

#include <mitkDICOMFileReader.h>
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

  /**
   * \ingroup DICOMModule
   * \brief Whether one of the passed frames belongs to the passed file.
   *
   * The paths are compared as filesystem paths, lexically and without
   * resolving them, so that separator spelling does not matter.
   */
  MITKDICOM_EXPORT bool ContainsFile(const DICOMImageFrameList& frames, const std::string& file);

  /**
   * \ingroup DICOMModule
   * \brief Set the passed files as the reader's input and analyze them with
   *        the frame model.
   *
   * The files are scanned by a DICOMDCMTKTagScanner that reads the frame model,
   * for the tags of interest the reader has at the time of the call, and the
   * scan is handed to the reader as its tag cache. A reader that scans for
   * itself reads every file as a single frame.
   *
   * \return The tag cache the analysis ran on.
   */
  MITKDICOM_EXPORT DICOMTagCache::Pointer AnalyzeWithFrameModel(DICOMFileReader& reader, const StringList& files);
}

#endif
