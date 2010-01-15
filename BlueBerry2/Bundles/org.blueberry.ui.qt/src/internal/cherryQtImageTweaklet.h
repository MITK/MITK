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


#ifndef CHERRYQTIMAGETWEAKLET_H_
#define CHERRYQTIMAGETWEAKLET_H_

#include <cherryImageTweaklet.h>

namespace cherry {

class QtImageTweaklet : public ImageTweaklet
{
public:

  virtual SmartPointer<ImageDescriptor> CreateFromFile(const std::string& filename, const std::string& pluginid);
  virtual SmartPointer<ImageDescriptor> CreateFromImage(void* img);
  virtual SmartPointer<ImageDescriptor> GetMissingImageDescriptor();

  virtual void DestroyImage(const void* img);

};

}

#endif /* CHERRYQTIMAGETWEAKLET_H_ */
