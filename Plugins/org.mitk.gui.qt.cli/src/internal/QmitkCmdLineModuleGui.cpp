/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkCmdLineModuleGui.h"
#include "QmitkUiLoader.h"
#include <QVariant>
#include <ctkCmdLineModuleXslTransform.h>

//-----------------------------------------------------------------------------
QmitkCmdLineModuleGui::QmitkCmdLineModuleGui(const mitk::DataStorage* dataStorage, const ctkCmdLineModuleReference& moduleRef)
  : ctkCmdLineModuleQtGui(moduleRef)
, m_DataStorage(dataStorage)
{
}


//-----------------------------------------------------------------------------
QmitkCmdLineModuleGui::~QmitkCmdLineModuleGui()
{
}


//-----------------------------------------------------------------------------
QUiLoader* QmitkCmdLineModuleGui::uiLoader() const
{
  if (Loader == NULL)
  {
    Loader = new QmitkUiLoader(m_DataStorage);
  }
  return Loader;
}


//-----------------------------------------------------------------------------
ctkCmdLineModuleXslTransform* QmitkCmdLineModuleGui::xslTransform() const
{
  ctkCmdLineModuleQtGui::xslTransform();
  if (Transform != NULL)
  {
    Transform->bindVariable("imageWidget", QVariant(QString("QmitkDataStorageComboBox")));
  }
  return Transform;
}
