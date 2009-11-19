#include "QmitkNodeDescriptorManager.h"
#include <memory>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAND.h>
#include <mitkNodePredicateDataType.h>
#include <mitkProperties.h>
#include <QList>


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
  mitk::NodePredicateAND::Pointer isBinaryImage = mitk::NodePredicateAND::New(isBinary, isImage);
  this->AddDescriptor(new QmitkNodeDescriptor(tr("ImageMask"), QString(":/Qmitk/Binaerbilder_48.png"), isBinaryImage, this));

  // Adding "PointSet"
  mitk::NodePredicateDataType::Pointer isPointSet = mitk::NodePredicateDataType::New("PointSet");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PointSet"), QString(":/Qmitk/PointSet_48.png"), isPointSet, this));

  // Adding "Surface"
  mitk::NodePredicateDataType::Pointer isSurface = mitk::NodePredicateDataType::New("Surface");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("Surface"), QString(":/Qmitk/Surface_48.png.png"), isSurface, this));

  // Adding "PlanarLine"
  mitk::NodePredicateDataType::Pointer isPlanarLine = mitk::NodePredicateDataType::New("PlanarLine");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PlanarLine"), QString(":/Qmitk/PlanarLine_48.png"), isPlanarLine, this));

  // Adding "PlanarCircle"
  mitk::NodePredicateDataType::Pointer isPlanarCircle = mitk::NodePredicateDataType::New("PlanarCircle");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PlanarCircle"), QString(":/Qmitk/PlanarCircle_48.png"), isPlanarCircle, this));

  // Adding "PlanarAngle"
  mitk::NodePredicateDataType::Pointer isPlanarAngle = mitk::NodePredicateDataType::New("PlanarAngle");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PlanarAngle"), QString(":/Qmitk/PlanarAngle_48.png"), isPlanarAngle, this));

  // Adding "PlanarFourPointAngle"
  mitk::NodePredicateDataType::Pointer isPlanarFourPointAngle = mitk::NodePredicateDataType::New("PlanarFourPointAngle");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PlanarFourPointAngle"), QString(":/Qmitk/PlanarFourPointAngle_48.png"), isPlanarFourPointAngle, this));

  // Adding "PlanarRectangle"
  mitk::NodePredicateDataType::Pointer isPlanarRectangle = mitk::NodePredicateDataType::New("PlanarRectangle");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PlanarRectangle"), QString(":/Qmitk/PlanarRectangle_48.png"), isPlanarRectangle, this));

  // Adding "PlanarPolygon"
  mitk::NodePredicateDataType::Pointer isPlanarPolygon = mitk::NodePredicateDataType::New("PlanarPolygon");
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PlanarPolygon"), QString(":/Qmitk/PlanarPolygon_48.png"), isPlanarPolygon, this));

  // Adding "PlanarPath"
  mitk::NodePredicateProperty::Pointer isNotClosedPolygon
    = mitk::NodePredicateProperty::New("ClosedPlanarPolygon", mitk::BoolProperty::New(false));
  mitk::NodePredicateAND::Pointer isPlanarPath = mitk::NodePredicateAND::New(isNotClosedPolygon, isPlanarPolygon);
  this->AddDescriptor(new QmitkNodeDescriptor(tr("PlanarPath"), QString(":/Qmitk/PlanarPath_48.png"), isPlanarPath, this));

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

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetDescriptor( const mitk::DataTreeNode* _Node ) const
{
  QmitkNodeDescriptor* _Descriptor = m_UnknownDataTreeNodeDescriptor;

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
QList<QAction*> QmitkNodeDescriptorManager::GetActions( const mitk::DataTreeNode* _Node ) const
{
  QList<QAction*> actions = m_UnknownDataTreeNodeDescriptor->GetBatchActions();
  actions.append(m_UnknownDataTreeNodeDescriptor->GetActions());
  QmitkNodeDescriptor* lastDescriptor = m_UnknownDataTreeNodeDescriptor;

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

QList<QAction*> QmitkNodeDescriptorManager::GetActions( const std::vector<mitk::DataTreeNode*>& _Nodes ) const
{
  QList<QAction*> actions = m_UnknownDataTreeNodeDescriptor->GetBatchActions();
  std::vector<QmitkNodeDescriptor*> nodeDescriptors;
  QmitkNodeDescriptor* lastDescriptor;

  // find all descriptors for the nodes (unique)
  for( std::vector<mitk::DataTreeNode*>::const_iterator it = _Nodes.begin()
    ; it != _Nodes.end(); ++it)
  {
    lastDescriptor = this->GetDescriptor(*it);
    if(std::find(nodeDescriptors.begin(), nodeDescriptors.end(), lastDescriptor) == nodeDescriptors.end())
      nodeDescriptors.push_back(lastDescriptor);
  }
  // add all actions for the found descriptors
  lastDescriptor = m_UnknownDataTreeNodeDescriptor;
  for( std::vector<QmitkNodeDescriptor*>::const_iterator it = nodeDescriptors.begin()
    ; it != nodeDescriptors.end(); ++it)
  {
    actions.append(lastDescriptor->GetSeparator());
    lastDescriptor = *it;
    actions.append(lastDescriptor->GetBatchActions());
  }

  return actions;
}

QmitkNodeDescriptorManager::QmitkNodeDescriptorManager()
: m_UnknownDataTreeNodeDescriptor(new QmitkNodeDescriptor("Unknown", QString(":/Qmitk/DataTypeUnknown_48.png"), 0, this))
{
  this->Initialize();
}

QmitkNodeDescriptorManager::~QmitkNodeDescriptorManager()
{
  //delete m_UnknownDataTreeNodeDescriptor;
  //qDeleteAll(m_NodeDescriptors);
}

QmitkNodeDescriptor* QmitkNodeDescriptorManager::GetUnknownDataTreeNodeDescriptor() const
{
  return m_UnknownDataTreeNodeDescriptor;
}
