/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPixelValueView.h"

#include <mitkNodePredicateDataType.h>
#include <mitkImage.h>
#include <mitkCompositePixelValueToString.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>

#include <ui_QmitkPixelValueView.h>

const std::string QmitkPixelValueView::VIEW_ID = "org.mitk.views.pixelvalue";

QmitkPixelValueView::QmitkPixelValueView(QObject*)
  : m_Ui(new Ui::QmitkPixelValueView)
{
}

QmitkPixelValueView::~QmitkPixelValueView()
{
}

void QmitkPixelValueView::CreateQtPartControl(QWidget* parent)
{
  m_Ui->setupUi(parent);

  this->m_SliceNavigationListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceNavigationListener, &QmitkSliceNavigationListener::SelectedPositionChanged, this, &QmitkPixelValueView::OnSelectedPositionChanged);
  connect(&m_SliceNavigationListener, &QmitkSliceNavigationListener::SelectedTimePointChanged, this, &QmitkPixelValueView::OnSelectedTimePointChanged);

  this->Update();
}

void QmitkPixelValueView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkPixelValueView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkPixelValueView::RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartInputChanged(renderWindowPart);
}

void QmitkPixelValueView::OnSelectedPositionChanged(const mitk::Point3D&)
{
  this->Update();
}

void QmitkPixelValueView::OnSelectedTimePointChanged(const mitk::TimePointType&)
{
  this->Update();
}

void QmitkPixelValueView::NodeChanged(const mitk::DataNode*)
{
  this->Update();
}

void QmitkPixelValueView::Update()
{
  const auto position = m_SliceNavigationListener.GetCurrentSelectedPosition();
  const auto timePoint = m_SliceNavigationListener.GetCurrentSelectedTimePoint();
  
  auto isImageData = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto nodes = GetDataStorage()->GetSubset(isImageData);

  if (nodes.IsNull())
  {
    this->Clear();
    return;
  }

  mitk::Image::Pointer image;
  mitk::DataNode::Pointer topParentNode;

  int component = 0;

  auto node = mitk::FindTopmostVisibleNode(nodes, position, timePoint, nullptr);

  if (node.IsNull())
  {
    this->Clear();
    return;
  }

  bool isBinary = false;
  node->GetBoolProperty("binary", isBinary);

  if (isBinary)
  {
    auto parentNodes = GetDataStorage()->GetSources(node, nullptr, true);

    if (!parentNodes->empty())
      topParentNode = FindTopmostVisibleNode(nodes, position, timePoint, nullptr);

    if (topParentNode.IsNotNull())
    {
      image = dynamic_cast<mitk::Image*>(topParentNode->GetData());
      topParentNode->GetIntProperty("Image.Displayed Component", component);
    }
    else
    {
      image = dynamic_cast<mitk::Image*>(node->GetData());
      node->GetIntProperty("Image.Displayed Component", component);
    }
  }
  else
  {
    image = dynamic_cast<mitk::Image*>(node->GetData());
    node->GetIntProperty("Image.Displayed Component", component);
  }

  if (image.IsNotNull())
  {
    m_Ui->imageNameLineEdit->setText(QString::fromStdString(node->GetName()));

    itk::Index<3> index;
    image->GetGeometry()->WorldToIndex(position, index);

    auto pixelType = image->GetChannelDescriptor().GetPixelType().GetPixelType();

    if (pixelType == itk::IOPixelEnum::RGB || pixelType == itk::IOPixelEnum::RGBA)
    {
      m_Ui->pixelValueLineEdit->setText(QString::fromStdString(mitk::ConvertCompositePixelValueToString(image, index)));
      return;
    }

    if (pixelType == itk::IOPixelEnum::DIFFUSIONTENSOR3D || pixelType == itk::IOPixelEnum::SYMMETRICSECONDRANKTENSOR)
    {
      m_Ui->pixelValueLineEdit->setText(QStringLiteral("See ODF Details view."));
      return;
    }

    mitk::ScalarType pixelValue = 0.0;

    mitkPixelTypeMultiplex5(
      mitk::FastSinglePixelAccess,
      image->GetChannelDescriptor().GetPixelType(),
      image,
      image->GetVolumeData(image->GetTimeGeometry()->TimePointToTimeStep(timePoint)),
      index,
      pixelValue,
      component);

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream.precision(2);

    if (fabs(pixelValue) > 1000000 || fabs(pixelValue) < 0.01)
    {
      stream << std::scientific;
    }
    else
    {
      stream << std::fixed;
    }

    stream << pixelValue;

    m_Ui->pixelValueLineEdit->setText(QString::fromStdString(stream.str()));
  }
  else
  {
    this->Clear();
  }
}

void QmitkPixelValueView::Clear()
{
  m_Ui->imageNameLineEdit->clear();
  m_Ui->pixelValueLineEdit->clear();
}

void QmitkPixelValueView::SetFocus()
{
}
