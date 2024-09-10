/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIQuestionFactory.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

using namespace mitk::Forms;

IQuestionFactory* IQuestionFactory::GetInstance()
{
  static auto context = us::GetModuleContext();
  static us::ServiceReference<IQuestionFactory> serviceReference;

  if (!serviceReference)
    serviceReference = context->GetServiceReference<IQuestionFactory>();

  return serviceReference
    ? context->GetService<IQuestionFactory>(serviceReference)
    : nullptr;
}

IQuestionFactory::~IQuestionFactory() = default;
