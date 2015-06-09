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

#include "mitkItkLoggingAdapter.h"
#include "mitkLogMacros.h"

void mitk::ItkLoggingAdapter::Initialize()
{
   itk::OutputWindow::SetInstance(mitk::ItkLoggingAdapter::New());
}

void mitk::ItkLoggingAdapter::DisplayText(const char* s)
{
   MITK_INFO("ItkLogging") << s;
}

mitk::ItkLoggingAdapter::ItkLoggingAdapter()
{
}

mitk::ItkLoggingAdapter::~ItkLoggingAdapter()
{
}

