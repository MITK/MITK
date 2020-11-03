/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSortByImagePositionPatient_h
#define mitkSortByImagePositionPatient_h

#include "mitkDICOMSortCriterion.h"

#include "mitkVector.h"

namespace mitk
{

/**
  \ingroup DICOMModule
  \brief Sort by distance of image origin along image normal (for use in DICOMTagBasedSorter).

  To compare two datasets, their distance to the world origin is calculated.
  This distance is calculated along the image normals because we do not know
  the image orientation in advance, to any of the three coordinates could be identical for all datasets.

  \note This class assumes that the datasets have identical orientations!
*/
class MITKDICOM_EXPORT SortByImagePositionPatient : public DICOMSortCriterion
{
  public:

    mitkClassMacro( SortByImagePositionPatient, DICOMSortCriterion );
    mitkNewMacro1Param( SortByImagePositionPatient, DICOMSortCriterion::Pointer );

    DICOMTagList GetTagsOfInterest() const override;
    bool IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const override;

    double NumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to) const override;

    void Print(std::ostream& os) const override;

    bool operator==(const DICOMSortCriterion& other) const override;

  protected:

    SortByImagePositionPatient( DICOMSortCriterion::Pointer secondaryCriterion = nullptr );
    ~SortByImagePositionPatient() override;

    SortByImagePositionPatient(const SortByImagePositionPatient& other);
    SortByImagePositionPatient& operator=(const SortByImagePositionPatient& other);

    double InternalNumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to, bool& possible) const;

  private:
};

}

#endif
