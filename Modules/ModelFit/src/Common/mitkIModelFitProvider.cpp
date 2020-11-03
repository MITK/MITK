/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
