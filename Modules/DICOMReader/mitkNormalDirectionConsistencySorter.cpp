/*=================================================================== The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

//#define MBILOG_ENABLE_DEBUG

#include "mitkNormalDirectionConsistencySorter.h"

#include <algorithm>

mitk::NormalDirectionConsistencySorter
::NormalDirectionConsistencySorter()
:DICOMDatasetSorter()
{
}

mitk::NormalDirectionConsistencySorter
::NormalDirectionConsistencySorter(const NormalDirectionConsistencySorter& other )
:DICOMDatasetSorter(other)
{
}

mitk::NormalDirectionConsistencySorter
::~NormalDirectionConsistencySorter()
{
}

void
mitk::NormalDirectionConsistencySorter
::PrintConfiguration(std::ostream& os, const std::string& indent) const
{
  os << indent << "NormalDirectionConsistencySorter TODO" << std::endl;
}


mitk::NormalDirectionConsistencySorter&
mitk::NormalDirectionConsistencySorter
::operator=(const NormalDirectionConsistencySorter& other)
{
  if (this != &other)
  {
    DICOMDatasetSorter::operator=(other);
  }
  return *this;
}

mitk::DICOMTagList
mitk::NormalDirectionConsistencySorter
::GetTagsOfInterest()
{
  DICOMTagList tags;
  tags.push_back( DICOMTag(0x0020, 0x0032) ); // ImagePositionPatient
  tags.push_back( DICOMTag(0x0020, 0x0037) ); // ImageOrientationPatient

  return tags;
}

void
mitk::NormalDirectionConsistencySorter
::Sort()
{
  DICOMDatasetList datasets = GetInput();

  if (datasets.size() > 1)
  {
    // at some point in the code, there is the expectation that
    // the direction of the slice normals is the same as the direction between
    // first and last slice origin. We need to make this sure here, because
    // we want to feed the files into itk::ImageSeriesReader with the consistent
    // setting of ReverseOrderOff.

    static const DICOMTag tagImagePositionPatient = DICOMTag(0x0020,0x0032); // Image Position (Patient)
    static const DICOMTag    tagImageOrientation = DICOMTag(0x0020, 0x0037); // Image Orientation

    DICOMDatasetAccess* firstDS = datasets.front();
    DICOMDatasetAccess*  lastDS = datasets.back();

    // make sure here that the direction from slice to slice is the direction of
    // image normals...
    std::string imageOrientationString = firstDS->GetTagValueAsString( tagImageOrientation );
    std::string imagePositionPatientFirst = firstDS->GetTagValueAsString( tagImagePositionPatient );
    std::string imagePositionPatientLast = lastDS->GetTagValueAsString( tagImagePositionPatient );

    static Vector3D right; right.Fill(0.0);
    static Vector3D up; up.Fill(0.0);
    static bool hasOrientation(false);
    DICOMStringToOrientationVectors( imageOrientationString,
        right, up, hasOrientation );

    static Point3D firstOrigin; firstOrigin.Fill(0.0f);
    static bool firstHasOrigin(false);
    firstOrigin = DICOMStringToPoint3D( imagePositionPatientFirst, firstHasOrigin );

    static Point3D lastOrigin; lastOrigin.Fill(0.0f);
    static bool lastHasOrigin(false);
    lastOrigin = DICOMStringToPoint3D( imagePositionPatientLast, lastHasOrigin );

    static Vector3D normal;
    normal[0] = right[1] * up[2] - right[2] * up[1];
    normal[1] = right[2] * up[0] - right[0] * up[2];
    normal[2] = right[0] * up[1] - right[1] * up[0];
    normal.Normalize();

    static Vector3D directionOfSlices;
    directionOfSlices = lastOrigin - firstOrigin;
    directionOfSlices.Normalize();

    MITK_DEBUG << "Making sense of \norientation '" << imageOrientationString
      << "'\nfirst position '" << imagePositionPatientFirst
      << "'\nlast position '" << imagePositionPatientLast << "'";
    MITK_DEBUG << "Normal: " << normal;
    MITK_DEBUG << "Direction of slices: " << directionOfSlices;

    if ( (directionOfSlices - normal).GetNorm() > 0.5 )
    {
      MITK_DEBUG << "Need to reverse filenames";
      std::reverse( datasets.begin(), datasets.end() );
    }
  }

  this->SetNumberOfOutputs(1);
  this->SetOutput(0, datasets);
}
