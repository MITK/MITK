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
 
#include "QmitkCmdLineModuleFactoryGui.h"
#include "QmitkCmdLineModuleGui.h"

//-----------------------------------------------------------------------------
QmitkCmdLineModuleFactoryGui::QmitkCmdLineModuleFactoryGui(const mitk::DataStorage* dataStorage)
  : m_DataStorage(dataStorage)
{

}


//-----------------------------------------------------------------------------
QmitkCmdLineModuleFactoryGui::~QmitkCmdLineModuleFactoryGui()
{

}


//-----------------------------------------------------------------------------
ctkCmdLineModule* QmitkCmdLineModuleFactoryGui::create(const ctkCmdLineModuleReference& moduleRef)
{
  QmitkCmdLineModuleGui* gui = new QmitkCmdLineModuleGui(m_DataStorage, moduleRef);
  return gui;
}
