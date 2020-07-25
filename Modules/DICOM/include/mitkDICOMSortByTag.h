/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSortByTag_h
#define mitkDICOMSortByTag_h

#include "mitkDICOMSortCriterion.h"

namespace mitk
{

/**
  \ingroup DICOMModule
  \brief Compare two datasets by the value of a single tag (for use in DICOMTagBasedSorter).

  The class will compare the tag values by
   1. numerical value if possible (i.e. both datasets have a value that is numerical)
   2. alphabetical order otherwise

  If the comparison results in equalness, it is refered to the secondary criterion, see
  DICOMSortByTag::NextLevelIsLeftBeforeRight().

*/
class MITKDICOM_EXPORT DICOMSortByTag : public DICOMSortCriterion
{
  public:

    mitkClassMacro( DICOMSortByTag, DICOMSortCriterion );
    mitkNewMacro1Param( DICOMSortByTag, const DICOMTag& );
    mitkNewMacro2Param( DICOMSortByTag, const DICOMTag&, DICOMSortCriterion::Pointer );

    DICOMTagList GetTagsOfInterest() const override;
    bool IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const override;

    double NumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to) const override;

    void Print(std::ostream& os) const override;

    bool operator==(const DICOMSortCriterion& other) const override;

  protected:

    DICOMSortByTag( const DICOMTag& tag, DICOMSortCriterion::Pointer secondaryCriterion = nullptr );
    ~DICOMSortByTag() override;

    DICOMSortByTag(const DICOMSortByTag& other);
    DICOMSortByTag& operator=(const DICOMSortByTag& other);

    bool StringCompare(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right, const DICOMTag& tag) const;
    bool NumericCompare(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right, const DICOMTag& tag) const;

  private:

    DICOMTag m_Tag;
};

}

#endif
