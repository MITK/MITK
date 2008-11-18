/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date: 2008-06-25 16:22:13 +0200 (Mi, 25 Jun 2008) $
Version:   $Revision: 14645 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <Poco/ClassLibrary.h>

#include <cherryIViewPart.h>
#include "src/QmitkHelloWorldView.h"

POCO_BEGIN_NAMED_MANIFEST(cherryIViewPart, cherry::IViewPart)
  POCO_EXPORT_CLASS(QmitkHelloWorldView)
POCO_END_MANIFEST
