/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
