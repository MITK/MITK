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

#ifndef QmitkNodeDescriptor_h
#define QmitkNodeDescriptor_h

#include <map>
#include "mitkDataNode.h"
#include <QList>
#include <QAction>
#include <QWidgetAction>
#include <QString>
#include <QIcon>
#include <mitkNodePredicateBase.h>

/// 
/// \class QmitkNodeQmitkNodeDescriptor
/// \brief QmitkNodeQmitkNodeDescriptor is <i>Decorator</i> class for the mitk::DataNode
/// which enhances certain mitk::DataNode by additional infos needed by the GUI (Icon, ...)
///
/// Moreover, QmitkNodeQmitkNodeDescriptor stores a Menu for actions that can be taken
/// for a certain DataNode, e.g. for DataNodes containing images this menu
/// can be filled with Image Filter Actions, etc.
///
/// \sa QmitkDataNodeQmitkNodeDescriptorManager
///
class QMITK_EXPORT QmitkNodeDescriptor : public QObject
{  
  Q_OBJECT
public:
  ///
  /// Creates a new QmitkNodeQmitkNodeDescriptor
  ///
  QmitkNodeDescriptor(const QString& _ClassName, const QString& _PathToIcon
    , mitk::NodePredicateBase* _Predicate, QObject* parent);
  ///
  /// Deletes all actions
  ///
  virtual ~QmitkNodeDescriptor();
  ///
  /// Returns a name for this class of DataNodes (e.g. "Image", "Image Mask", etc.)
  ///
  virtual QString GetClassName() const;
  ///
  /// Returns an Icon for this class of DataNodes
  /// 
  virtual QIcon GetIcon() const;
  ///
  /// Returns an Icon for this class of DataNodes
  ///
  virtual QAction* GetSeparator() const;
  ///
  /// Check if this class describes the given node
  ///
  virtual bool CheckNode(const mitk::DataNode* node) const;
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
  QString m_PathToIcon;
  mitk::NodePredicateBase::Pointer m_Predicate;
  QList<QAction*> m_Actions;
  QList<QAction*> m_BatchActions;
  QAction* m_Separator;
};

#endif // QmitkNodeDescriptor_h
