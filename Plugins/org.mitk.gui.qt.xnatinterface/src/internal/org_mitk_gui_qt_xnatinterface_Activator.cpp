/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "org_mitk_gui_qt_xnatinterface_Activator.h"

#include <QtPlugin>

#include "QmitkXnatEditor.h"
#include "QmitkXnatSimpleSearchView.h"
#include "QmitkXnatTreeBrowserView.h"
#include "QmitkXnatConnectionPreferencePage.h"

namespace mitk {

void org_mitk_gui_qt_xnatinterface_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatEditor, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatSimpleSearchView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatTreeBrowserView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkXnatConnectionPreferencePage, context)
}

void org_mitk_gui_qt_xnatinterface_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_xnatinterface, mitk::org_mitk_gui_qt_xnatinterface_Activator)
