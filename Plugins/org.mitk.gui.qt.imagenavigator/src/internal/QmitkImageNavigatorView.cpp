/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageNavigatorView.h"

#include <itkSpatialOrientationAdapter.h>

#include <QmitkStepperAdapter.h>
#include <QmitkRenderWindow.h>

#include <mitkTimeGeometry.h>

#include <berryConstants.h>
#include <mitkPlaneGeometry.h>
#include <mitkNodePredicateDataType.h>
#include <mitkStatusBar.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkNodePredicateProperty.h>

#include <mitkCompositePixelValueToString.h>

const std::string QmitkImageNavigatorView::VIEW_ID = "org.mitk.views.imagenavigator";

QmitkImageNavigatorView::QmitkImageNavigatorView()
  : m_AxialStepper(nullptr)
  , m_SagittalStepper(nullptr)
  , m_FrontalStepper(nullptr)
  , m_TimeStepper(nullptr)
  , m_Parent(nullptr)
  , m_IRenderWindowPart(nullptr)
{
}

QmitkImageNavigatorView::~QmitkImageNavigatorView()
{
}

void QmitkImageNavigatorView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  connect(m_Controls.m_XWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateValueChanged()));
  connect(m_Controls.m_YWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateValueChanged()));
  connect(m_Controls.m_ZWorldCoordinateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnMillimetreCoordinateValueChanged()));

  m_Parent->setEnabled(false);

  mitk::IRenderWindowPart* renderPart = this->GetRenderWindowPart();
  this->RenderWindowPartActivated(renderPart);
}

void QmitkImageNavigatorView::SetFocus ()
{
  m_Controls.m_XWorldCoordinateSpinBox->setFocus();
}

void QmitkImageNavigatorView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (this->m_IRenderWindowPart != renderWindowPart)
  {
    this->m_IRenderWindowPart = renderWindowPart;
    this->m_Parent->setEnabled(true);

    QmitkRenderWindow* renderWindow = renderWindowPart->GetQmitkRenderWindow("axial");
    if (renderWindow)
    {
      if (m_AxialStepper) m_AxialStepper->deleteLater();
      m_AxialStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorAxial,
                                                     renderWindow->GetSliceNavigationController()->GetSlice(),
                                                     "sliceNavigatorAxialFromSimpleExample");
      m_Controls.m_SliceNavigatorAxial->setEnabled(true);
      m_Controls.m_AxialLabel->setEnabled(true);
      m_Controls.m_ZWorldCoordinateSpinBox->setEnabled(true);
      connect(m_AxialStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
      connect(m_AxialStepper, SIGNAL(Refetch()), this, SLOT(UpdateStatusBar()));
    }
    else
    {
      m_Controls.m_SliceNavigatorAxial->setEnabled(false);
      m_Controls.m_AxialLabel->setEnabled(false);
      m_Controls.m_ZWorldCoordinateSpinBox->setEnabled(false);
    }

    renderWindow = renderWindowPart->GetQmitkRenderWindow("sagittal");
    if (renderWindow)
    {
      if (m_SagittalStepper) m_SagittalStepper->deleteLater();
      m_SagittalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorSagittal,
                                                  renderWindow->GetSliceNavigationController()->GetSlice(),
                                                  "sliceNavigatorSagittalFromSimpleExample");
      m_Controls.m_SliceNavigatorSagittal->setEnabled(true);
      m_Controls.m_SagittalLabel->setEnabled(true);
      m_Controls.m_YWorldCoordinateSpinBox->setEnabled(true);
      connect(m_SagittalStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
      connect(m_SagittalStepper, SIGNAL(Refetch()), this, SLOT(UpdateStatusBar()));
    }
    else
    {
      m_Controls.m_SliceNavigatorSagittal->setEnabled(false);
      m_Controls.m_SagittalLabel->setEnabled(false);
      m_Controls.m_YWorldCoordinateSpinBox->setEnabled(false);
    }

    renderWindow = renderWindowPart->GetQmitkRenderWindow("coronal");
    if (renderWindow)
    {
      if (m_FrontalStepper) m_FrontalStepper->deleteLater();
      m_FrontalStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorFrontal,
                                                 renderWindow->GetSliceNavigationController()->GetSlice(),
                                                 "sliceNavigatorFrontalFromSimpleExample");
      m_Controls.m_SliceNavigatorFrontal->setEnabled(true);
      m_Controls.m_CoronalLabel->setEnabled(true);
      m_Controls.m_XWorldCoordinateSpinBox->setEnabled(true);
      connect(m_FrontalStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
      connect(m_FrontalStepper, SIGNAL(Refetch()), this, SLOT(UpdateStatusBar()));
    }
    else
    {
      m_Controls.m_SliceNavigatorFrontal->setEnabled(false);
      m_Controls.m_CoronalLabel->setEnabled(false);
      m_Controls.m_XWorldCoordinateSpinBox->setEnabled(false);
    }

    mitk::SliceNavigationController* timeController = renderWindowPart->GetTimeNavigationController();
    if (timeController)
    {
      if (m_TimeStepper) m_TimeStepper->deleteLater();
      m_TimeStepper = new QmitkStepperAdapter(m_Controls.m_SliceNavigatorTime,
                                              timeController->GetTime(),
                                              "sliceNavigatorTimeFromSimpleExample");
      m_Controls.m_SliceNavigatorTime->setEnabled(true);
      m_Controls.m_TimeLabel->setEnabled(true);
      connect(m_TimeStepper, SIGNAL(Refetch()), this, SLOT(UpdateStatusBar()));
    }
    else
    {
      m_Controls.m_SliceNavigatorTime->setEnabled(false);
      m_Controls.m_TimeLabel->setEnabled(false);
    }

    this->OnRefetch();
    this->UpdateStatusBar();
  }
}

void QmitkImageNavigatorView::UpdateStatusBar()
{
  if (m_IRenderWindowPart != nullptr)
  {
    mitk::Point3D position = m_IRenderWindowPart->GetSelectedPosition();
    mitk::BaseRenderer::Pointer baseRenderer = mitk::BaseRenderer::GetInstance(m_IRenderWindowPart->GetActiveQmitkRenderWindow()->GetVtkRenderWindow());
    auto globalCurrentTimePoint = baseRenderer->GetTime();
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImageData = mitk::TNodePredicateDataType<mitk::Image>::New();

    mitk::DataStorage::SetOfObjects::ConstPointer nodes = GetDataStorage()->GetSubset(isImageData).GetPointer();

    if (nodes.IsNotNull())
    {
      mitk::Image::Pointer image3D;
      mitk::DataNode::Pointer node;
      mitk::DataNode::Pointer topSourceNode;

      int component = 0;

      node = mitk::FindTopmostVisibleNode(nodes, position, globalCurrentTimePoint, baseRenderer);

      if (node.IsNotNull())
      {
        bool isBinary(false);
        node->GetBoolProperty("binary", isBinary);
        if (isBinary)
        {
          mitk::DataStorage::SetOfObjects::ConstPointer sourcenodes = GetDataStorage()->GetSources(node, nullptr, true);
          if (!sourcenodes->empty())
          {
            topSourceNode = mitk::FindTopmostVisibleNode(sourcenodes, position, globalCurrentTimePoint, baseRenderer);
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
      }

      // get the position and pixel value from the image and build up status bar text
      auto statusBar = mitk::StatusBar::GetInstance();

      if (image3D.IsNotNull() && statusBar != nullptr)
      {
        itk::Index<3> p;
        image3D->GetGeometry()->WorldToIndex(position, p);

        auto pixelType = image3D->GetChannelDescriptor().GetPixelType().GetPixelType();

        if (pixelType == itk::ImageIOBase::RGB || pixelType == itk::ImageIOBase::RGBA)
        {
          std::string pixelValue = "Pixel RGB(A) value: ";
          pixelValue.append(ConvertCompositePixelValueToString(image3D, p));
          statusBar->DisplayImageInfo(position, p, globalCurrentTimePoint, pixelValue.c_str());
        }
        else if (pixelType == itk::ImageIOBase::DIFFUSIONTENSOR3D || pixelType == itk::ImageIOBase::SYMMETRICSECONDRANKTENSOR)
        {
          std::string pixelValue = "See ODF Details view. ";
          statusBar->DisplayImageInfo(position, p, globalCurrentTimePoint, pixelValue.c_str());
        }
        else
        {
          itk::Index<3> p;
          image3D->GetGeometry()->WorldToIndex(position, p);
          mitk::ScalarType pixelValue;
          mitkPixelTypeMultiplex5(
            mitk::FastSinglePixelAccess,
            image3D->GetChannelDescriptor().GetPixelType(),
            image3D,
            image3D->GetVolumeData(image3D->GetTimeGeometry()->TimePointToTimeStep(globalCurrentTimePoint)),
            p,
            pixelValue,
            component);
          statusBar->DisplayImageInfo(position, p, globalCurrentTimePoint, pixelValue);
        }
      }
      else
      {
        statusBar->DisplayImageInfoInvalid();
      }
    }
  }
}

void QmitkImageNavigatorView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  m_IRenderWindowPart = nullptr;
  m_Parent->setEnabled(false);
}

int QmitkImageNavigatorView::GetSizeFlags(bool width)
{
  if(!width)
  {
    return berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL;
  }
  else
  {
    return 0;
  }
}

int QmitkImageNavigatorView::ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult)
{
  if(width==false)
  {
    return 200;
  }
  else
  {
    return preferredResult;
  }
}

int QmitkImageNavigatorView::GetClosestAxisIndex(mitk::Vector3D normal)
{
  // cos(theta) = normal . axis
  // cos(theta) = (a, b, c) . (d, e, f)
  // cos(theta) = (a, b, c) . (1, 0, 0) = a
  // cos(theta) = (a, b, c) . (0, 1, 0) = b
  // cos(theta) = (a, b, c) . (0, 0, 1) = c
  double absCosThetaWithAxis[3];

  for (int i = 0; i < 3; i++)
  {
    absCosThetaWithAxis[i] = fabs(normal[i]);
  }
  int largestIndex = 0;
  double largestValue = absCosThetaWithAxis[0];
  for (int i = 1; i < 3; i++)
  {
    if (absCosThetaWithAxis[i] > largestValue)
    {
      largestValue = absCosThetaWithAxis[i];
      largestIndex = i;
    }
  }
  return largestIndex;
}

void QmitkImageNavigatorView::SetBorderColors()
{
  if (m_IRenderWindowPart)
  {
    QString decoColor;

    QmitkRenderWindow* renderWindow = m_IRenderWindowPart->GetQmitkRenderWindow("axial");
    if (renderWindow)
    {
      decoColor = GetDecorationColorOfGeometry(renderWindow);
      mitk::PlaneGeometry::ConstPointer geometry = renderWindow->GetSliceNavigationController()->GetCurrentPlaneGeometry();
      if (geometry.IsNotNull())
      {
        mitk::Vector3D normal = geometry->GetNormal();
        int axis = this->GetClosestAxisIndex(normal);
        this->SetBorderColor(axis, decoColor);
      }
    }

    renderWindow = m_IRenderWindowPart->GetQmitkRenderWindow("sagittal");
    if (renderWindow)
    {
      decoColor = GetDecorationColorOfGeometry(renderWindow);
      mitk::PlaneGeometry::ConstPointer geometry = renderWindow->GetSliceNavigationController()->GetCurrentPlaneGeometry();
      if (geometry.IsNotNull())
      {
        mitk::Vector3D normal = geometry->GetNormal();
        int axis = this->GetClosestAxisIndex(normal);
        this->SetBorderColor(axis, decoColor);
      }
    }

    renderWindow = m_IRenderWindowPart->GetQmitkRenderWindow("coronal");
    if (renderWindow)
    {
      decoColor = GetDecorationColorOfGeometry(renderWindow);
      mitk::PlaneGeometry::ConstPointer geometry = renderWindow->GetSliceNavigationController()->GetCurrentPlaneGeometry();
      if (geometry.IsNotNull())
      {
        mitk::Vector3D normal = geometry->GetNormal();
        int axis = this->GetClosestAxisIndex(normal);
        this->SetBorderColor(axis, decoColor);
      }
    }
  }
}

QString QmitkImageNavigatorView::GetDecorationColorOfGeometry(QmitkRenderWindow* renderWindow)
{
  QColor color;
  float rgb[3] = {1.0f, 1.0f, 1.0f};
  float rgbMax = 255.0f;
  mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow())->GetCurrentWorldPlaneGeometryNode()->GetColor(rgb);
  color.setRed(static_cast<int>(rgb[0]*rgbMax + 0.5));
  color.setGreen(static_cast<int>(rgb[1]*rgbMax + 0.5));
  color.setBlue(static_cast<int>(rgb[2]*rgbMax + 0.5));
  QString colorAsString = QString(color.name());
  return colorAsString;
}

void QmitkImageNavigatorView::SetBorderColor(int axis, QString colorAsStyleSheetString)
{
  if (axis == 0)
  {
    this->SetBorderColor(m_Controls.m_XWorldCoordinateSpinBox, colorAsStyleSheetString);
  }
  else if (axis == 1)
  {
    this->SetBorderColor(m_Controls.m_YWorldCoordinateSpinBox, colorAsStyleSheetString);
  }
  else if (axis == 2)
  {
    this->SetBorderColor(m_Controls.m_ZWorldCoordinateSpinBox, colorAsStyleSheetString);
  }
}

void QmitkImageNavigatorView::SetBorderColor(QDoubleSpinBox *spinBox, QString colorAsStyleSheetString)
{
  assert(spinBox);
  spinBox->setStyleSheet(QString("border: 2px solid ") + colorAsStyleSheetString + ";");
}

void QmitkImageNavigatorView::SetStepSizes()
{
  this->SetStepSize(0);
  this->SetStepSize(1);
  this->SetStepSize(2);
}

void QmitkImageNavigatorView::SetStepSize(int axis)
{
  if (m_IRenderWindowPart)
  {
    mitk::BaseGeometry::ConstPointer geometry = m_IRenderWindowPart->GetActiveQmitkRenderWindow()->GetSliceNavigationController()->GetInputWorldGeometry3D();

    if (geometry.IsNotNull())
    {
      mitk::Point3D crossPositionInIndexCoordinates;
      mitk::Point3D crossPositionInIndexCoordinatesPlus1;
      mitk::Point3D crossPositionInMillimetresPlus1;
      mitk::Vector3D transformedAxisDirection;

      mitk::Point3D crossPositionInMillimetres = m_IRenderWindowPart->GetSelectedPosition();
      geometry->WorldToIndex(crossPositionInMillimetres, crossPositionInIndexCoordinates);

      crossPositionInIndexCoordinatesPlus1 = crossPositionInIndexCoordinates;
      crossPositionInIndexCoordinatesPlus1[axis] += 1;

      geometry->IndexToWorld(crossPositionInIndexCoordinatesPlus1, crossPositionInMillimetresPlus1);

      transformedAxisDirection = crossPositionInMillimetresPlus1 - crossPositionInMillimetres;

      int closestAxisInMillimetreSpace = this->GetClosestAxisIndex(transformedAxisDirection);
      double stepSize = transformedAxisDirection.GetNorm();
      this->SetStepSize(closestAxisInMillimetreSpace, stepSize);
    }
  }
}

void QmitkImageNavigatorView::SetStepSize(int axis, double stepSize)
{
  if (axis == 0)
  {
    m_Controls.m_XWorldCoordinateSpinBox->setSingleStep(stepSize);
  }
  else if (axis == 1)
  {
    m_Controls.m_YWorldCoordinateSpinBox->setSingleStep(stepSize);
  }
  else if (axis == 2)
  {
    m_Controls.m_ZWorldCoordinateSpinBox->setSingleStep(stepSize);
  }
}

void QmitkImageNavigatorView::OnMillimetreCoordinateValueChanged()
{
  if (m_IRenderWindowPart)
  {
    mitk::TimeGeometry::ConstPointer geometry = m_IRenderWindowPart->GetActiveQmitkRenderWindow()->GetSliceNavigationController()->GetInputWorldTimeGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Point3D positionInWorldCoordinates;
      positionInWorldCoordinates[0] = m_Controls.m_XWorldCoordinateSpinBox->value();
      positionInWorldCoordinates[1] = m_Controls.m_YWorldCoordinateSpinBox->value();
      positionInWorldCoordinates[2] = m_Controls.m_ZWorldCoordinateSpinBox->value();

      m_IRenderWindowPart->SetSelectedPosition(positionInWorldCoordinates);
    }
  }
}

void QmitkImageNavigatorView::OnRefetch()
{
  if (m_IRenderWindowPart)
  {
    mitk::BaseGeometry::ConstPointer geometry = m_IRenderWindowPart->GetActiveQmitkRenderWindow()->GetSliceNavigationController()->GetInputWorldGeometry3D();
    mitk::TimeGeometry::ConstPointer timeGeometry = m_IRenderWindowPart->GetActiveQmitkRenderWindow()->GetSliceNavigationController()->GetInputWorldTimeGeometry();

    if (geometry.IsNull() && timeGeometry.IsNotNull())
    {
      mitk::TimeStepType timeStep = m_IRenderWindowPart->GetActiveQmitkRenderWindow()->GetSliceNavigationController()->GetTime()->GetPos();
      geometry = timeGeometry->GetGeometryForTimeStep(timeStep);
      SetVisibilityOfTimeSlider(timeGeometry->CountTimeSteps());
    }

    if (geometry.IsNotNull())
    {
      mitk::BoundingBox::BoundsArrayType bounds = geometry->GetBounds();

      mitk::Point3D cornerPoint1InIndexCoordinates;
      cornerPoint1InIndexCoordinates[0] = bounds[0];
      cornerPoint1InIndexCoordinates[1] = bounds[2];
      cornerPoint1InIndexCoordinates[2] = bounds[4];

      mitk::Point3D cornerPoint2InIndexCoordinates;
      cornerPoint2InIndexCoordinates[0] = bounds[1];
      cornerPoint2InIndexCoordinates[1] = bounds[3];
      cornerPoint2InIndexCoordinates[2] = bounds[5];

      if (!geometry->GetImageGeometry())
      {
        cornerPoint1InIndexCoordinates[0] += 0.5;
        cornerPoint1InIndexCoordinates[1] += 0.5;
        cornerPoint1InIndexCoordinates[2] += 0.5;
        cornerPoint2InIndexCoordinates[0] -= 0.5;
        cornerPoint2InIndexCoordinates[1] -= 0.5;
        cornerPoint2InIndexCoordinates[2] -= 0.5;
      }

      mitk::Point3D crossPositionInWorldCoordinates = m_IRenderWindowPart->GetSelectedPosition();

      mitk::Point3D cornerPoint1InWorldCoordinates;
      mitk::Point3D cornerPoint2InWorldCoordinates;

      geometry->IndexToWorld(cornerPoint1InIndexCoordinates, cornerPoint1InWorldCoordinates);
      geometry->IndexToWorld(cornerPoint2InIndexCoordinates, cornerPoint2InWorldCoordinates);

      m_Controls.m_XWorldCoordinateSpinBox->blockSignals(true);
      m_Controls.m_YWorldCoordinateSpinBox->blockSignals(true);
      m_Controls.m_ZWorldCoordinateSpinBox->blockSignals(true);

      m_Controls.m_XWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]));
      m_Controls.m_YWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]));
      m_Controls.m_ZWorldCoordinateSpinBox->setMinimum(std::min(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]));
      m_Controls.m_XWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]));
      m_Controls.m_YWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]));
      m_Controls.m_ZWorldCoordinateSpinBox->setMaximum(std::max(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]));

      m_Controls.m_XWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[0]);
      m_Controls.m_YWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[1]);
      m_Controls.m_ZWorldCoordinateSpinBox->setValue(crossPositionInWorldCoordinates[2]);

      m_Controls.m_XWorldCoordinateSpinBox->blockSignals(false);
      m_Controls.m_YWorldCoordinateSpinBox->blockSignals(false);
      m_Controls.m_ZWorldCoordinateSpinBox->blockSignals(false);

      /// Calculating 'inverse direction' property.

      mitk::AffineTransform3D::MatrixType matrix = geometry->GetIndexToWorldTransform()->GetMatrix();
      matrix.GetVnlMatrix().normalize_columns();
      mitk::AffineTransform3D::MatrixType::InternalMatrixType inverseMatrix = matrix.GetInverse();

      for (int worldAxis = 0; worldAxis < 3; ++worldAxis)
      {
        QmitkRenderWindow* renderWindow =
            worldAxis == 0 ? m_IRenderWindowPart->GetQmitkRenderWindow("sagittal") :
            worldAxis == 1 ? m_IRenderWindowPart->GetQmitkRenderWindow("coronal") :
                             m_IRenderWindowPart->GetQmitkRenderWindow("axial");

        if (renderWindow)
        {
          const mitk::BaseGeometry* rendererGeometry = renderWindow->GetRenderer()->GetCurrentWorldGeometry();

          /// Because of some problems with the current way of event signalling,
          /// 'Modified' events are sent out from the stepper while the renderer
          /// does not have a geometry yet. Therefore, we do a nullptr check here.
          /// See bug T22122. This check can be resolved after T22122 got fixed.
          if (rendererGeometry)
          {
            int dominantAxis = itk::Function::Max3(
                inverseMatrix[0][worldAxis],
                inverseMatrix[1][worldAxis],
                inverseMatrix[2][worldAxis]);

            bool referenceGeometryAxisInverted = inverseMatrix[dominantAxis][worldAxis] < 0;
            bool rendererZAxisInverted = rendererGeometry->GetAxisVector(2)[worldAxis] < 0;

            /// `referenceGeometryAxisInverted` tells if the direction of the corresponding axis
            /// of the reference geometry is flipped compared to the 'world direction' or not.
            ///
            /// `rendererZAxisInverted` tells if direction of the renderer geometry z axis is
            /// flipped compared to the 'world direction' or not. This is the same as the indexing
            /// direction in the slice navigation controller and matches the 'top' property when
            /// initialising the renderer planes. (If 'top' was true then the direction is
            /// inverted.)
            ///
            /// The world direction can be +1 ('up') that means right, anterior or superior, or
            /// it can be -1 ('down') that means left, posterior or inferior, respectively.
            ///
            /// If these two do not match, we have to invert the index between the slice navigation
            /// controller and the slider navigator widget, so that the user can see and control
            /// the index according to the reference geometry, rather than the slice navigation
            /// controller. The index in the slice navigation controller depends on in which way
            /// the reference geometry has been resliced for the renderer, and it does not necessarily
            /// match neither the world direction, nor the direction of the corresponding axis of
            /// the reference geometry. Hence, it is a merely internal information that should not
            /// be exposed to the GUI.
            ///
            /// So that one can navigate in the same world direction by dragging the slider
            /// right, regardless of the direction of the corresponding axis of the reference
            /// geometry, we invert the direction of the controls if the reference geometry axis
            /// is inverted but the direction is not ('inversDirection' is false) or the other
            /// way around.

            bool inverseDirection = referenceGeometryAxisInverted != rendererZAxisInverted;

            QmitkSliderNavigatorWidget* navigatorWidget =
                worldAxis == 0 ? m_Controls.m_SliceNavigatorSagittal :
                worldAxis == 1 ? m_Controls.m_SliceNavigatorFrontal :
                                 m_Controls.m_SliceNavigatorAxial;

            navigatorWidget->SetInverseDirection(inverseDirection);

            // This should be a preference (see T22254)
            // bool invertedControls = referenceGeometryAxisInverted != inverseDirection;
            // navigatorWidget->SetInvertedControls(invertedControls);
          }
        }
      }
    }

    this->SetBorderColors();

  }
}


void QmitkImageNavigatorView::SetVisibilityOfTimeSlider(std::size_t timeSteps)
{
  m_Controls.m_SliceNavigatorTime->setVisible(timeSteps > 1);
  m_Controls.m_TimeLabel->setVisible(timeSteps > 1);
}
