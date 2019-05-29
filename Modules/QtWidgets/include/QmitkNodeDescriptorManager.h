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

#ifndef QmitkNodeDescriptorManager_h
#define QmitkNodeDescriptorManager_h

#include <MitkQtWidgetsExports.h>

#include "QmitkNodeDescriptor.h"

/**
 * \ingroup QmitkModule
 * \brief QmitkNodeDescriptorManager manages a set of QmitkNodeDescriptors
 *
 * \sa QmitkNodeDescriptor
 */
class MITKQTWIDGETS_EXPORT QmitkNodeDescriptorManager : public QObject
{
  Q_OBJECT

public:
  ///
  /// \return the solely instance of QmitkNodeDescriptorManager
  ///
  static QmitkNodeDescriptorManager* GetInstance();

  ///
  /// Initializes the QmitkNodeDescriptorManager.
  /// Adds a few standard Descriptors.
  /// This Descriptors are added:
  /// - A QmitkNodeDescriptor for the class of "Image" DataNodes
  /// - A QmitkNodeDescriptor for the class of "Image Mask" DataNodes
  /// - A QmitkNodeDescriptor for the class of "Surface" DataNodes
  /// - A QmitkNodeDescriptor for the class of "PointSet" DataNodes
  ///
  virtual void Initialize();

  ///
  /// Adds a new descriptor to the manager. The manager takes the ownership.
  ///
  void AddDescriptor(QmitkNodeDescriptor* descriptor);

  ///
  /// Removes and deletes a descriptor from the manager
  ///
  void RemoveDescriptor(QmitkNodeDescriptor* descriptor);

  ///
  /// Get the last descriptor in the descriptors list that matches the given node.
  /// *Attention*: More specialized Descriptors should therefore be appended at
  /// the end of the list, e.g. first add "Image", then add "Image Mask"
  ///
  /// \return a QmitkNodeDescriptor for the given node or a QmitkNodeDescriptor describing unknown nodes (never 0)
  /// \sa AddDescriptor()
  ///
  QmitkNodeDescriptor* GetDescriptor(const mitk::DataNode* node) const;

  ///
  /// Get the last QmitkNodeDescriptor for the given class name
  ///
  /// \return a QmitkNodeDescriptor for the given class name or 0 if there is no QmitkNodeDescriptor for _ClassName
  ///
  QmitkNodeDescriptor* GetDescriptor(const QString& className) const;

  ///
  /// \return The UnknownDataNodeDescriptor, which is the default Descriptor for all Nodes.
  ///
  QmitkNodeDescriptor* GetUnknownDataNodeDescriptor() const;

  ///
  /// Returns a list of all actions that are associated with the given node.
  /// If there are more than one Descriptors for this node all actions
  /// will be merged together.
  /// E.g. all actions from the "unknown" DataNodes will be added to
  /// this list. Generic Actions like Save, Load, etc. are stored there.
  ///
  QList<QAction*> GetActions(const mitk::DataNode* node) const;
  ///
  /// \return a list of actions associated with the given nodes
  ///
  QList<QAction*> GetActions(const QList<mitk::DataNode::Pointer>& nodes) const;
  ///
  /// Deletes all Descriptors in the list
  ///
  ~QmitkNodeDescriptorManager() override;

protected:
  ///
  /// Creates the m_UnknownDataNodeDescriptor
  /// Calls Initialize
  ///
  QmitkNodeDescriptorManager();

protected:
  ///
  /// This is the standard QmitkNodeDescriptor matching every node
  ///
  QmitkNodeDescriptor* m_UnknownDataNodeDescriptor;
  ///
  /// Holds all user defined descriptors
  ///
  QList<QmitkNodeDescriptor*> m_NodeDescriptors;
};

#endif // QmitkNodeDescriptorManager_h
