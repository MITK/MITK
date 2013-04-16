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


#ifndef BERRYFILEIMAGEDESCRIPTOR_H_
#define BERRYFILEIMAGEDESCRIPTOR_H_

#include "berryImageDescriptor.h"

#include <iostream>

class QIcon;

namespace berry {

class QtFileImageDescriptor : public ImageDescriptor
{

private:

  const std::string filename;
  const std::string pluginid;

  QIcon* CreateFromStream(std::istream* s);

public:

  QtFileImageDescriptor(const std::string& filename, const std::string& pluginid);

  virtual void* CreateImage(bool returnMissingImageOnError = true);

  virtual void DestroyImage(void* img);

  bool operator ==(const Object* o) const;
};

}

#endif /* BERRYFILEIMAGEDESCRIPTOR_H_ */
