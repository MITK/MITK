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


#ifndef CHERRYQTICONIMAGEDESCRIPTOR_H_
#define CHERRYQTICONIMAGEDESCRIPTOR_H_

#include <cherryImageDescriptor.h>

class QIcon;

namespace cherry {

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

#endif /* CHERRYQTICONIMAGEDESCRIPTOR_H_ */
