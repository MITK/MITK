/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNodeDescriptorManager.h"
#include <memory>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <QmitkStyleManager.h>
#include <QList>
#include <QSet>
#include <QResource>

QmitkNodeDescriptorManager* QmitkNodeDescriptorManager::GetInstance()
{
  static QmitkNodeDescriptorManager instance;
  return &instance;
}

void QmitkNodeDescriptorManager::Initialize()
{
  auto isImage = mitk::NodePredicateDataType::New("Image");
  AddDescriptor(new QmitkNodeDescriptor("Image", ":/Qmitk/Images_48.png", isImage, this));

  auto isMultiComponentImage = mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateProperty::New("Image.Displayed Component"));
  AddDescriptor(new QmitkNodeDescriptor("MultiComponentImage", ":/Qmitk/Images_48.png", isMultiComponentImage, this));

  auto isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  auto isBinaryImage = mitk::NodePredicateAnd::New(isBinary, isImage);
  AddDescriptor(new QmitkNodeDescriptor("ImageMask", ":/Qmitk/Binaerbilder_48.png", isBinaryImage, this));

  auto isLabelSetImage = mitk::NodePredicateDataType::New("LabelSetImage");
  AddDescriptor(new QmitkNodeDescriptor("LabelSetImage", ":/Qmitk/LabelSetImage_48.png", isLabelSetImage, this));

  auto segmentationTaskListIcon = QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/SegmentationTaskListIcon.svg"));
  auto isSegmentationTaskList = mitk::NodePredicateDataType::New("SegmentationTaskList");
  AddDescriptor(new QmitkNodeDescriptor("SegmentationTaskList", segmentationTaskListIcon, isSegmentationTaskList, this));

  auto roiIcon = QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/ROIIcon.svg"));
  auto isROI = mitk::NodePredicateDataType::New("ROI");
  AddDescriptor(new QmitkNodeDescriptor("ROI", roiIcon, isROI, this));

  auto geometryDataIcon = QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/GeometryDataIcon.svg"));
  auto isGeometryData = mitk::NodePredicateDataType::New("GeometryData");
  AddDescriptor(new QmitkNodeDescriptor("GeometryData", geometryDataIcon, isGeometryData, this));

  auto isPointSet = mitk::NodePredicateDataType::New("PointSet");
  AddDescriptor(new QmitkNodeDescriptor("PointSet", ":/Qmitk/PointSet_48.png", isPointSet, this));

  auto isSurface = mitk::NodePredicateDataType::New("Surface");
  AddDescriptor(new QmitkNodeDescriptor("Surface", ":/Qmitk/Surface_48.png", isSurface, this));

  auto isNotBinary = mitk::NodePredicateNot::New(isBinary);
  auto isNoneBinaryImage = mitk::NodePredicateAnd::New(isImage, isNotBinary);
  AddDescriptor(new QmitkNodeDescriptor("NoneBinaryImage", ":/Qmitk/Images_48.png", isNoneBinaryImage, this));

  auto isPlanarLine = mitk::NodePredicateDataType::New("PlanarLine");
  AddDescriptor(new QmitkNodeDescriptor("PlanarLine", ":/Qmitk/PlanarLine_48.png", isPlanarLine, this));

  auto isPlanarCircle = mitk::NodePredicateDataType::New("PlanarCircle");
  AddDescriptor(new QmitkNodeDescriptor("PlanarCircle", ":/Qmitk/PlanarCircle_48.png", isPlanarCircle, this));

  auto isPlanarEllipse = mitk::NodePredicateDataType::New("PlanarEllipse");
  AddDescriptor(new QmitkNodeDescriptor("PlanarEllipse", ":/Qmitk/PlanarEllipse_48.png", isPlanarEllipse, this));

  auto isPlanarAngle = mitk::NodePredicateDataType::New("PlanarAngle");
  AddDescriptor(new QmitkNodeDescriptor("PlanarAngle", ":/Qmitk/PlanarAngle_48.png", isPlanarAngle, this));

  auto isPlanarFourPointAngle = mitk::NodePredicateDataType::New("PlanarFourPointAngle");
  AddDescriptor(new QmitkNodeDescriptor("PlanarFourPointAngle", ":/Qmitk/PlanarFourPointAngle_48.png", isPlanarFourPointAngle, this));

  auto isPlanarRectangle = mitk::NodePredicateDataType::New("PlanarRectangle");
  AddDescriptor(new QmitkNodeDescriptor("PlanarRectangle", ":/Qmitk/PlanarRectangle_48.png", isPlanarRectangle, this));

  auto isPlanarPolygon = mitk::NodePredicateDataType::New("PlanarPolygon");
  AddDescriptor(new QmitkNodeDescriptor("PlanarPolygon", ":/Qmitk/PlanarPolygon_48.png", isPlanarPolygon, this));

  auto isNotClosedPolygon = mitk::NodePredicateProperty::New("ClosedPlanarPolygon", mitk::BoolProperty::New(false));
  auto isPlanarPath = mitk::NodePredicateAnd::New(isNotClosedPolygon, isPlanarPolygon);
  AddDescriptor(new QmitkNodeDescriptor("PlanarPath", ":/Qmitk/PlanarPath_48.png", isPlanarPath, this));

  auto isPlanarDoubleEllipse = mitk::NodePredicateDataType::New("PlanarDoubleEllipse");
  AddDescriptor(new QmitkNodeDescriptor("PlanarDoubleEllipse", ":/Qmitk/PlanarDoubleEllipse_48.png", isPlanarDoubleEllipse, this));

  auto isPlanarBezierCurve = mitk::NodePredicateDataType::New("PlanarBezierCurve");
  AddDescriptor(new QmitkNodeDescriptor("PlanarBezierCurve", ":/Qmitk/PlanarBezierCurve_48.png", isPlanarBezierCurve, this));

  auto isPlanarSubdivisionPolygon = mitk::NodePredicateDataType::New("PlanarSubdivisionPolygon");
  AddDescriptor(new QmitkNodeDescriptor("PlanarSubdivisionPolygon", ":/Qmitk/PlanarSubdivisionPolygon_48.png", isPlanarSubdivisionPolygon, this));
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

QmitkNodeDescriptor *QmitkNodeDescriptorManager::GetUnknownDataNodeDescriptor()
{
  return m_UnknownDataNodeDescriptor;
}
