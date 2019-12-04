/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCmdLineModuleFactoryGui.h"
#include "QmitkCmdLineModuleGui.h"

//-----------------------------------------------------------------------------
struct QmitkCmdLineModuleFactoryGuiPrivate
{
  QmitkCmdLineModuleFactoryGuiPrivate(const mitk::DataStorage* dataStorage)
    : m_DataStorage(dataStorage)
  {}

  const mitk::DataStorage* m_DataStorage;
};

//-----------------------------------------------------------------------------
QmitkCmdLineModuleFactoryGui::QmitkCmdLineModuleFactoryGui(const mitk::DataStorage* dataStorage)
  : d(new QmitkCmdLineModuleFactoryGuiPrivate(dataStorage))
{
}


//-----------------------------------------------------------------------------
QmitkCmdLineModuleFactoryGui::~QmitkCmdLineModuleFactoryGui()
{
}


//-----------------------------------------------------------------------------
ctkCmdLineModuleFrontendQtGui* QmitkCmdLineModuleFactoryGui::create(const ctkCmdLineModuleReference& moduleRef)
{
  ctkCmdLineModuleFrontendQtGui* gui = new QmitkCmdLineModuleGui(d->m_DataStorage, moduleRef);
  return gui;
}
