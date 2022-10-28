/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVoxelValueView.h"

#include <mitkNodePredicateDataType.h>
#include <mitkImage.h>
#include <mitkCompositePixelValueToString.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>

#include <string.h>


const std::string QmitkVoxelValueView::VIEW_ID = "org.mitk.views.qmitkvoxelvalue";

QmitkVoxelValueView::QmitkVoxelValueView(QObject*)
  : m_Layout(nullptr)
  , m_Label(nullptr)
  , m_TextWidget(nullptr)
{
}

QmitkVoxelValueView::~QmitkVoxelValueView()
{
}

void QmitkVoxelValueView::CreateQtPartControl(QWidget *parent)
{
  m_Layout = new QHBoxLayout(parent);
  m_Layout->setMargin(0);

  m_Label = new QLabel(parent);
  m_Label->setText("Pixel value:");
  m_Layout->addWidget(m_Label);

  m_TextWidget = new QLineEdit(parent);
  m_TextWidget->setReadOnly(true);
  m_Layout->addWidget(m_TextWidget);

  this->m_SliceNavigationListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceNavigationListener, &QmitkSliceNavigationListener::SliceChanged, this, &QmitkVoxelValueView::OnSliceChanged);
}

void QmitkVoxelValueView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkVoxelValueView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_SliceNavigationListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkVoxelValueView::OnSliceChanged()
{
  const auto position = m_SliceNavigationListener.GetCurrentSelectedPosition();
  const auto timePoint = m_SliceNavigationListener.GetCurrentSelectedTimePoint();
  
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImageData = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = GetDataStorage()->GetSubset(isImageData).GetPointer();
  if (nodes.IsNull())
  {
    return;
  }

  mitk::Image::Pointer image3D;
  mitk::DataNode::Pointer node;
  mitk::DataNode::Pointer topSourceNode;

  int component = 0;

  node = mitk::FindTopmostVisibleNode(nodes, position, timePoint, nullptr);
  if (node.IsNull())
  {
    return;
  }

  bool isBinary(false);
  node->GetBoolProperty("binary", isBinary);
  if (isBinary)
  {
    mitk::DataStorage::SetOfObjects::ConstPointer sourcenodes = GetDataStorage()->GetSources(node, nullptr, true);
    if (!sourcenodes->empty())
    {
      topSourceNode = FindTopmostVisibleNode(nodes, position, timePoint, nullptr);
    }
    if (topSourceNode.IsNotNull())
    {
      image3D = dynamic_cast<mitk::Image*>(topSourceNode->GetData());
      topSourceNode->GetIntProperty("Image.Displayed Component", component);
    }
    else
    {
      image3D = dynamic_cast<mitk::Image*>(node->GetData());
      node->GetIntProperty("Image.Displayed Component", component);
    }
  }
  else
  {
    image3D = dynamic_cast<mitk::Image*>(node->GetData());
    node->GetIntProperty("Image.Displayed Component", component);
  }

  // get the position and pixel value from the image and build up voxel view text
  std::string voxelText = "";
  if (image3D.IsNotNull())
  {
    itk::Index<3> p;
    image3D->GetGeometry()->WorldToIndex(position, p);

    auto pixelType = image3D->GetChannelDescriptor().GetPixelType().GetPixelType();
    if (pixelType == itk::IOPixelEnum::RGB || pixelType == itk::IOPixelEnum::RGBA)
    {
      m_Label->setText("Pixel RGB(A) value: ");
      voxelText.append(ConvertCompositePixelValueToString(image3D, p));
    }
    else if (pixelType == itk::IOPixelEnum::DIFFUSIONTENSOR3D || pixelType == itk::IOPixelEnum::SYMMETRICSECONDRANKTENSOR)
    {
      m_Label->setText("See ODF Details view. ");
    }
    else
    {
      m_Label->setText("Pixel value: ");
      mitk::ScalarType pixelValue;
      mitkPixelTypeMultiplex5(
        mitk::FastSinglePixelAccess,
        image3D->GetChannelDescriptor().GetPixelType(),
        image3D,
        image3D->GetVolumeData(image3D->GetTimeGeometry()->TimePointToTimeStep(timePoint)),
        p,
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

      voxelText = stream.str();
    }

    m_TextWidget->setText(QString(voxelText.c_str()));
  }
  else
  {
    m_TextWidget->setText(QString(""));
  }
}
