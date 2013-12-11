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

#ifndef mitkSortByImagePositionPatient_h
#define mitkSortByImagePositionPatient_h

#include "mitkDICOMSortCriterion.h"

#include "mitkVector.h"

namespace mitk
{

class DICOMReader_EXPORT SortByImagePositionPatient : public DICOMSortCriterion
{
  public:

    mitkClassMacro( SortByImagePositionPatient, DICOMSortCriterion );
    mitkNewMacro1Param( SortByImagePositionPatient, DICOMSortCriterion::Pointer );

    virtual DICOMTagList GetTagsOfInterest() const;
    virtual bool IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const;

  protected:

    SortByImagePositionPatient( DICOMSortCriterion::Pointer secondaryCriterion = NULL );
    virtual ~SortByImagePositionPatient();

    SortByImagePositionPatient(const SortByImagePositionPatient& other);
    SortByImagePositionPatient& operator=(const SortByImagePositionPatient& other);

    mitk::Point3D DICOMStringToPoint3D(const std::string& s, bool& successful) const;
    void DICOMStringToOrientationVectors(const std::string& s, Vector3D& right, Vector3D& up, bool& successful) const;

  private:
};

}

#endif
