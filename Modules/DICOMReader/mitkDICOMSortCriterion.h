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

#ifndef mitkDICOMSortCriterion_h
#define mitkDICOMSortCriterion_h

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "mitkDICOMDatasetAccess.h"

namespace mitk
{

class DICOMReader_EXPORT DICOMSortCriterion : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMSortCriterion, itk::LightObject );

    DICOMTagList GetAllTagsOfInterest() const;
    virtual DICOMTagList GetTagsOfInterest() const = 0;

    virtual bool IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const = 0;

  protected:

    DICOMSortCriterion( DICOMSortCriterion::Pointer secondaryCriterion );
    virtual ~DICOMSortCriterion();

    bool NextLevelIsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const;

    DICOMSortCriterion(const DICOMSortCriterion& other);
    DICOMSortCriterion& operator=(const DICOMSortCriterion& other);

    DICOMSortCriterion::Pointer m_SecondaryCriterion;
};

}

#endif
