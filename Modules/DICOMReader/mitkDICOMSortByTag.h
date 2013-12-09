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

class DICOMReader_EXPORT DICOMSortByTag : public DICOMSortCriterion
{
  public:

    mitkClassMacro( DICOMSortByTag, DICOMSortCriterion );
    mitkNewMacro1Param( DICOMSortByTag, const DICOMTag& );
    mitkNewMacro2Param( DICOMSortByTag, const DICOMTag&, DICOMSortCriterion::Pointer );

    virtual DICOMTagList GetTagsOfInterest() const;
    virtual bool IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const;

  protected:

    DICOMSortByTag( const DICOMTag& tag, DICOMSortCriterion::Pointer secondaryCriterion = NULL );
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
