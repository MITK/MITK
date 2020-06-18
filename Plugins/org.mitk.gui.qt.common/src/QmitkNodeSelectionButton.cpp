/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNodeSelectionButton.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkExtractSliceFilter.h>
#include <vtkMitkLevelWindowFilter.h>
#include <mitkPlanarFigure.h>
#include <mitkPropertyNameHelper.h>

// mitk qt widgets module
#include <QmitkNodeDescriptorManager.h>

// berry includes
#include <berryWorkbenchPlugin.h>
#include <berryQtStyleManager.h>

#include <vtkLookupTable.h>

#include <QEvent>
#include <QPainter>
#include <QTextDocument>

QPixmap GetPixmapFromImageNode(const mitk::DataNode* dataNode, int height)
{
  if (nullptr == dataNode)
  {
    return QPixmap();
  }

  const mitk::Image* image = dynamic_cast<const mitk::Image*>(dataNode->GetData());
  if ((nullptr == image || !image->IsInitialized()) || // -> must be an image
    (image->GetPixelType().GetNumberOfComponents() != 1)) // -> for now only single component are allowed
  {
    auto descManager = QmitkNodeDescriptorManager::GetInstance();
    auto desc = descManager->GetDescriptor(dataNode);
    auto icon = desc->GetIcon(dataNode);
    auto fallBackMap = icon.pixmap(height, height);
    return fallBackMap;
  }

  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  int sliceNumber = image->GetDimension(2) / 2;
  planeGeometry->InitializeStandardPlane(image->GetGeometry(), mitk::PlaneGeometry::Axial, sliceNumber);

  mitk::ExtractSliceFilter::Pointer extractSliceFilter = mitk::ExtractSliceFilter::New();
  extractSliceFilter->SetInput(image);
  extractSliceFilter->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_CUBIC);
  extractSliceFilter->SetResliceTransformByGeometry(image->GetGeometry());
  extractSliceFilter->SetWorldGeometry(planeGeometry);
  extractSliceFilter->SetOutputDimensionality(2);
  extractSliceFilter->SetVtkOutputRequest(true);
  extractSliceFilter->Update();

  vtkImageData* imageData = extractSliceFilter->GetVtkOutput();

  mitk::LevelWindow levelWindow;
  dataNode->GetLevelWindow(levelWindow);
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
  lookupTable->SetSaturationRange(0.0, 0.0);
  lookupTable->SetValueRange(0.0, 1.0);
  lookupTable->SetHueRange(0.0, 0.0);
  lookupTable->SetRampToLinear();

  vtkSmartPointer<vtkMitkLevelWindowFilter> levelWindowFilter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();
  levelWindowFilter->SetLookupTable(lookupTable);
  levelWindowFilter->SetInputData(imageData);
  levelWindowFilter->SetMinOpacity(0.0);
  levelWindowFilter->SetMaxOpacity(1.0);
  int dims[3];
  imageData->GetDimensions(dims);
  double clippingBounds[] = { 0.0, static_cast<double>(dims[0]), 0.0, static_cast<double>(dims[1]) };
  levelWindowFilter->SetClippingBounds(clippingBounds);
  levelWindowFilter->Update();
  imageData = levelWindowFilter->GetOutput();

  QImage thumbnailImage(reinterpret_cast<const unsigned char*>(imageData->GetScalarPointer()), dims[0], dims[1], QImage::Format_ARGB32);

  if (dims[0] > dims[1])
  {
    thumbnailImage = thumbnailImage.scaledToWidth(height, Qt::SmoothTransformation).rgbSwapped();
  }
  else
  {
    thumbnailImage = thumbnailImage.scaledToHeight(height, Qt::SmoothTransformation).rgbSwapped();
  }

  return QPixmap::fromImage(thumbnailImage);
}

QmitkNodeSelectionButton::QmitkNodeSelectionButton(QWidget *parent)
  : QPushButton(parent)
  , m_OutDatedThumbNail(true)
  , m_DataMTime(0)
  , m_IsOptional(true)
  , m_NodeModifiedObserverTag(0)
  , m_NodeObserved(false)
{
}

QmitkNodeSelectionButton::~QmitkNodeSelectionButton()
{
  this->RemoveNodeObserver();
  this->m_SelectedNode = nullptr;
}

void QmitkNodeSelectionButton::AddNodeObserver()
{
  if (this->m_SelectedNode.IsNotNull())
  {
    if (m_NodeObserved)
    {
      MITK_DEBUG << "Invalid observer state in QmitkNodeSelectionButton. There is already a registered observer. Internal logic is not correct. May be an old observer was not removed.";
    }

    auto modifiedCommand = itk::MemberCommand<QmitkNodeSelectionButton>::New();
    modifiedCommand->SetCallbackFunction(this, &QmitkNodeSelectionButton::OnNodeModified);

    // const cast because we need non const nodes and it seems to be the lesser of two evil.
    // the changes to the node are only on the observer level. The other option would be to
    // make the public interface require non const nodes, this we don't want to introduce.
    auto nonconst_node = const_cast<mitk::DataNode*>(this->m_SelectedNode.GetPointer());
    m_NodeModifiedObserverTag = nonconst_node->AddObserver(itk::ModifiedEvent(), modifiedCommand);
    m_NodeObserved = true;
  }
}

void QmitkNodeSelectionButton::RemoveNodeObserver()
{
  if (this->m_SelectedNode.IsNotNull())
  {
    // const cast because we need non const nodes and it seems to be the lesser of two evil.
    // the changes to the node are only on the observer level. The other option would be to
    // make the public interface require non const nodes, this we don't want to introduce.
    auto nonconst_node = const_cast<mitk::DataNode*>(this->m_SelectedNode.GetPointer());
    nonconst_node->RemoveObserver(m_NodeModifiedObserverTag);
  }
  m_NodeObserved = false;
}

void QmitkNodeSelectionButton::OnNodeModified(const itk::Object * /*caller*/, const itk::EventObject & event)
{
  if (itk::ModifiedEvent().CheckEvent(&event))
  {
    this->update();
  }
}

const mitk::DataNode* QmitkNodeSelectionButton::GetSelectedNode() const
{
  return m_SelectedNode;
}

void QmitkNodeSelectionButton::SetSelectedNode(const mitk::DataNode* node)
{
  if (m_SelectedNode != node)
  {
    this->RemoveNodeObserver();
    this->m_SelectedNode = node;
    this->m_OutDatedThumbNail = true;
    this->AddNodeObserver();
  }

  this->update();
}

void QmitkNodeSelectionButton::SetNodeInfo(QString info)
{
  this->m_Info = info;
  this->update();
}

void QmitkNodeSelectionButton::paintEvent(QPaintEvent *p)
{
  QString stylesheet;

  ctkPluginContext* context = berry::WorkbenchPlugin::GetDefault()->GetPluginContext();
  ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
  if (styleManagerRef)
  {
    auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
    stylesheet = styleManager->GetStylesheet();
  }

  QPushButton::paintEvent(p);

  QPainter painter(this);
  QTextDocument td(this);
  td.setDefaultStyleSheet(stylesheet);

  auto widgetSize = this->size();
  QPoint origin = QPoint(5, 5);

  if (this->m_SelectedNode)
  {
    auto iconLength = widgetSize.height() - 10;
    auto node = this->m_SelectedNode;

    itk::ModifiedTimeType dataMTime = 0;
    if (m_SelectedNode->GetData())
    {
      dataMTime = m_SelectedNode->GetData()->GetMTime();
    }
    if (dataMTime>m_DataMTime || this->m_OutDatedThumbNail)
    {
      this->m_ThumbNail = GetPixmapFromImageNode(node, iconLength);
      this->m_OutDatedThumbNail = false;
      m_DataMTime = dataMTime;
    }

    auto thumbNailOrigin = origin;
    thumbNailOrigin.setY(thumbNailOrigin.y() + ((iconLength - m_ThumbNail.height()) / 2));
    painter.drawPixmap(thumbNailOrigin, m_ThumbNail);
    origin.setX(origin.x() + iconLength + 5);

    if (this->isEnabled())
    {
      td.setHtml(QString::fromStdString("<font class=\"normal\">" + node->GetName() + "</font>"));
    }
    else
    {
      td.setHtml(QString::fromStdString("<font class=\"disabled\">" + node->GetName() + "</font>"));
    }
  }
  else
  {
    if (this->isEnabled())
    {
      if (this->m_IsOptional)
      {
        td.setHtml(QString("<font class=\"normal\">") + m_Info + QString("</font>"));
      }
      else
      {
        td.setHtml(QString("<font class=\"warning\">") + m_Info + QString("</font>"));
      }
    }
    else
    {
      td.setHtml(QString("<font class=\"disabled\">") + m_Info + QString("</font>"));
    }
  }

  auto textSize = td.size();

  origin.setY( (widgetSize.height() - textSize.height()) / 2.);

  painter.translate(origin);
  td.drawContents(&painter);
}

void QmitkNodeSelectionButton::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::EnabledChange)
  {
    this->update();
  }
}

bool QmitkNodeSelectionButton::GetSelectionIsOptional() const
{
  return m_IsOptional;
}

void QmitkNodeSelectionButton::SetSelectionIsOptional(bool isOptional)
{
  m_IsOptional = isOptional;
  this->update();
}
