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

#include "mitkIModelFitProvider.h"

namespace mitk
{
  IModelFitProvider::~IModelFitProvider() {}
  std::string mitk::IModelFitProvider::PROP_DESCRIPTION()
  {
    static std::string s = "org.mitk.IModelFitProvider.description";
    return s;
  }

  std::string mitk::IModelFitProvider::PROP_MODEL_CLASS_ID()
  {
    static std::string s = "org.mitk.IModelFitProvider.classid";
    return s;
  }

  std::string mitk::IModelFitProvider::PROP_MODEL_TYPE()
  {
    static std::string s = "org.mitk.IModelFitProvider.modeltype";
    return s;
  }
}