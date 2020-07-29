/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSortCriterion_h
#define mitkDICOMSortCriterion_h

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "mitkDICOMDatasetAccess.h"

namespace mitk
{

/**
  \ingroup DICOMModule
  \brief A tag based sorting criterion for use in DICOMTagBasedSorter.

  This class is used within std::sort (see DICOMTagBasedSorter::Sort())
  and has to answer a simple question by implementing IsLeftBeforeRight().
  Each time IsLeftBeforeRight() is called, the method should return whether
  the left dataset should be sorted before the right dataset.

  Because there are identical tags values quite oftenly, a DICOMSortCriterion
  will always hold a secondary DICOMSortCriterion. In cases of equal tag
  values, the decision is refered to the secondary criterion.
*/
class MITKDICOM_EXPORT DICOMSortCriterion : public itk::LightObject
{
  public:

    mitkClassMacroItkParent( DICOMSortCriterion, itk::LightObject );

    /// \brief Tags used for comparison (includes seconary criteria).
    DICOMTagList GetAllTagsOfInterest() const;
    /// \brief Tags used for comparison.
    virtual DICOMTagList GetTagsOfInterest() const = 0;

    /// \brief Answer the sorting question.
    virtual bool IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const = 0;

    /// \brief Calculate a distance between two datasets.
    /// This ansers the question of consecutive datasets.
    virtual double NumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to) const = 0;

    /// \brief The fallback criterion.
    DICOMSortCriterion::ConstPointer GetSecondaryCriterion() const;

    /// brief describe this class in given stream.
    virtual void Print(std::ostream& os) const = 0;

    virtual bool operator==(const DICOMSortCriterion& other) const = 0;

  protected:

    DICOMSortCriterion( DICOMSortCriterion::Pointer secondaryCriterion );
    ~DICOMSortCriterion() override;

    bool NextLevelIsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const;

    explicit DICOMSortCriterion(const DICOMSortCriterion& other);
    DICOMSortCriterion& operator=(const DICOMSortCriterion& other);

    DICOMSortCriterion::Pointer m_SecondaryCriterion;
};

}

#endif
