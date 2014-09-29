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

#include <QHeaderView>
#include "QmitkBaseTreeWidget.h"
#include "QmitkBaseItemDelegate.h"
#include "QmitkNoEditItemDelegate.h"
#include "QmitkSceneTreeWidget.h"

QmitkBaseTreeWidget::QmitkBaseTreeWidget(QWidget* parent)
  : QTreeWidget(parent),
    m_Base(NULL)
{
  this->setItemDelegateForColumn(0, new QmitkNoEditItemDelegate);
  this->setItemDelegateForColumn(1, new QmitkBaseItemDelegate);

  this->setSortingEnabled(true);
  this->header()->setSortIndicator(0, Qt::AscendingOrder);
}

QmitkBaseTreeWidget::~QmitkBaseTreeWidget()
{
}

void QmitkBaseTreeWidget::clear()
{
  m_Base = NULL;
  m_GroupItemMap.clear();

  QTreeWidget::clear();
}

void QmitkBaseTreeWidget::OnSelectedBaseChanged(Base* base)
{
  this->clear();

  m_Base = base;

  if (base != NULL)
    this->FillTreeWidget();
}

void QmitkBaseTreeWidget::FillTreeWidget()
{
  const Base::VecData& dataFields = m_Base->getDataFields();

  for (Base::VecData::const_iterator dataField = dataFields.begin(); dataField != dataFields.end(); ++dataField)
  {
    if (!(*dataField)->isDisplayed())
      continue;

    QString name = QString::fromStdString((*dataField)->getName());

    if (name.isEmpty())
      continue;

    QString group = (*dataField)->getGroup();

    if (!group.isEmpty() && !m_GroupItemMap.contains(group))
      m_GroupItemMap.insert(group, new QTreeWidgetItem(this, QStringList() << group));

    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << name);
    item->setToolTip(0, (*dataField)->getHelp());

    item->setFlags(!(*dataField)->isReadOnly()
      ? item->flags() | Qt::ItemIsEditable
      : Qt::ItemIsSelectable);

    QString type = QString::fromStdString((*dataField)->getValueTypeString());

    if (type == "bool")
    {
      item->setFlags((item->flags() & ~Qt::EditRole) | Qt::ItemIsUserCheckable);
      item->setCheckState(1, (*dataField)->getValueString() == "1" ? Qt::Checked : Qt::Unchecked);
    }
    else
    {
      if (type == "double" || type == "float" || type == "int" || type == "string" || type == "unsigned int")
      {
        item->setData(1, Qt::DisplayRole, QVariant::fromValue(QString::fromStdString((*dataField)->getValueString())));
      }
      else
      {
        item->setData(1, Qt::DisplayRole, QVariant::fromValue("[" + type + "]"));
      }

      item->setData(1, Qt::UserRole, QVariant::fromValue(*dataField));
    }

    if (group.isEmpty())
    {
      this->addTopLevelItem(item);
    }
    else
    {
      m_GroupItemMap[group]->addChild(item);
    }
  }

  this->setRootIsDecorated(!m_GroupItemMap.isEmpty());
  this->expandAll();
}
