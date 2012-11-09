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

#include "mitkVtkLoggingAdapter.h"
#include "mitkLogMacros.h"
#include <vtkObjectFactory.h>

namespace mitk
{
  vtkStandardNewMacro(VtkLoggingAdapter);
}

void mitk::VtkLoggingAdapter::Initialize()
{
  mitk::VtkLoggingAdapter* vtklog = mitk::VtkLoggingAdapter::New();
  vtkOutputWindow::SetInstance(vtklog);
  vtklog->Delete();
}

void mitk::VtkLoggingAdapter::DisplayText(const char* t)
{
  MITK_INFO("VtkText") << t;
}

void mitk::VtkLoggingAdapter::DisplayErrorText(const char *t)
{
  MITK_ERROR("VtkError") << t;
}

void mitk::VtkLoggingAdapter::DisplayWarningText(const char *t)
{
  MITK_WARN("VtkWarning") << t;
}

void mitk::VtkLoggingAdapter::DisplayGenericWarningText(const char *t)
{
  MITK_WARN("VtkGenericWarning") << t;
}

void mitk::VtkLoggingAdapter::DisplayDebugText(const char *t)
{
  MITK_DEBUG("VtkDebug") << t;
}
