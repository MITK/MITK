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

#include "QmitkNodeDescriptorManager.h"
#include <memory>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateDataType.h>
#include <mitkProperties.h>

#include <QList>
#include <QSet>


QmitkNodeDescriptorManager* QmitkNodeDescriptorManager::GetInstance()
{
  static QmitkNodeDescriptorManager _Instance;
  return &_Instance; 
/*
  static std::auto_ptr<QmitkNodeDescriptorManager> instance;
  if(instance.get() == 0)
  {
    instance.reset(new QmitkNodeDescriptorManager());
    instance->Initialize();
  }
  return instance.get();*/

}

void QmitkNodeDescriptorManager::Initialize()
{
  // Adding "Images"
  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("Image"), QString(":/Qmitk/Images_48.png"), isImage, this));

  // Adding "Image Masks"
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isBinaryImage = mitk::NodePredicateAnd::New(isBinary, isImage);
  this->AddDescriptor(new QmitkNodeDescriptor(tr("ImageMask"), QString(":/Qmitk/Binaerbilder_48.png"), isBinaryImage, this));

  // Adding "PointSet"
  mitk::NodePredicateDataType::Pointer isPointSet = mitk::NodePredicateDataType::New("PointSet");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PointSet"), QString(":/Qmitk/PointSet_48.png"), isPointSet, this));

  // Adding "Surface"
  mitk::NodePredicateDataType::Pointer isSurface = mitk::NodePredicateDataType::New("Surface");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("Surface"), QString(":/Qmitk/Surface_48.png"), isSurface, this));

  // Adding "NoneBinaryImages"
  mitk::NodePredicateNot::Pointer isNotBinary = mitk::NodePredicateNot::New(isBinary);
  mitk::NodePredicateAnd::Pointer isNoneBinaryImage = mitk::NodePredicateAnd::New(isImage, isNotBinary);
  this->AddDescriptor(new QmitkNodeDescriptor(tr("NoneBinaryImage"), QString(":/Qmitk/Images_48.png"), isNoneBinaryImage, this));

}

void QmitkNodeDescriptorManager::AddDescriptor( QmitkNodeDescriptor* _Descriptor )
{
  _Descriptor->setParent(this);
  m_NodeDescriptors.push_back(_Descriptor);
}

void QmitkNodeDescriptorManager::RemoveDescriptor( QmitkNodeDescriptor* _Descriptor )
{
  int index = m_NodeDescriptors.indexOf(_Descriptor);

  if(index != -1)
  {
    m_NodeDescriptors.removeAt(index);
    _Descriptor->setParent(0);
    delete _Descriptor;
  }
  
}

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetDescriptor( const mitk::DataNode* _Node ) const
{
  QmitkNodeDescriptor* _Descriptor = m_UnknownDataNodeDescriptor;

  for(QList<QmitkNodeDescriptor*>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
  {
    if((*it)->CheckNode(_Node))
      _Descriptor = *it;
  }

  return _Descriptor;
}

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetDescriptor( const QString& _ClassName ) const
{
  QmitkNodeDescriptor* _Descriptor = 0;

  for(QList<QmitkNodeDescriptor*>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
  {
    if((*it)->GetClassName() == _ClassName)
      _Descriptor = *it;
  }

  return _Descriptor;
}
QList<QAction*> QmitkNodeDescriptorManager::GetActions( const mitk::DataNode* _Node ) const
{
  QList<QAction*> actions = m_UnknownDataNodeDescriptor->GetBatchActions();
  actions.append(m_UnknownDataNodeDescriptor->GetActions());
  QmitkNodeDescriptor* lastDescriptor = m_UnknownDataNodeDescriptor;

  for(QList<QmitkNodeDescriptor*>::const_iterator it = m_NodeDescriptors.begin(); it != m_NodeDescriptors.end(); ++it)
  {
    if((*it)->CheckNode(_Node))
    {
      actions.append(lastDescriptor->GetSeparator());
      lastDescriptor = *it;
      actions.append(lastDescriptor->GetBatchActions());
      actions.append(lastDescriptor->GetActions());
    }
  }

  return actions;
}

QList<QAction*> QmitkNodeDescriptorManager::GetActions( const QList<mitk::DataNode::Pointer> &_Nodes ) const
{
  QList<QAction*> actions = m_UnknownDataNodeDescriptor->GetBatchActions();
  QSet<QmitkNodeDescriptor*> nodeDescriptors;
  QmitkNodeDescriptor* lastDescriptor;

  // find all descriptors for the nodes (unique)
  foreach (mitk::DataNode::Pointer node, _Nodes)
  {
    lastDescriptor = this->GetDescriptor(node);
    nodeDescriptors.insert(lastDescriptor);
  }
  // add all actions for the found descriptors
  lastDescriptor = m_UnknownDataNodeDescriptor;
  foreach (QmitkNodeDescriptor* descr, nodeDescriptors)
  {
    actions.append(lastDescriptor->GetSeparator());
    lastDescriptor = descr;
    actions.append(lastDescriptor->GetBatchActions());
  }

  return actions;
}

QmitkNodeDescriptorManager::QmitkNodeDescriptorManager()
: m_UnknownDataNodeDescriptor(new QmitkNodeDescriptor("Unknown", QString(":/Qmitk/DataTypeUnknown_48.png"), 0, this))
{
  this->Initialize();
}

QmitkNodeDescriptorManager::~QmitkNodeDescriptorManager()
{
  //delete m_UnknownDataNodeDescriptor;
  //qDeleteAll(m_NodeDescriptors);
}

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetUnknownDataNodeDescriptor() const
{
  return m_UnknownDataNodeDescriptor;
}
