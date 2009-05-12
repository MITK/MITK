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


#ifndef CHERRYIMAGETWEAKLET_H_
#define CHERRYIMAGETWEAKLET_H_

#include "../cherryUiDll.h"
#include "../internal/cherryTweaklets.h"
#include "../cherryImageDescriptor.h"

#include <iostream>

namespace cherry
{

/**
 * Provides the set of cursors used for drag-and-drop.
 */
struct CHERRY_UI ImageTweaklet : public Object
{

  cherryInterfaceMacro(ImageTweaklet, cherry);

  static Tweaklets::TweakKey<ImageTweaklet> KEY;

  virtual SmartPointer<ImageDescriptor> CreateFromFile(const std::string& filename, const std::string& pluginid) = 0;
  virtual SmartPointer<ImageDescriptor> CreateFromImage(void* img) = 0;
  virtual SmartPointer<ImageDescriptor> GetMissingImageDescriptor() = 0;

  virtual void DestroyImage(const void* img) = 0;
};

}

#endif /* CHERRYIMAGETWEAKLET_H_ */
