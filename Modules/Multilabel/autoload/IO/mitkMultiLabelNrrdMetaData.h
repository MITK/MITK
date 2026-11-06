/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelNrrdMetaData_h
#define mitkMultiLabelNrrdMetaData_h

#include <mitkPropertyList.h>

#include <itkMetaDataDictionary.h>

#include <string>
#include <vector>

namespace mitk
{
  /**
   * \brief Keys of the native multi-label NRRD header that are not segmentation properties.
   *
   * These are the fields the reader interprets itself, those ITK adds on reading, and the IO
   * meta information of an earlier read. Each key matches every property name it is a prefix
   * of; property names are the header keys with '_' read as '.'.
   */
  std::vector<std::string> GetMultiLabelNrrdMetaDataKeys();

  /**
   * \brief GetMultiLabelNrrdMetaDataKeys() plus the keys of the layer and label definitions
   * of the legacy LabelSetImage NRRD format.
   */
  std::vector<std::string> GetLegacyLabelSetNrrdMetaDataKeys();

  /**
   * \brief Extracts the segmentation properties of a multi-label NRRD header: every field that
   * matches none of metaDataKeys.
   *
   * Like ItkImageIO::ExtractMetaDataAsPropertyList, which it calls, it registers a persistence
   * info for every property it returns.
   */
  PropertyList::Pointer ExtractSegmentationPropertiesFromNrrdHeader(const itk::MetaDataDictionary& dictionary,
                                                                    const std::string& mimeTypeName,
                                                                    const std::vector<std::string>& metaDataKeys);
}

#endif
