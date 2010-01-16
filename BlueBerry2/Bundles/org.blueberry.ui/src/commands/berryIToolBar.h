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


#ifndef BERRYITOOLBAR_H_
#define BERRYITOOLBAR_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include "../berryUiDll.h"

namespace berry {

struct BERRY_UI IToolBar : public Object
{
  osgiInterfaceMacro(berry::IToolBar)
};

}

#endif /* BERRYITOOLBAR_H_ */
