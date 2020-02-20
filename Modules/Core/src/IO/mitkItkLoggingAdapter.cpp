/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkItkLoggingAdapter.h"
#include "mitkLogMacros.h"

void mitk::ItkLoggingAdapter::Initialize()
{
  itk::OutputWindow::SetInstance(mitk::ItkLoggingAdapter::New());
}

void mitk::ItkLoggingAdapter::DisplayText(const char *s)
{
  MITK_INFO("ItkLogging") << s;
}

mitk::ItkLoggingAdapter::ItkLoggingAdapter()
{
}

mitk::ItkLoggingAdapter::~ItkLoggingAdapter()
{
}
