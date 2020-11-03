/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNodeDescriptor.h"
#include <memory>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>

QmitkNodeDescriptor::QmitkNodeDescriptor(const QString &_ClassName,
                                         const QString &_PathToIcon,
                                         mitk::NodePredicateBase *_Predicate,
                                         QObject *parent)
  : QmitkNodeDescriptor(_ClassName, QIcon(_PathToIcon), _Predicate, parent)
{
}

QmitkNodeDescriptor::QmitkNodeDescriptor(const QString &_ClassName,
                                         const QIcon &_Icon,
                                         mitk::NodePredicateBase *_Predicate,
                                         QObject *parent)
  : QObject(parent),
    m_ClassName(_ClassName),
    m_Icon(_Icon),
    m_Predicate(_Predicate),
    m_Separator(new QAction(this))
{
  m_Separator->setSeparator(true);
}

QString QmitkNodeDescriptor::GetNameOfClass() const
{
  return m_ClassName;
}

QIcon QmitkNodeDescriptor::GetIcon(const mitk::DataNode *) const
{
  return m_Icon;
}

QList<QAction *> QmitkNodeDescriptor::GetActions() const
{
  return m_Actions;
}

bool QmitkNodeDescriptor::CheckNode(const mitk::DataNode *node) const
{
  if (m_Predicate.IsNotNull())
    return m_Predicate->CheckNode(node);
  return false;
}

void QmitkNodeDescriptor::AddAction(QAction *action, bool isBatchAction)
{
  if (!action)
    return;

  if (isBatchAction)
    m_BatchActions.push_back(action);
  else
    m_Actions.push_back(action);
  QObject::connect(action, SIGNAL(destroyed(QObject *)), this, SLOT(ActionDestroyed(QObject *)));
}

void QmitkNodeDescriptor::RemoveAction(QAction *_Action)
{
  int index = m_Actions.indexOf(_Action);
  int indexOfWidgetAction = m_BatchActions.indexOf(_Action);

  if (index != -1)
  {
    m_Actions.removeAt(index);
  }
  else if (indexOfWidgetAction != -1)
  {
    m_BatchActions.removeAt(indexOfWidgetAction);
  }

  if (_Action != nullptr)
  {
    QObject::disconnect(_Action, SIGNAL(destroyed(QObject *)), this, SLOT(ActionDestroyed(QObject *)));
  }
}

QmitkNodeDescriptor::~QmitkNodeDescriptor()
{
  // all children are destroyed here by Qt
}

QAction *QmitkNodeDescriptor::GetSeparator() const
{
  return m_Separator;
}

QList<QAction *> QmitkNodeDescriptor::GetBatchActions() const
{
  return m_BatchActions;
}

void QmitkNodeDescriptor::ActionDestroyed(QObject *obj /*= 0 */)
{
  this->RemoveAction(qobject_cast<QAction *>(obj));
}
