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


#ifndef BERRYIMAGETWEAKLET_H_
#define BERRYIMAGETWEAKLET_H_

#include "../berryUiDll.h"
#include "../internal/berryTweaklets.h"
#include "../berryImageDescriptor.h"

#include <iostream>

namespace berry
{

/**
 * Provides the set of cursors used for drag-and-drop.
 */
struct BERRY_UI ImageTweaklet : public Object
{

  berryInterfaceMacro(ImageTweaklet, berry);

  static Tweaklets::TweakKey<ImageTweaklet> KEY;

  virtual SmartPointer<ImageDescriptor> CreateFromFile(const std::string& filename, const std::string& pluginid) = 0;
  virtual SmartPointer<ImageDescriptor> CreateFromImage(void* img) = 0;
  virtual SmartPointer<ImageDescriptor> GetMissingImageDescriptor() = 0;

  virtual void DestroyImage(const void* img) = 0;
};

}

#endif /* BERRYIMAGETWEAKLET_H_ */
