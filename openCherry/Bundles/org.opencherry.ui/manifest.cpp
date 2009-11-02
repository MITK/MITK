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

#include "Poco/ClassLibrary.h"

#include <cherryIBundleActivator.h>
#include "src/internal/cherryWorkbenchPlugin.h"

POCO_BEGIN_MANIFEST(cherry::IBundleActivator)
  POCO_EXPORT_CLASS(cherry::WorkbenchPlugin)
POCO_END_MANIFEST

#include "src/internal/intro/cherryViewIntroAdapterPart.h"

POCO_BEGIN_NAMED_MANIFEST(cherryIViewPart, cherry::IViewPart)
  POCO_EXPORT_CLASS(cherry::ViewIntroAdapterPart)
POCO_END_MANIFEST
