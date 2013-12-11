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

mitk::DICOMTagList
mitk::SortByImagePositionPatient
::GetTagsOfInterest() const
{
  DICOMTagList tags;
  tags.push_back( DICOMTag(0x0020, 0x0032) ); // ImagePositionPatient
  tags.push_back( DICOMTag(0x0020, 0x0037) ); // ImageOrientationPatient

  return tags;
}

mitk::Point3D
mitk::SortByImagePositionPatient
::DICOMStringToPoint3D(const std::string& s, bool& successful) const
{
  Point3D p;
  successful = true;

  std::istringstream originReader(s);
  std::string coordinate;
  unsigned int dim(0);
  while( std::getline( originReader, coordinate, '\\' ) && dim < 3)
  {
    p[dim++]= atof(coordinate.c_str());
  }

  if (dim && dim != 3)
  {
    successful = false;
    MITK_ERROR << "Reader implementation made wrong assumption on tag (0020,0032). Found " << dim << " instead of 3 values.";
  }
  else if (dim == 0)
  {
    successful = false;
    p.Fill(0.0); // assume default (0,0,0)
  }

  return p;
}


void
mitk::SortByImagePositionPatient
::DICOMStringToOrientationVectors(const std::string& s, Vector3D& right, Vector3D& up, bool& successful) const
{
  successful = true;

  std::istringstream orientationReader(s);
  std::string coordinate;
  unsigned int dim(0);
  while( std::getline( orientationReader, coordinate, '\\' ) && dim < 6 )
  {
    if (dim<3)
    {
      right[dim++] = atof(coordinate.c_str());
    }
    else
    {
      up[dim++ - 3] = atof(coordinate.c_str());
    }
  }

  if (dim && dim != 6)
  {
    successful = false;
    MITK_ERROR << "Reader implementation made wrong assumption on tag (0020,0037). Found " << dim << " instead of 6 values.";
  }
  else if (dim == 0)
  {
    // fill with defaults
    right.Fill(0.0);
    right[0] = 1.0;

    up.Fill(0.0);
    up[1] = 1.0;

    successful = false;
  }
}


bool
mitk::SortByImagePositionPatient
::IsLeftBeforeRight(const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right) const
{
  // sort by distance to world origin, assuming (almost) equal orientation
  static const DICOMTag tagImagePositionPatient = DICOMTag(0x0020,0x0032); // Image Position (Patient)
  static const DICOMTag    tagImageOrientation = DICOMTag(0x0020, 0x0037); // Image Orientation

  static Vector3D leftRight; leftRight.Fill(0.0);
  static Vector3D leftUp; leftUp.Fill(0.0);
  static bool leftHasOrientation(false);
  DICOMStringToOrientationVectors( left->GetTagValueAsString( tagImageOrientation ),
                                   leftRight, leftUp, leftHasOrientation );

  static Vector3D rightRight; rightRight.Fill(0.0);
  static Vector3D rightUp; rightUp.Fill(0.0);
  static bool rightHasOrientation(false);
  DICOMStringToOrientationVectors( right->GetTagValueAsString( tagImageOrientation ),
                                   rightRight, rightUp, rightHasOrientation );

  static Point3D leftOrigin; leftOrigin.Fill(0.0f);
  static bool leftHasOrigin(false);
  leftOrigin = DICOMStringToPoint3D( left->GetTagValueAsString( tagImagePositionPatient ), leftHasOrigin );

  static Point3D rightOrigin; rightOrigin.Fill(0.0f);
  static bool rightHasOrigin(false);
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

  static Vector3D normal;
  normal[0] = leftRight[1] * leftUp[5] - leftRight[2] * leftUp[4];
  normal[1] = leftRight[2] * leftUp[3] - leftRight[0] * leftUp[5];
  normal[2] = leftRight[0] * leftUp[4] - leftRight[1] * leftUp[3];

  static double leftDistance = 0.0;
  static double rightDistance = 0.0;

  // this computes the distance from world origin (0,0,0) ALONG THE NORMAL of the image planes
  for (unsigned int dim = 0; dim < 3; ++dim)
  {
    leftDistance += normal[dim] * leftOrigin[dim];
    rightDistance += normal[dim] * rightOrigin[dim];
  }

  // if we can sort by just comparing the distance, we do exactly that
  if ( fabs(leftDistance - rightDistance) >= mitk::eps)
  {
    // default: compare position
    return leftDistance < rightDistance;
  }
  else
  {
    return this->NextLevelIsLeftBeforeRight(left, right);
  }
}
