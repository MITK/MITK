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

  NamedContourModelSet::NamedContourModelSetInt::NamedContourModelSetInt()
  {
    roiName = "ContourModelSet";
    contourModelSet = mitk::ContourModelSet::New();
  }

  NamedContourModelSet::NamedContourModelSetInt::~NamedContourModelSetInt(){}

  void NamedContourModelSet::NamedContourModelSetInt::SetRoiName(char* name)
  {
    this->roiName = name;
  }

  char* NamedContourModelSet::NamedContourModelSetInt::GetRoiName()
  {
    return this->roiName;
  }

  void NamedContourModelSet::NamedContourModelSetInt::SetContourModelSet(mitk::ContourModelSet::Pointer pContourModelSet)
  {
    this->contourModelSet = pContourModelSet;
  }

  mitk::ContourModelSet::Pointer NamedContourModelSet::NamedContourModelSetInt::GetContourModelSet()
  {
    return this->contourModelSet;
  }

}
