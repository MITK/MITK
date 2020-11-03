/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCmdLineModuleMenuComboBox.h"
#include <iostream>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QList>
#include <cassert>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleManager.h>

// -------------------------------------------------------------------------
QmitkCmdLineModuleMenuComboBox::QmitkCmdLineModuleMenuComboBox(QWidget* parent)
: ctkMenuComboBox(parent)
, m_ModuleManager(nullptr)
{
  qRegisterMetaType<ctkCmdLineModuleReference>();
}


// -------------------------------------------------------------------------
QmitkCmdLineModuleMenuComboBox::~QmitkCmdLineModuleMenuComboBox()
{

}


// -------------------------------------------------------------------------
void QmitkCmdLineModuleMenuComboBox::SetManager(ctkCmdLineModuleManager* manager)
{
  if (m_ModuleManager != nullptr)
  {
    QObject::disconnect(manager, nullptr, this, nullptr);
  }

  m_ModuleManager = manager;

  connect(m_ModuleManager, SIGNAL(moduleRegistered(const ctkCmdLineModuleReference&)), this, SLOT(OnModuleRegistered(const ctkCmdLineModuleReference&)));
  connect(m_ModuleManager, SIGNAL(moduleUnregistered(const ctkCmdLineModuleReference&)), this, SLOT(OnModuleUnRegistered(const ctkCmdLineModuleReference&)));
}


// -------------------------------------------------------------------------
ctkCmdLineModuleManager* QmitkCmdLineModuleMenuComboBox::GetManager() const
{
  return m_ModuleManager;
}


// -------------------------------------------------------------------------
void QmitkCmdLineModuleMenuComboBox::OnModuleRegistered(const ctkCmdLineModuleReference&)
{
  this->RebuildMenu();
}


// -------------------------------------------------------------------------
void QmitkCmdLineModuleMenuComboBox::OnModuleUnRegistered(const ctkCmdLineModuleReference&)
{
  this->RebuildMenu();
}


// -------------------------------------------------------------------------
void QmitkCmdLineModuleMenuComboBox::AddName(
    QList< QHash<QString, QMenu*>* >& listOfHashMaps,
    const int& depth,
    const QString& name,
    QMenu* menu
    )
{
  if (depth >= listOfHashMaps.size())
  {
    int need = depth - listOfHashMaps.size();
    for (int i = 0; i <= need; i++)
    {
      auto  newHashMap = new QHash<QString, QMenu*>();
      listOfHashMaps.push_back(newHashMap);
    }
  }
  listOfHashMaps[depth]->insert(name, menu);
}


// -------------------------------------------------------------------------
void QmitkCmdLineModuleMenuComboBox::RebuildMenu()
{
  if (m_ModuleManager == nullptr)
  {
    qDebug() << "QmitkCmdLineModuleMenuComboBox::RebuildMenu(): Module Manager is nullptr? Surely a bug?";
    return;
  }

  // Can't see another way to get a nested menu, without rebuilding the whole thing each time.
  // :-(

  auto  menu = new QMenu();
  QStringList listOfModules;

  QList<ctkCmdLineModuleReference> references = m_ModuleManager->moduleReferences();

  // Get full names
  for (int i = 0; i < references.size(); i++)
  {
    ctkCmdLineModuleReference reference = references[i];
    ctkCmdLineModuleDescription description = reference.description();
    QString title = description.title();
    QString category = description.category();
    QString fullName = category + "." + title;
    listOfModules << fullName;
  }

  // Sort list, so menu comes out in some sort of nice order.
  listOfModules.sort();

  // Temporary data structure to enable connecting menus together.
  QList< QHash<QString, QMenu*>* > list;

  // Iterate through all modules.
  foreach (QString fullName, listOfModules)
  {
    // Pointer to keep track of where we are in the menu tree.
    QMenu *currentMenu = menu;

    // Get individual parts, as they correspond to menu levels.
    QStringList nameParts = fullName.split(".", QString::SkipEmptyParts);

    // Iterate over each part, building up either a QMenu or QAction.
    for (int i = 0; i < nameParts.size(); i++)
    {
      QString part = nameParts[i];

      if (i != nameParts.size() - 1)
      {
        // Need to add a menu/submenu, not an action.
        if (list.size() <= i || list[i] == nullptr || !list[i]->contains(part))
        {
          auto  newMenu = new QMenu(part);
          currentMenu->addMenu(newMenu);
          currentMenu = newMenu;

          // Add this newMenu pointer to temporary datastructure,
          // so we know we have already created it.
          this->AddName(list, i, part, newMenu);
        }
        else
        {
          currentMenu = list[i]->value(part);
        }
      }
      else
      {
        // Leaf node, just add the action.
        QAction *action = currentMenu->addAction(part);

        // We set the object name, so we can retrieve it later when we want to
        // rebuild a new GUI depending on the name of the action.
        // see QmitkCmdLineModuleProgressWidget.
        action->setObjectName(fullName);
      }
    }
  }

  // Clearup termporary data structure
  for (int i = 0; i < list.size(); i++)
  {
    delete list[i];
  }

  // Set the constructed menu on the base class combo box.
  this->setMenu(menu);
}
