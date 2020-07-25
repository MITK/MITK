/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMFilenameSorter_h
#define mitkDICOMFilenameSorter_h

#include "mitkDICOMDatasetSorter.h"

namespace mitk
{

/**
  \ingroup DICOMModule
  \brief sort files based on filename (last resort).
*/
class DICOMFilenameSorter : public DICOMDatasetSorter
{
  public:

    mitkClassMacro( DICOMFilenameSorter, DICOMDatasetSorter );

    itkNewMacro( DICOMFilenameSorter );

    DICOMTagList GetTagsOfInterest() override;

    void Sort() override;

    void PrintConfiguration(std::ostream& os, const std::string& indent = "") const override;

    bool operator==(const DICOMDatasetSorter& other) const override;

  protected:

    struct FilenameSort
    {
      bool operator() (const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right);
    };

    DICOMFilenameSorter();
    ~DICOMFilenameSorter() override;

    DICOMFilenameSorter(const DICOMFilenameSorter& other);
    DICOMFilenameSorter& operator=(const DICOMFilenameSorter& other);
};

}

#endif
