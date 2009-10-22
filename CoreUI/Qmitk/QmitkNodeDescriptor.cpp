#include "QmitkNodeDescriptor.h"
#include <memory>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAND.h>
#include <mitkNodePredicateDataType.h>
#include <mitkProperties.h>

QmitkNodeDescriptor::QmitkNodeDescriptor( const QString& _ClassName, const QIcon& _Icon
                                                   , mitk::NodePredicateBase* _Predicate, QObject* parent )
: QObject(parent)
, m_ClassName(_ClassName)
, m_Predicate(_Predicate)
, m_Icon(_Icon)
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
  return m_Icon;
}

QList<QAction*> QmitkNodeDescriptor::GetActions() const
{
  return m_Actions;
}

bool QmitkNodeDescriptor::CheckNode( const mitk::DataTreeNode* node ) const
{
  if(m_Predicate.IsNotNull())
    return m_Predicate->CheckNode(node);
  return false;
}

QAction * QmitkNodeDescriptor::AddAction( const QString & text, bool isBatchAction )
{
  QAction* action = new QAction(text, this);
  if(isBatchAction)
    m_BatchActions.push_back(action);
  else
    m_Actions.push_back(action);
  return action;
}

QAction * QmitkNodeDescriptor::AddAction( const QIcon & icon, const QString & text, bool isBatchAction )
{
  QAction* action = new QAction(icon, text, this);
  if(isBatchAction)
    m_BatchActions.push_back(action);
  else
    m_Actions.push_back(action);
  return action;
}

QWidgetAction * QmitkNodeDescriptor::AddWidgetAction( bool isBatchAction )
{
  QWidgetAction* action = new QWidgetAction(this);
  if(isBatchAction)
    m_BatchActions.push_back(action);
  else
    m_Actions.push_back(action);
  return action;
}

void QmitkNodeDescriptor::RemoveAction( QAction* _Action )
{
  int index = m_Actions.indexOf(_Action);
  int indexOfWidgetAction = m_BatchActions.indexOf(_Action);

  if(index != -1)
  {
    m_Actions.removeAt(index);
    _Action->setParent(0);
    delete _Action;
  }
  else if(indexOfWidgetAction != -1)
  {
    m_BatchActions.removeAt(indexOfWidgetAction);
    _Action->setParent(0);
    delete _Action;
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