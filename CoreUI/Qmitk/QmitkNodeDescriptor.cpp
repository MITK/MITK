/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkNodeDescriptor.h"
#include <memory>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAND.h>
#include <mitkNodePredicateDataType.h>
#include <mitkProperties.h>

QmitkNodeDescriptor::QmitkNodeDescriptor( const QString& _ClassName, const QString& _PathToIcon
                                                   , mitk::NodePredicateBase* _Predicate, QObject* parent )
: QObject(parent)
, m_ClassName(_ClassName)
, m_PathToIcon(_PathToIcon)
, m_Predicate(_Predicate)
, m_Separator(new QAction(this))
{
  m_Separator->setSeparator(true);
}

QString QmitkNodeDescriptor::GetClassName() const
{
  return m_ClassName;
}

QIcon QmitkNodeDescriptor::GetIcon() const
{
  return QIcon(m_PathToIcon);
}

QList<QAction*> QmitkNodeDescriptor::GetActions() const
{
  return m_Actions;
}

bool QmitkNodeDescriptor::CheckNode( const mitk::DataNode* node ) const
{
  if(m_Predicate.IsNotNull())
    return m_Predicate->CheckNode(node);
  return false;
}

void QmitkNodeDescriptor::AddAction( QAction* action, bool isBatchAction )
{
  if(!action)
    return;

  if(isBatchAction)
    m_BatchActions.push_back(action);
  else
    m_Actions.push_back(action);
  QObject::connect( action, SIGNAL( destroyed(QObject *) )
    , this, SLOT( ActionDestroyed(QObject *) ) );
}

void QmitkNodeDescriptor::RemoveAction( QAction* _Action )
{
  int index = m_Actions.indexOf(_Action);
  int indexOfWidgetAction = m_BatchActions.indexOf(_Action);

  if(index != -1)
  {
    m_Actions.removeAt(index);
  }
  else if(indexOfWidgetAction != -1)
  {
    m_BatchActions.removeAt(indexOfWidgetAction);
  }

  if( _Action != 0)
  {
    QObject::disconnect( _Action, SIGNAL( destroyed(QObject *) )
      , this, SLOT( ActionDestroyed(QObject *) ) );
  }
}

QmitkNodeDescriptor::~QmitkNodeDescriptor()
{
  // all children are destroyed here by Qt
}

QAction* QmitkNodeDescriptor::GetSeparator() const
{
  return m_Separator;
}

QList<QAction*> QmitkNodeDescriptor::GetBatchActions() const
{
  return m_BatchActions;
}

void QmitkNodeDescriptor::ActionDestroyed( QObject * obj /*= 0 */ )
{
  this->RemoveAction( qobject_cast<QAction*>(obj) );
}