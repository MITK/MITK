/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIQuestionWidgetFactory.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

using namespace mitk::Forms::UI;

IQuestionWidgetFactory* IQuestionWidgetFactory::GetInstance()
{
  static auto context = us::GetModuleContext();
  static us::ServiceReference<IQuestionWidgetFactory> serviceReference;

  if (!serviceReference)
    serviceReference = context->GetServiceReference<IQuestionWidgetFactory>();

  return serviceReference
    ? context->GetService<IQuestionWidgetFactory>(serviceReference)
    : nullptr;
}

IQuestionWidgetFactory::~IQuestionWidgetFactory() = default;
