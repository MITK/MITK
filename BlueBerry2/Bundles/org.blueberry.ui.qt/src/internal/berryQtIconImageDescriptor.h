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


#ifndef BERRYQTICONIMAGEDESCRIPTOR_H_
#define BERRYQTICONIMAGEDESCRIPTOR_H_

#include <berryImageDescriptor.h>

class QIcon;

namespace berry {

class QtIconImageDescriptor : public ImageDescriptor
{

private:

  QIcon* icon;

public:

  QtIconImageDescriptor(void* img);

  virtual void* CreateImage(bool returnMissingImageOnError = true);

  virtual void DestroyImage(void* img);

  virtual bool operator==(const Object* o) const;

};

}

#endif /* BERRYQTICONIMAGEDESCRIPTOR_H_ */
