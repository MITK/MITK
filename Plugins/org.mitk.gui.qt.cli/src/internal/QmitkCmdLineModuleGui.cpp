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
#include <QIODevice>
#include <QFile>
#include <QScopedPointer>
#include <ctkCmdLineModuleXslTransform.h>
#include <ctkCmdLineModuleParameter.h>
#include <ctkCmdLineModuleDescription.h>
#include "mitkDataStorage.h"

//-----------------------------------------------------------------------------
struct QmitkCmdLineModuleGuiPrivate
{
  QmitkCmdLineModuleGuiPrivate(const mitk::DataStorage* dataStorage)
    : m_DataStorage(dataStorage), m_Loader(NULL), m_Transform(NULL)
  {}

  const mitk::DataStorage* m_DataStorage;
  mutable QScopedPointer<QUiLoader> m_Loader;
  mutable QScopedPointer<ctkCmdLineModuleXslTransform> m_Transform;
};


//-----------------------------------------------------------------------------
QmitkCmdLineModuleGui::QmitkCmdLineModuleGui(const mitk::DataStorage* dataStorage, const ctkCmdLineModuleReference& moduleRef)
  : ctkCmdLineModuleFrontendQtGui(moduleRef)
, d(new QmitkCmdLineModuleGuiPrivate(dataStorage))
{
}


//-----------------------------------------------------------------------------
QmitkCmdLineModuleGui::~QmitkCmdLineModuleGui()
{
}


//-----------------------------------------------------------------------------
QUiLoader* QmitkCmdLineModuleGui::uiLoader() const
{
  // Here we are creating a QUiLoader locally, so when this method
  // is called, it overrides the one in the base class, so the base
  // class one is never constructed, and this one is constructed as
  // a replacement.
  if (d->m_Loader == NULL)
  {
    d->m_Loader.reset(new QmitkUiLoader(d->m_DataStorage));
  }
  return d->m_Loader.data();
}


//-----------------------------------------------------------------------------
ctkCmdLineModuleXslTransform* QmitkCmdLineModuleGui::xslTransform() const
{
  // This is a virtual getter, overriding the one in the base class.
  // However, we want to use the transform in the base class, and just append to it.
  // So we call the base class one, modify it by adding some stuff, and then return
  // the pointer to the one in the base class.
  ctkCmdLineModuleXslTransform *transform = ctkCmdLineModuleFrontendQtGui::xslTransform();
  if (transform != NULL)
  {
    transform->bindVariable("imageInputWidget", QVariant(QString("QmitkDataStorageComboBoxWithSelectNone")));
    QIODevice* transformQmitkDataStorageComboBox(new QFile(":/CommandLineModulesResources/QmitkDataStorageComboBox.xsl"));
    if (transformQmitkDataStorageComboBox)
    {
      transform->setXslExtraTransformation(transformQmitkDataStorageComboBox);
    }
  }
  return transform;
}


//-----------------------------------------------------------------------------
QVariant QmitkCmdLineModuleGui::value(const QString &parameter, int role) const
{
  if (role == UserRole)
  {
    ctkCmdLineModuleParameter param = this->moduleReference().description().parameter(parameter);
    if (param.channel() == "input" && param.tag() == "image")
    {
      return this->customValue(parameter, "GetSelectedNode");
    }
    return QVariant();
  }
  return ctkCmdLineModuleFrontendQtGui::value(parameter, role);
}
