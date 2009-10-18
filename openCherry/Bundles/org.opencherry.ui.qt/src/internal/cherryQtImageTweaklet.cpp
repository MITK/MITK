/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryQtImageTweaklet.h"

#include <cherryImageDescriptor.h>
#include "cherryQtFileImageDescriptor.h"
#include "cherryQtIconImageDescriptor.h"
#include "cherryQtMissingImageDescriptor.h"

namespace cherry {

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
