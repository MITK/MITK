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

#ifndef mitkDICOMSortByTag_h
#define mitkDICOMSortByTag_h

#include "mitkDICOMSortCriterion.h"

namespace mitk
{

/**
  \ingroup DICOMReaderModule
  \brief Compare two datasets by the value of a single tag (for use in DICOMTagBasedSorter).

  The class will compare the tag values by
   1. numerical value if possible (i.e. both datasets have a value that is numerical)
   2. alphabetical order otherwise

  If the comparison results in equalness, it is refered to the secondary criterion, see
  DICOMSortByTag::NextLevelIsLeftBeforeRight().

*/
class MITKDICOMREADER_EXPORT DICOMSortByTag : public DICOMSortCriterion
{
  public:

    mitkClassMacro( DICOMSortByTag, DICOMSortCriterion );
    mitkNewMacro1Param( DICOMSortByTag, const DICOMTag& );
    mitkNewMacro2Param( DICOMSortByTag, const DICOMTag&, DICOMSortCriterion::Pointer );

    virtual DICOMTagList GetTagsOfInterest() const override;
    virtual bool IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const override;

    virtual double NumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to) const override;

    virtual void Print(std::ostream& os) const override;

    virtual bool operator==(const DICOMSortCriterion& other) const override;

  protected:

    DICOMSortByTag( const DICOMTag& tag, DICOMSortCriterion::Pointer secondaryCriterion = nullptr );
    virtual ~DICOMSortByTag();

    DICOMSortByTag(const DICOMSortByTag& other);
    DICOMSortByTag& operator=(const DICOMSortByTag& other);

    bool StringCompare(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right, const DICOMTag& tag) const;
    bool NumericCompare(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right, const DICOMTag& tag) const;

  private:

    DICOMTag m_Tag;
};

}

#endif
