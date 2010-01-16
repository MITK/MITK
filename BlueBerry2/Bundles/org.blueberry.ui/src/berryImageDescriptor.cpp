/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryImageDescriptor.h"

#include "tweaklets/berryImageTweaklet.h"

namespace berry {

ImageDescriptor::Pointer ImageDescriptor::CreateFromFile(const std::string& filename, const std::string& pluginid)
{
  return Tweaklets::Get(ImageTweaklet::KEY)->CreateFromFile(filename, pluginid);
}

ImageDescriptor::Pointer ImageDescriptor::CreateFromImage(void* img)
{
  return Tweaklets::Get(ImageTweaklet::KEY)->CreateFromImage(img);
}

ImageDescriptor::Pointer ImageDescriptor::GetMissingImageDescriptor()
{
  return Tweaklets::Get(ImageTweaklet::KEY)->GetMissingImageDescriptor();
}

ImageDescriptor::ImageDescriptor()
{

}

}
