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
