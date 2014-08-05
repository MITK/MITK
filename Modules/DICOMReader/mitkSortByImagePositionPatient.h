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

/**
  \ingroup DICOMReaderModule
  \brief Sort by distance of image origin along image normal (for use in DICOMTagBasedSorter).

  To compare two datasets, their distance to the world origin is calculated.
  This distance is calculated along the image normals because we do not know
  the image orientation in advance, to any of the three coordinates could be identical for all datasets.

  \note This class assumes that the datasets have identical orientations!
*/
class MitkDICOMReader_EXPORT SortByImagePositionPatient : public DICOMSortCriterion
{
  public:

    mitkClassMacro( SortByImagePositionPatient, DICOMSortCriterion );
    mitkNewMacro1Param( SortByImagePositionPatient, DICOMSortCriterion::Pointer );

    virtual DICOMTagList GetTagsOfInterest() const;
    virtual bool IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const;

    virtual double NumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to) const;

    virtual void Print(std::ostream& os) const;

    virtual bool operator==(const DICOMSortCriterion& other) const;

  protected:

    SortByImagePositionPatient( DICOMSortCriterion::Pointer secondaryCriterion = NULL );
    virtual ~SortByImagePositionPatient();

    SortByImagePositionPatient(const SortByImagePositionPatient& other);
    SortByImagePositionPatient& operator=(const SortByImagePositionPatient& other);

    double InternalNumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to, bool& possible) const;

  private:
};

}

#endif
