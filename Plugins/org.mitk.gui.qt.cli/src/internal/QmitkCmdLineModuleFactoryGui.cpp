/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
 
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
