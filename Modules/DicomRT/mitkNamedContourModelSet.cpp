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


#include "mitkNamedContourModelSet.h"

namespace mitk
{

  NamedContourModelSet::NamedContourModelSet()
  {
    roiName = "ContourModelSet";
    contourModelSet = mitk::ContourModelSet::New();
  }

  NamedContourModelSet::~NamedContourModelSet(){}

  void NamedContourModelSet::SetRoiName(char* name)
  {
    this->roiName = name;
  }

  char* NamedContourModelSet::GetRoiName()
  {
    return this->roiName;
  }

  void NamedContourModelSet::SetContourModelSet(mitk::ContourModelSet::Pointer pContourModelSet)
  {
    this->contourModelSet = pContourModelSet;
  }

  mitk::ContourModelSet::Pointer NamedContourModelSet::GetContourModelSet()
  {
    return this->contourModelSet;
  }

}
