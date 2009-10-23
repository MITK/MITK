#ifndef QmitkNodeDescriptor_h
#define QmitkNodeDescriptor_h

#include <map>
#include "mitkDataTreeNode.h"
#include <QList>
#include <QAction>
#include <QWidgetAction>
#include <QString>
#include <QIcon>
#include <mitkNodePredicateBase.h>

/// 
/// \class QmitkNodeQmitkNodeDescriptor
/// \brief QmitkNodeQmitkNodeDescriptor is <i>Decorator</i> class for the mitk::DataTreeNode
/// which enhances certain mitk::DataTreeNode by additional infos needed by the GUI (Icon, ...)
///
/// Moreover, QmitkNodeQmitkNodeDescriptor stores a Menu for actions that can be taken
/// for a certain DataTreeNode, e.g. for DataTreeNodes containing images this menu
/// can be filled with Image Filter Actions, etc.
///
/// \sa QmitkDataTreeNodeQmitkNodeDescriptorManager
///
class QMITK_EXPORT QmitkNodeDescriptor : public QObject
{  
  Q_OBJECT
public:
  ///
  /// Creates a new QmitkNodeQmitkNodeDescriptor
  ///
  QmitkNodeDescriptor(const QString& _ClassName, const QIcon& _Icon
    , mitk::NodePredicateBase* _Predicate, QObject* parent);
  ///
  /// Deletes all actions
  ///
  virtual ~QmitkNodeDescriptor();
  ///
  /// Returns a name for this class of DataTreeNodes (e.g. "Image", "Image Mask", etc.)
  ///
  virtual QString GetClassName() const;
  ///
  /// Returns an Icon for this class of DataTreeNodes
  ///
  virtual QIcon GetIcon() const;
  ///
  /// Returns an Icon for this class of DataTreeNodes
  ///
  virtual QAction* GetSeparator() const;
  ///
  /// Check if this class describes the given node
  ///
  virtual bool CheckNode(const mitk::DataTreeNode* node) const;
  ///
  /// Create and return an action with this descriptor as owner
  ///
  virtual void AddAction ( QAction * action, bool isBatchAction=true );
  ///
  /// Remove and delete (!) an action
  ///
  virtual void RemoveAction(QAction* _Action);
  ///
  /// Get all actions associated with this class of nodes
  ///
  virtual QList<QAction*> GetActions() const;
  ///
  /// Get all actions for this descriptor class that can be executed on multiple nodes
  /// (no priot knowledge abpout the node is required)
  ///
  virtual QList<QAction*> GetBatchActions() const;

public slots:
  /// Called when an action was destroyed
  void ActionDestroyed ( QObject * obj = 0 );
protected:
  QString m_ClassName;
  QIcon m_Icon;
  mitk::NodePredicateBase::Pointer m_Predicate;
  QList<QAction*> m_Actions;
  QList<QAction*> m_BatchActions;
  QAction* m_Separator;
};

#endif // QmitkNodeDescriptor_h
