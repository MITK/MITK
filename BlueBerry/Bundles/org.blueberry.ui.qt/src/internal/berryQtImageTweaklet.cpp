/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtImageTweaklet.h"

#include <berryImageDescriptor.h>
#include "berryQtFileImageDescriptor.h"
#include "berryQtIconImageDescriptor.h"
#include "berryQtMissingImageDescriptor.h"

namespace berry {

QtImageTweaklet::QtImageTweaklet()
{

}

QtImageTweaklet::QtImageTweaklet(const QtImageTweaklet& other)
{
  Q_UNUSED(other)
}

SmartPointer<ImageDescriptor> QtImageTweaklet::CreateFromFile(const std::string& filename, const std::string& pluginid)
{
  ImageDescriptor::Pointer descriptor(new QtFileImageDescriptor(filename, pluginid));
  return descriptor;
}

SmartPointer<ImageDescriptor> QtImageTweaklet::CreateFromImage(void* img)
{
  ImageDescriptor::Pointer descriptor(new QtIconImageDescriptor(img));
  return descriptor;
}

SmartPointer<ImageDescriptor> QtImageTweaklet::GetMissingImageDescriptor()
{
  SmartPointer<QtMissingImageDescriptor> descriptor(new QtMissingImageDescriptor());
  return descriptor;
}

void QtImageTweaklet::DestroyImage(const void*  /*img*/)
{

}

}
