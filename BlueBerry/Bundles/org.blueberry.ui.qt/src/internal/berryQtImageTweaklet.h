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


#ifndef BERRYQTIMAGETWEAKLET_H_
#define BERRYQTIMAGETWEAKLET_H_

#include <berryImageTweaklet.h>

namespace berry {

class QtImageTweaklet : public QObject, public ImageTweaklet
{
  Q_OBJECT
  Q_INTERFACES(berry::ImageTweaklet)

public:

  QtImageTweaklet();
  QtImageTweaklet(const QtImageTweaklet& other);

  virtual SmartPointer<ImageDescriptor> CreateFromFile(const std::string& filename, const std::string& pluginid);
  virtual SmartPointer<ImageDescriptor> CreateFromImage(void* img);
  virtual SmartPointer<ImageDescriptor> GetMissingImageDescriptor();

  virtual void DestroyImage(const void* img);

};

}

#endif /* BERRYQTIMAGETWEAKLET_H_ */
