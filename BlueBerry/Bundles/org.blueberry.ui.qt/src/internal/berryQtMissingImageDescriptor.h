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


#ifndef BERRYQTMISSINGIMAGEDESCRIPTOR_H_
#define BERRYQTMISSINGIMAGEDESCRIPTOR_H_

#include <berryImageDescriptor.h>

class QIcon;

namespace berry {

class QtMissingImageDescriptor : public ImageDescriptor
{

private:

public:

  virtual void* CreateImage(bool returnMissingImageOnError = true);

  virtual void DestroyImage(void* img);

  virtual bool operator==(const Object* o) const;

};

}

#endif /* BERRYQTICONIMAGEDESCRIPTOR_H_ */
