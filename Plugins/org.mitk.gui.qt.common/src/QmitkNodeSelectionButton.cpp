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


#include "QmitkNodeSelectionButton.h"

// berry includes
#include <berryWorkbenchPlugin.h>
#include <berryQtStyleManager.h>

#include "QPainter"
#include "QTextDocument"

#include <mitkDataNode.h>
#include <QmitkNodeDescriptorManager.h>

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkExtractSliceFilter.h>
#include <vtkMitkLevelWindowFilter.h>
#include <mitkPlanarFigure.h>
#include <mitkPropertyNameHelper.h>

// vtk
#include <vtkLookupTable.h>

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

  thumbnailImage = thumbnailImage.scaledToHeight(height,Qt::SmoothTransformation).rgbSwapped();
  return QPixmap::fromImage(thumbnailImage);
}

QmitkNodeSelectionButton::QmitkNodeSelectionButton(QWidget *parent)
  : QPushButton(parent), m_OutDatedThumpNail(true)
{ }

QmitkNodeSelectionButton::~QmitkNodeSelectionButton()
{
  this->m_SelectedNode = nullptr;
}

const mitk::DataNode* QmitkNodeSelectionButton::GetSelectedNode() const
{
  return m_SelectedNode;
}

void QmitkNodeSelectionButton::SetSelectedNode(const mitk::DataNode* node)
{
  if (m_SelectedNode != node)
  {
    this->m_SelectedNode = node;
    this->m_OutDatedThumpNail = true;
  }

  this->update();
};

void QmitkNodeSelectionButton::SetNodeInfo(QString info)
{
  this->m_Info = info;
  this->update();
};

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

    if (this->m_OutDatedThumpNail)
    {
      this->m_ThumpNail = GetPixmapFromImageNode(node, iconLength);
      this->m_OutDatedThumpNail = false;
    }

    painter.drawPixmap(origin, m_ThumpNail);
    origin.setX(origin.x() + iconLength + 5);

    td.setHtml(QString::fromStdString("<font class=\"normal\">"+node->GetName()+"</font>"));
  }
  else
  {
    td.setHtml(m_Info);
  }

  auto textSize = td.size();

  origin.setY( (widgetSize.height() - textSize.height()) / 2.);

  painter.translate(origin);
  td.drawContents(&painter);

}
