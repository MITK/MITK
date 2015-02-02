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

#include "mitkSortByImagePositionPatient.h"
#include "mitkDICOMTag.h"

mitk::SortByImagePositionPatient
::SortByImagePositionPatient(DICOMSortCriterion::Pointer secondaryCriterion)
:DICOMSortCriterion(secondaryCriterion)
{
}

mitk::SortByImagePositionPatient
::~SortByImagePositionPatient()
{
}

mitk::SortByImagePositionPatient
::SortByImagePositionPatient(const SortByImagePositionPatient& other )
:DICOMSortCriterion(other)
{
}

mitk::SortByImagePositionPatient&
mitk::SortByImagePositionPatient
::operator=(const SortByImagePositionPatient& other)
{
  if (this != &other)
  {
    DICOMSortCriterion::operator=(other);
  }
  return *this;
}

bool
mitk::SortByImagePositionPatient
::operator==(const DICOMSortCriterion& other) const
{
  return dynamic_cast<const SortByImagePositionPatient*>(&other) != NULL; // same class
}

void
mitk::SortByImagePositionPatient
::Print(std::ostream& os) const
{
  os << "(0020,0032) Image Position (Patient) along normal of (0020,0037) Image Orientation (Patient)";
}

mitk::DICOMTagList
mitk::SortByImagePositionPatient
::GetTagsOfInterest() const
{
  DICOMTagList tags;
  tags.push_back( DICOMTag(0x0020, 0x0032) ); // ImagePositionPatient
  tags.push_back( DICOMTag(0x0020, 0x0037) ); // ImageOrientationPatient

  return tags;
}

bool
mitk::SortByImagePositionPatient
::IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const
{
  bool possible(false);
  double distance = InternalNumericDistance(left, right, possible); // returns 0.0 if not possible
  if (possible)
  {
    return distance > 0.0;
  }
  else
  {
    return this->NextLevelIsLeftBeforeRight(left, right);
  }
}

double
mitk::SortByImagePositionPatient
::InternalNumericDistance(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right, bool& possible) const
{
  // sort by distance to world origin, assuming (almost) equal orientation
  static const DICOMTag tagImagePositionPatient = DICOMTag(0x0020,0x0032); // Image Position (Patient)
  static const DICOMTag    tagImageOrientation = DICOMTag(0x0020, 0x0037); // Image Orientation

  Vector3D leftRight; leftRight.Fill(0.0);
  Vector3D leftUp; leftUp.Fill(0.0);
  bool leftHasOrientation(false);
  DICOMStringToOrientationVectors( left->GetTagValueAsString( tagImageOrientation ),
                                   leftRight, leftUp, leftHasOrientation );

  Vector3D rightRight; rightRight.Fill(0.0);
  Vector3D rightUp; rightUp.Fill(0.0);
  bool rightHasOrientation(false);
  DICOMStringToOrientationVectors( right->GetTagValueAsString( tagImageOrientation ),
                                   rightRight, rightUp, rightHasOrientation );

  Point3D leftOrigin; leftOrigin.Fill(0.0f);
  bool leftHasOrigin(false);
  leftOrigin = DICOMStringToPoint3D( left->GetTagValueAsString( tagImagePositionPatient ), leftHasOrigin );

  Point3D rightOrigin; rightOrigin.Fill(0.0f);
  bool rightHasOrigin(false);
  rightOrigin = DICOMStringToPoint3D( right->GetTagValueAsString( tagImagePositionPatient ), rightHasOrigin );

  //   we tolerate very small differences in image orientation, since we got to know about
  //   acquisitions where these values change across a single series (7th decimal digit)
  //   (http://bugs.mitk.org/show_bug.cgi?id=12263)
  //   still, we want to check if our assumption of 'almost equal' orientations is valid
  for (unsigned int dim = 0; dim < 3; ++dim)
  {
    if (   fabs(leftRight[dim] - rightRight[dim]) > 0.0001
        || fabs(leftUp[dim] - rightUp[dim]) > 0.0001)
    {
      MITK_ERROR << "Dicom images have different orientations.";
      throw std::logic_error("Dicom images have different orientations. Call GetSeries() first to separate images.");
    }
  }

  Vector3D normal;
  normal[0] = leftRight[1] * leftUp[2] - leftRight[2] * leftUp[1];
  normal[1] = leftRight[2] * leftUp[0] - leftRight[0] * leftUp[2];
  normal[2] = leftRight[0] * leftUp[1] - leftRight[1] * leftUp[0];

  double leftDistance = 0.0;
  double rightDistance = 0.0;

  // this computes the distance from world origin (0,0,0) ALONG THE NORMAL of the image planes
  for (unsigned int dim = 0; dim < 3; ++dim)
  {
    leftDistance += normal[dim] * leftOrigin[dim];
    rightDistance += normal[dim] * rightOrigin[dim];
  }

  // if we can sort by just comparing the distance, we do exactly that
  if ( fabs(leftDistance - rightDistance) >= mitk::eps)
  {
    possible = true;
    // default: compare position
    return rightDistance - leftDistance; // if (left < right> ==> diff > 0
  }
  else
  {
    possible = false;
    return 0.0;
  }
}


double
mitk::SortByImagePositionPatient
::NumericDistance(const mitk::DICOMDatasetAccess* from, const mitk::DICOMDatasetAccess* to) const
{
  bool possible(false);
  double retVal = InternalNumericDistance(from, to, possible); // returns 0.0 if not possible
  return possible ? retVal : 0.0;
}
