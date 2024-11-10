/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageNavigatorView.h"
#include <ui_QmitkImageNavigatorView.h>

#include <QmitkRenderWindow.h>
#include <QmitkStepperAdapter.h>

#include <mitkPlaneGeometry.h>
#include <mitkTimeGeometry.h>
#include <mitkTimeNavigationController.h>

#include <berryConstants.h>

#include <itkSpatialOrientationAdapter.h>

const std::string QmitkImageNavigatorView::VIEW_ID = "org.mitk.views.imagenavigator";

QmitkImageNavigatorView::QmitkImageNavigatorView()
  : m_Ui(new Ui::QmitkImageNavigatorView),
    m_TimeStepperAdapter(nullptr),
    m_RenderWindowPart(nullptr),
    m_Parent(nullptr)
{
}

QmitkImageNavigatorView::~QmitkImageNavigatorView()
{
}

void QmitkImageNavigatorView::CreateQtPartControl(QWidget *parent)
{
  using Self = QmitkImageNavigatorView;

  m_Parent = parent;
  m_Ui->setupUi(parent);

  m_AxisUi = {{
    { "coronal", m_Ui->xWorldCoordSpinBox, m_Ui->coronalLabel, m_Ui->coronalSliceNavWidget, nullptr },
    { "sagittal", m_Ui->yWorldCoordSpinBox, m_Ui->sagittalLabel, m_Ui->sagittalSliceNavWidget, nullptr },
    { "axial", m_Ui->zWorldCoordSpinBox, m_Ui->axialLabel, m_Ui->axialSliceNavWidget, nullptr }
  }};

  for (const auto& axisControls : m_AxisUi)
    connect(axisControls.SpinBox, &QDoubleSpinBox::valueChanged, this, &Self::OnCoordValueChanged);

  parent->setEnabled(false);

  this->RenderWindowPartActivated(this->GetRenderWindowPart());

  auto timeController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();

  if (m_TimeStepperAdapter)
    m_TimeStepperAdapter->deleteLater();

  m_TimeStepperAdapter = new QmitkStepperAdapter(m_Ui->timeSliceNavWidget, timeController->GetStepper());

  m_Ui->timeSliceNavWidget->setEnabled(true);
  m_Ui->timeLabel->setEnabled(true);

  connect(m_TimeStepperAdapter, &QmitkStepperAdapter::Refetch, this, &Self::OnRefetch);
}

void QmitkImageNavigatorView::SetFocus()
{
  m_Ui->xWorldCoordSpinBox->setFocus();
}

void QmitkImageNavigatorView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart != renderWindowPart)
  {
    m_RenderWindowPart = renderWindowPart;
    m_Parent->setEnabled(true);

    for (size_t i = 0; i < 3; ++i)
    {
      auto renderWindow = renderWindowPart->GetQmitkRenderWindow(m_AxisUi[i].RenderWindowId);
      const bool hasRenderWindow = renderWindow != nullptr;

      if (hasRenderWindow)
      {
        if (m_AxisUi[i].StepperAdapter != nullptr)
          m_AxisUi[i].StepperAdapter->deleteLater();

        m_AxisUi[i].StepperAdapter = new QmitkStepperAdapter(
          m_AxisUi[i].SliceNavWidget, renderWindow->GetSliceNavigationController()->GetStepper());

        connect(m_AxisUi[i].StepperAdapter, &QmitkStepperAdapter::Refetch, this, &QmitkImageNavigatorView::OnRefetch);
      }

      m_AxisUi[i].SliceNavWidget->setEnabled(hasRenderWindow);
      m_AxisUi[i].Label->setEnabled(hasRenderWindow);
      m_AxisUi[i].SpinBox->setEnabled(hasRenderWindow);
    }

    this->OnRefetch();
  }
}

void QmitkImageNavigatorView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
  m_RenderWindowPart = nullptr;
  m_Parent->setEnabled(false);
}

int QmitkImageNavigatorView::GetSizeFlags(bool width)
{
  return !width
    ? berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL
    : 0;
}

int QmitkImageNavigatorView::ComputePreferredSize(bool width, int, int, int preferredResult)
{
  return width
    ? preferredResult
    : 200;
}

int QmitkImageNavigatorView::GetClosestAxisIndex(const mitk::Vector3D& normal)
{
  // cos(theta) = normal . axis
  // cos(theta) = (a, b, c) . (d, e, f)
  // cos(theta) = (a, b, c) . (1, 0, 0) = a
  // cos(theta) = (a, b, c) . (0, 1, 0) = b
  // cos(theta) = (a, b, c) . (0, 0, 1) = c

  std::array<double, 3> absCosThetaWithAxis;

  for (int i = 0; i < 3; ++i)
    absCosThetaWithAxis[i] = std::fabs(normal[i]);

  int largestIndex = 0;
  double largestValue = absCosThetaWithAxis[0];

  for (int i = 1; i < 3; ++i)
  {
    if (absCosThetaWithAxis[i] > largestValue)
    {
      largestValue = absCosThetaWithAxis[i];
      largestIndex = i;
    }
  }

  return largestIndex;
}

void QmitkImageNavigatorView::SetBorderColor(QmitkRenderWindow* renderWindow)
{
  if (renderWindow == nullptr)
    return;

  if (auto planeGeometry = renderWindow->GetSliceNavigationController()->GetCurrentPlaneGeometry(); planeGeometry != nullptr)
  {
    int axis = this->GetClosestAxisIndex(planeGeometry->GetNormal());
    const auto hexColor = this->GetDecorationColorOfGeometry(renderWindow);

    this->SetBorderColor(axis, hexColor);
  }
}

void QmitkImageNavigatorView::SetBorderColors()
{
  if (m_RenderWindowPart == nullptr)
    return;

  this->SetBorderColor(m_RenderWindowPart->GetQmitkRenderWindow("axial"));
  this->SetBorderColor(m_RenderWindowPart->GetQmitkRenderWindow("sagittal"));
  this->SetBorderColor(m_RenderWindowPart->GetQmitkRenderWindow("coronal"));
}

QString QmitkImageNavigatorView::GetDecorationColorOfGeometry(QmitkRenderWindow* renderWindow)
{
  std::array<float, 3> rgb = { 1.0f, 1.0f, 1.0f };
  const float RGB_MAX = 255.0f;

  auto baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow());
  baseRenderer->GetCurrentWorldPlaneGeometryNode()->GetColor(rgb.data());

  QColor color(
    static_cast<int>(rgb[0] * RGB_MAX + 0.5f),
    static_cast<int>(rgb[1] * RGB_MAX + 0.5f),
    static_cast<int>(rgb[2] * RGB_MAX + 0.5f));

  return color.name();
}

void QmitkImageNavigatorView::SetBorderColor(int axis, const QString& hexColor)
{
  if (axis >= 0 && axis < 3)
    this->SetBorderColor(m_AxisUi[axis].SpinBox, hexColor);
}

void QmitkImageNavigatorView::SetBorderColor(QDoubleSpinBox* spinBox, const QString& hexColor)
{
  if (spinBox != nullptr)
    spinBox->setStyleSheet(QString("border: 2px solid ") + hexColor + ";");
}

void QmitkImageNavigatorView::OnCoordValueChanged()
{
  if (m_RenderWindowPart != nullptr)
  {
    auto activeRenderWindow = m_RenderWindowPart->GetActiveQmitkRenderWindow();
    auto sliceNavController = activeRenderWindow->GetSliceNavigationController();
    mitk::TimeGeometry::ConstPointer geometry = sliceNavController->GetInputWorldTimeGeometry();

    if (geometry.IsNotNull())
    {
      mitk::Point3D positionInWorldCoordinates;

      for (size_t i = 0; i < 3; ++i)
        positionInWorldCoordinates[i] = m_AxisUi[i].SpinBox->value();

      m_RenderWindowPart->SetSelectedPosition(positionInWorldCoordinates);
    }
  }
}

void QmitkImageNavigatorView::OnRefetch()
{
  if (m_RenderWindowPart == nullptr)
    return;

  auto activeRenderWindow = m_RenderWindowPart->GetActiveQmitkRenderWindow();
  auto sliceNavController = activeRenderWindow->GetSliceNavigationController();
  mitk::TimeGeometry::ConstPointer timeGeometry = sliceNavController->GetInputWorldTimeGeometry();

  if (timeGeometry.IsNull())
    return;

  const auto* timeNavController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();

  this->SetVisibilityOfTimeSlider(timeNavController->GetStepper()->GetSteps());

  const auto timeStep = timeNavController->GetSelectedTimeStep();
  auto geometry = timeGeometry->GetGeometryForTimeStep(timeStep);

  if (geometry.IsNotNull())
  {
    auto bounds = geometry->GetBounds();

    auto indexCornerPts = std::make_pair(
      mitk::Point3D(bounds[0], bounds[2], bounds[4]),
      mitk::Point3D(bounds[1], bounds[3], bounds[5])
    );

    if (!geometry->GetImageGeometry())
    {
      indexCornerPts.first += mitk::Vector3D(0.5);
      indexCornerPts.second -= mitk::Vector3D(0.5);
    }

    std::pair<mitk::Point3D, mitk::Point3D> worldCornerPts;
    geometry->IndexToWorld(indexCornerPts.first, worldCornerPts.first);
    geometry->IndexToWorld(indexCornerPts.second, worldCornerPts.second);

    auto worldCrossPos = m_RenderWindowPart->GetSelectedPosition();

    for (size_t i = 0; i < 3; ++i)
    {
      m_AxisUi[i].SpinBox->blockSignals(true);
      m_AxisUi[i].SpinBox->setMinimum(std::min(worldCornerPts.first[i], worldCornerPts.second[i]));
      m_AxisUi[i].SpinBox->setMaximum(std::max(worldCornerPts.first[i], worldCornerPts.second[i]));
      m_AxisUi[i].SpinBox->setValue(worldCrossPos[i]);
      m_AxisUi[i].SpinBox->blockSignals(false);
    }

    /// Calculating 'inverse direction' property.

    auto matrix = geometry->GetIndexToWorldTransform()->GetMatrix();
    matrix.GetVnlMatrix().normalize_columns();

    auto invMatrix = matrix.GetInverse();

    for (int axis = 0; axis < 3; ++axis)
    {
      auto renderWindow = axis == 0 ? m_RenderWindowPart->GetQmitkRenderWindow("sagittal") :
                          axis == 1 ? m_RenderWindowPart->GetQmitkRenderWindow("coronal") :
                                      m_RenderWindowPart->GetQmitkRenderWindow("axial");

      if (renderWindow != nullptr)
      {
        auto rendererGeometry = renderWindow->GetRenderer()->GetCurrentWorldGeometry();

        /// Because of some problems with the current way of event signalling,
        /// 'Modified' events are sent out from the stepper while the renderer
        /// does not have a geometry yet. Therefore, we do a nullptr check here.
        /// See bug T22122. This check can be resolved after T22122 got fixed.
        if (rendererGeometry != nullptr)
        {
          int dominantAxis =
            itk::Function::Max3(invMatrix[0][axis], invMatrix[1][axis], invMatrix[2][axis]);

          bool referenceGeometryAxisInverted = invMatrix[dominantAxis][axis] < 0;
          bool rendererZAxisInverted = rendererGeometry->GetAxisVector(2)[axis] < 0;

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

          auto sliceNavigationWidget =
            axis == 0 ? m_Ui->sagittalSliceNavWidget :
            axis == 1 ? m_Ui->coronalSliceNavWidget :
                        m_Ui->axialSliceNavWidget;

          sliceNavigationWidget->SetInverseDirection(inverseDirection);

          // This should be a preference (see T22254)
          // bool invertedControls = referenceGeometryAxisInverted != inverseDirection;
          // navigatorWidget->SetInvertedControls(invertedControls);
        }
      }
    }
  }

  this->SetBorderColors();
}

void QmitkImageNavigatorView::SetVisibilityOfTimeSlider(std::size_t timeSteps)
{
  bool isVisible = timeSteps > 1;

  m_Ui->timeSliceNavWidget->setVisible(isVisible);
  m_Ui->timeLabel->setVisible(isVisible);
}
