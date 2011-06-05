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
#include "src/internal/berryQtPluginActivator.h"

#include "src/internal/berryQtWorkbenchTweaklet.h"
#include "src/internal/berryQtWorkbenchPageTweaklet.h"
#include "src/internal/berryQtDnDTweaklet.h"
#include "src/internal/berryQtWidgetsTweaklet.h"
#include "src/internal/berryQtImageTweaklet.h"
#include "src/internal/berryQtMessageDialogTweaklet.h"
#include "src/internal/defaultpresentation/berryQtWorkbenchPresentationFactory.h"

#include "src/internal/berryQtStylePreferencePage.h"

//****  Activator ****************

POCO_BEGIN_MANIFEST(berry::IBundleActivator)
  POCO_EXPORT_CLASS(berry::QtPluginActivator)
POCO_END_MANIFEST

//****  Tweaklets  ****************


//**** Preference Pages *****
POCO_BEGIN_NAMED_MANIFEST(berryIPreferencePage, berry::IPreferencePage)
  POCO_EXPORT_CLASS(berry::QtStylePreferencePage)
POCO_END_MANIFEST
