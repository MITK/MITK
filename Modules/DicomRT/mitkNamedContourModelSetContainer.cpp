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


#include "mitkNamedContourModelSetContainer.h"

namespace mitk
{

  NamedContourModelSetContainer::NamedContourModelSet::NamedContourModelSet()
  {
    roiName = "ContourModelSet";
    contourModelSet = mitk::ContourModelSet::New();
  }

  NamedContourModelSetContainer::NamedContourModelSet::~NamedContourModelSet(){}

  void NamedContourModelSetContainer::NamedContourModelSet::SetRoiName(char* name)
  {
    this->roiName = name;
  }

  char* NamedContourModelSetContainer::NamedContourModelSet::GetRoiName()
  {
    return this->roiName;
  }

  void NamedContourModelSetContainer::NamedContourModelSet::SetContourModelSet(mitk::ContourModelSet::Pointer pContourModelSet)
  {
    this->contourModelSet = pContourModelSet;
  }

  mitk::ContourModelSet::Pointer NamedContourModelSetContainer::NamedContourModelSet::GetContourModelSet()
  {
    return this->contourModelSet;
  }

  void NamedContourModelSetContainer::SetNamedContourModelSet(mitk::ContourModelSet::Pointer modelSet, char * name)
  {
    mitk::NamedContourModelSetContainer::NamedContourModelSet::Pointer contourModelSet
        = mitk::NamedContourModelSetContainer::NamedContourModelSet::New();
    contourModelSet->SetContourModelSet(modelSet);
    contourModelSet->SetRoiName(name);
    this->namedContourModelSetVector.push_back(contourModelSet);
  }

}
