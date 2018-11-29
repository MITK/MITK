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

#include "QmitkNodeDescriptorManager.h"
#include <memory>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>

#include <QList>
#include <QSet>

QmitkNodeDescriptorManager* QmitkNodeDescriptorManager::GetInstance()
{
  static QmitkNodeDescriptorManager instance;
  return &instance;
}

void QmitkNodeDescriptorManager::Initialize()
{
  auto isImage = mitk::NodePredicateDataType::New("Image");
  AddDescriptor(new QmitkNodeDescriptor(tr("Image"), QString(":/Qmitk/Images_48.png"), isImage, this));

  auto isMultiComponentImage = mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateProperty::New("Image.Displayed Component"));
  AddDescriptor(new QmitkNodeDescriptor(tr("MultiComponentImage"), QString(": / Qmitk / Images_48.png"), isMultiComponentImage, this));

  auto isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  auto isBinaryImage = mitk::NodePredicateAnd::New(isBinary, isImage);
  AddDescriptor(new QmitkNodeDescriptor(tr("ImageMask"), QString(":/Qmitk/Binaerbilder_48.png"), isBinaryImage, this));

  auto isLabelSetImage = mitk::NodePredicateDataType::New("LabelSetImage");
  AddDescriptor(new QmitkNodeDescriptor(tr("LabelSetImage"), QString(":/Qmitk/LabelSetImage_48.png"), isLabelSetImage, this));

  auto isPointSet = mitk::NodePredicateDataType::New("PointSet");
  AddDescriptor(new QmitkNodeDescriptor(tr("PointSet"), QString(":/Qmitk/PointSet_48.png"), isPointSet, this));

  auto isSurface = mitk::NodePredicateDataType::New("Surface");
  AddDescriptor(new QmitkNodeDescriptor(tr("Surface"), QString(":/Qmitk/Surface_48.png"), isSurface, this));

  auto isNotBinary = mitk::NodePredicateNot::New(isBinary);
  auto isNoneBinaryImage = mitk::NodePredicateAnd::New(isImage, isNotBinary);
  AddDescriptor(new QmitkNodeDescriptor(tr("NoneBinaryImage"), QString(":/Qmitk/Images_48.png"), isNoneBinaryImage, this));
}

void QmitkNodeDescriptorManager::AddDescriptor(QmitkNodeDescriptor* descriptor)
{
  descriptor->setParent(this);
  m_NodeDescriptors.push_back(descriptor);
}

void QmitkNodeDescriptorManager::RemoveDescriptor(QmitkNodeDescriptor* descriptor)
{
  int index = m_NodeDescriptors.indexOf(descriptor);

  if (index != -1)
  {
    m_NodeDescriptors.removeAt(index);
    descriptor->setParent(nullptr);
    delete descriptor;
  }
}

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetDescriptor(const mitk::DataNode* node) const
{
  QmitkNodeDescriptor* descriptor = m_UnknownDataNodeDescriptor;

  for (QList<QmitkNodeDescriptor *>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
  {
    if ((*it)->CheckNode(node))
      descriptor = *it;
  }

  return descriptor;
}

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetDescriptor(const QString& className) const
{
  QmitkNodeDescriptor* descriptor = nullptr;

  if (className == "Unknown")
  {
    return m_UnknownDataNodeDescriptor;
  }
  else
  {
    for (QList<QmitkNodeDescriptor *>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
    {
      if ((*it)->GetNameOfClass() == className)
        descriptor = *it;
    }
  }

  return descriptor;
}

QList<QAction*> QmitkNodeDescriptorManager::GetActions(const mitk::DataNode* node) const
{
  QList<QAction*> actions = m_UnknownDataNodeDescriptor->GetBatchActions();
  actions.append(m_UnknownDataNodeDescriptor->GetActions());
  QmitkNodeDescriptor* lastDescriptor = m_UnknownDataNodeDescriptor;

  for (QList<QmitkNodeDescriptor *>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
  {
    if ((*it)->CheckNode(node))
    {
      actions.append(lastDescriptor->GetSeparator());
      lastDescriptor = *it;
      actions.append(lastDescriptor->GetBatchActions());
      actions.append(lastDescriptor->GetActions());
    }
  }

  return actions;
}

QList<QAction*> QmitkNodeDescriptorManager::GetActions(const QList<mitk::DataNode::Pointer>& nodes) const
{
  QList<QAction*> actions = m_UnknownDataNodeDescriptor->GetBatchActions();
  QmitkNodeDescriptor* lastDescriptor = m_UnknownDataNodeDescriptor;

  // find all descriptors for the nodes (unique)
  QSet<QmitkNodeDescriptor*> nodeDescriptors;
  for (const auto& node : nodes)
  {
    for (QList<QmitkNodeDescriptor*>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
    {
      if ((*it)->CheckNode(node))
      {
        nodeDescriptors.insert(*it);
      }
    }
  }

  // add all actions for the found descriptors
  for (const auto& nodeDescriptor : nodeDescriptors)
  {
    actions.append(lastDescriptor->GetSeparator());
    lastDescriptor = nodeDescriptor;
    actions.append(lastDescriptor->GetBatchActions());
  }

  return actions;
}

QmitkNodeDescriptorManager::QmitkNodeDescriptorManager()
  : m_UnknownDataNodeDescriptor(new QmitkNodeDescriptor("Unknown", QString(":/Qmitk/DataTypeUnknown_48.png"), nullptr, this))
{
  Initialize();
}

QmitkNodeDescriptorManager::~QmitkNodeDescriptorManager()
{
  // delete m_UnknownDataNodeDescriptor;
  // qDeleteAll(m_NodeDescriptors);
}

QmitkNodeDescriptor *QmitkNodeDescriptorManager::GetUnknownDataNodeDescriptor() const
{
  return m_UnknownDataNodeDescriptor;
}
