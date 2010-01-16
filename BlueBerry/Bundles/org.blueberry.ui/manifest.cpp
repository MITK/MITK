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

#include "Poco/ClassLibrary.h"

#include <berryIBundleActivator.h>
#include "src/internal/berryWorkbenchPlugin.h"

POCO_BEGIN_MANIFEST(berry::IBundleActivator)
  POCO_EXPORT_CLASS(berry::WorkbenchPlugin)
POCO_END_MANIFEST

#include "src/internal/intro/berryViewIntroAdapterPart.h"

POCO_BEGIN_NAMED_MANIFEST(berryIViewPart, berry::IViewPart)
  POCO_EXPORT_CLASS(berry::ViewIntroAdapterPart)
POCO_END_MANIFEST

#include "src/internal/intro/berryEditorIntroAdapterPart.h"

POCO_BEGIN_NAMED_MANIFEST(berryIEditorPart, berry::IEditorPart)
  POCO_EXPORT_CLASS(berry::EditorIntroAdapterPart)
POCO_END_MANIFEST
