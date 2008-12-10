/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date: 2008-06-12 19:58:42 +0200 (Do, 12 Jun 2008) $
Version:   $Revision: 14610 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef (@GENERATE_VIEW_CLASS@_H_INCLUDED)
#define @GENERATE_VIEW_CLASS@_H_INCLUDED

#include <cherryPlugin.h>
#include <cherryIBundleContext.h>
#include <event/cherryPlatformEvent.h>

#include "cherryRuntimeDll.h"

namespace cherry {

class CHERRY_RUNTIME @GENERATE_VIEW_CLASS@ : public Plugin
{
public:
  
  static const std::string @PLUGIN_ID@;
  
  void Start(IBundleContext::Pointer context);
  
};

}

#endif /*@GENERATE_VIEW_CLASS@_H_INCLUDED*/
