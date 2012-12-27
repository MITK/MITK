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


#ifndef BERRYIMAGETWEAKLET_H_
#define BERRYIMAGETWEAKLET_H_

#include <org_blueberry_ui_Export.h>
#include "internal/berryTweaklets.h"
#include "berryImageDescriptor.h"

#include <iostream>

namespace berry
{

/**
 * Provides the set of cursors used for drag-and-drop.
 */
struct BERRY_UI ImageTweaklet
{
  static Tweaklets::TweakKey<ImageTweaklet> KEY;

  virtual SmartPointer<ImageDescriptor> CreateFromFile(const std::string& filename, const std::string& pluginid) = 0;
  virtual SmartPointer<ImageDescriptor> CreateFromImage(void* img) = 0;
  virtual SmartPointer<ImageDescriptor> GetMissingImageDescriptor() = 0;

  virtual void DestroyImage(const void* img) = 0;
};

}

Q_DECLARE_INTERFACE(berry::ImageTweaklet, "org.blueberry.ImageTweaklet")

#endif /* BERRYIMAGETWEAKLET_H_ */
