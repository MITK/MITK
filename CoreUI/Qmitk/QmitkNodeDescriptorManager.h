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

#ifndef QmitkNodeDescriptorManager_h
#define QmitkNodeDescriptorManager_h

#include "QmitkNodeDescriptor.h"

/// 
/// \class QmitkNodeDescriptorManager
/// \brief QmitkNodeDescriptorManager manages a set of QmitkNodeDescriptors
///
/// \sa QmitkNodeDescriptor
///
class QMITK_EXPORT QmitkNodeDescriptorManager : public QObject
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
  void AddDescriptor(QmitkNodeDescriptor* _Descriptor);

  ///
  /// Removes and deletes a descriptor from the manager
  ///
  void RemoveDescriptor(QmitkNodeDescriptor* _Descriptor);

  ///
  /// Get the last descriptor in the descriptors list that matches the given node.
  /// *Attention*: More specialized Descriptors should therefore be appended at
  /// the end of the list, e.g. first add "Image", then add "Image Mask"
  ///
  /// \return a QmitkNodeDescriptor for the given node or a QmitkNodeDescriptor describing unknown nodes (never 0)
  /// \sa AddDescriptor()
  ///
  QmitkNodeDescriptor* GetDescriptor(const mitk::DataNode* _Node) const;

  ///
  /// Get the last QmitkNodeDescriptor for the given class name
  ///
  /// \return a QmitkNodeDescriptor for the given class name or 0 if there is no QmitkNodeDescriptor for _ClassName
  ///
  QmitkNodeDescriptor* GetDescriptor(const QString& _ClassName) const;

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
  QList<QAction*> GetActions(const mitk::DataNode* _Node) const;
  ///
  /// \return a list of actions associated with the given nodes
  ///
  QList<QAction*> GetActions( const std::vector<mitk::DataNode*>& _Nodes ) const;
  ///
  /// Deletes all Descriptors in the list
  ///
  virtual ~QmitkNodeDescriptorManager();
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
