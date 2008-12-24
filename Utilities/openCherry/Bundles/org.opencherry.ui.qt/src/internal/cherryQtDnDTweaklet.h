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


#ifndef CHERRYQTDNDTWEAKLET_H_
#define CHERRYQTDNDTWEAKLET_H_

#include <tweaklets/cherryDnDTweaklet.h>

namespace cherry {

class QtDnDTweaklet : public DnDTweaklet
{
public:

  ITracker* CreateTracker();
};

}

#endif /* CHERRYQTDNDTWEAKLET_H_ */
