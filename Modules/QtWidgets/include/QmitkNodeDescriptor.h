/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNodeDescriptor_h
#define QmitkNodeDescriptor_h

#include <MitkQtWidgetsExports.h>

#include "mitkDataNode.h"
#include <QAction>
#include <QIcon>
#include <QList>
#include <QString>
#include <QWidgetAction>
#include <map>
#include <mitkNodePredicateBase.h>

/**
 * \ingroup QmitkModule
 * \brief <i>Decorator</i> class for mitk::DataNode.
 *
 * \sa QmitkNodeDescriptorManager
 */
class MITKQTWIDGETS_EXPORT QmitkNodeDescriptor : public QObject
{
  Q_OBJECT
public:
  ///
  /// Creates a new QmitkNodeDescriptor
  ///
  QmitkNodeDescriptor(const QString &_ClassName,
                      const QString &_PathToIcon,
                      mitk::NodePredicateBase *_Predicate,
                      QObject *parent);

  QmitkNodeDescriptor(const QString &_ClassName,
                      const QIcon &_Icon,
                      mitk::NodePredicateBase *_Predicate,
                      QObject *parent);

  ///
  /// Deletes all actions
  ///
  ~QmitkNodeDescriptor() override;
  ///
  /// Returns a name for this class of DataNodes (e.g. "Image", "Image Mask", etc.)
  ///
  virtual QString GetNameOfClass() const;
  ///
  /// Returns an Icon for this class of DataNodes
  ///
  virtual QIcon GetIcon(const mitk::DataNode *node) const;
  ///
  /// Returns an Icon for this class of DataNodes
  ///
  virtual QAction *GetSeparator() const;
  ///
  /// Check if this class describes the given node
  ///
  virtual bool CheckNode(const mitk::DataNode *node) const;
  ///
  /// Create and return an action with this descriptor as owner
  ///
  virtual void AddAction(QAction *action, bool isBatchAction = true);
  ///
  /// Remove and delete (!) an action
  ///
  virtual void RemoveAction(QAction *_Action);
  ///
  /// Get all actions associated with this class of nodes
  ///
  virtual QList<QAction *> GetActions() const;
  ///
  /// Get all actions for this descriptor class that can be executed on multiple nodes
  /// (no priot knowledge abpout the node is required)
  ///
  virtual QList<QAction *> GetBatchActions() const;

public slots:
  /// Called when an action was destroyed
  void ActionDestroyed(QObject *obj = nullptr);

protected:
  QString m_ClassName;
  QIcon m_Icon;
  mitk::NodePredicateBase::Pointer m_Predicate;
  QList<QAction *> m_Actions;
  QList<QAction *> m_BatchActions;
  QAction *m_Separator;
};

#endif
