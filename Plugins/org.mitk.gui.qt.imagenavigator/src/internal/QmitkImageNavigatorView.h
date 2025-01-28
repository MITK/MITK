/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageNavigatorView_h
#define QmitkImageNavigatorView_h

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include <berryISizeProvider.h>

class QmitkSliceNavigationWidget;
class QmitkStepperAdapter;

class QDoubleSpinBox;
class QLabel;

namespace Ui
{
  class QmitkImageNavigatorView;
}

/**
 * \brief Allows to scan quickly through a dataset via axial, sagittal and coronal sliders,
 *        displaying millimeter location and stepper position.
 *
 * For images, the stepper position corresponds to a voxel index. For other datasets
 * such as surfaces, it corresponds to a sub-division of the bounding box.
 *
 * \ingroup org_mitk_gui_qt_imagenavigator_internal
 */
class QmitkImageNavigatorView :
    public QmitkAbstractView, public mitk::IRenderWindowPartListener,
    public berry::ISizeProvider
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkImageNavigatorView();
  ~QmitkImageNavigatorView() override;

  void CreateQtPartControl(QWidget *parent) override;

  int GetSizeFlags(bool width) override;
  int ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredResult) override;

private:
  struct AxisUi
  {
    QString RenderWindowId;
    QDoubleSpinBox* SpinBox;
    QLabel* Label;
    QmitkSliceNavigationWidget* SliceNavWidget;
    QmitkStepperAdapter* StepperAdapter;
  };

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  void OnCoordValueChanged();
  void OnRefetch();

  void SetBorderColors();
  void SetBorderColor(QDoubleSpinBox* spinBox, const QString& hexColor);
  void SetBorderColor(int axis, const QString& hexColor);
  void SetBorderColor(QmitkRenderWindow* renderWindow);

  int GetClosestAxisIndex(const mitk::Vector3D& normal);
  void SetVisibilityOfTimeSlider(size_t timeSteps);

  Ui::QmitkImageNavigatorView* m_Ui;
  std::array<AxisUi, 3> m_AxisUi;

  QmitkStepperAdapter* m_TimeStepperAdapter;
  mitk::IRenderWindowPart* m_RenderWindowPart;

  QWidget* m_Parent;

  /**
   * \brief Helper method to get the color of a helper geometry node.
   *
   * \param renderWindow The render window of the geometry.
   * \return The color for decoration in QString format (#RRGGBB).
   */
  QString GetDecorationColorOfGeometry(QmitkRenderWindow *renderWindow);
};

#endif
