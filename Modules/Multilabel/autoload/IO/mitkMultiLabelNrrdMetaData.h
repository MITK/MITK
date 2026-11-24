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

#include <set>
#include <string>
#include <vector>

namespace mitk
{
  /**
   * \brief Names that a multi-label format writes or interprets itself, so that a property of
   * such a name is no segmentation property.
   *
   * A name matches an exact name, or a prefix ending in '.' that it starts with; a name that
   * merely starts with an exact name, such as "modality.source" for "modality", is user content.
   * Property names are the NRRD header keys with '_' read as '.'.
   */
  struct MultiLabelReservedKeys
  {
    std::set<std::string> exact;
    std::vector<std::string> prefixes;

    bool Matches(const std::string& name) const;

    /** Exact names and prefixes in one list, as ItkImageIO takes its default meta data keys. */
    std::vector<std::string> GetAll() const;
  };

  /**
   * \brief The names reserved by the native multi-label NRRD format and the segmentation stack:
   * the fields their readers interpret themselves, those ITK adds on reading, the IO meta
   * information of an earlier read, and the location IOUtil records.
   */
  const MultiLabelReservedKeys& GetMultiLabelReservedKeys();

  /**
   * \brief GetMultiLabelReservedKeys() plus the keys of the layer and label definitions of the
   * legacy LabelSetImage NRRD format.
   */
  const MultiLabelReservedKeys& GetLegacyLabelSetReservedKeys();

  /**
   * \brief Extracts the segmentation properties of a multi-label NRRD header: every field whose
   * name reservedKeys does not match.
   *
   * Like ItkImageIO::ExtractMetaDataAsPropertyList, which it calls, it ensures a persistence info
   * for every property it returns, so that the next save writes it.
   */
  PropertyList::Pointer ExtractSegmentationPropertiesFromNrrdHeader(const itk::MetaDataDictionary& dictionary,
                                                                    const std::string& mimeTypeName,
                                                                    const MultiLabelReservedKeys& reservedKeys);

  /** \brief Removes every property whose name reservedKeys matches. */
  void RemoveReservedProperties(PropertyList* properties, const MultiLabelReservedKeys& reservedKeys);
}

#endif
