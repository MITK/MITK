/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageNavigatorView_h
#define QmitkImageNavigatorView_h

#include <berryISizeProvider.h>

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include "ui_QmitkImageNavigatorViewControls.h"

class QmitkStepperAdapter;

/*!
 * \ingroup org_mitk_gui_qt_imagenavigator_internal
 *
 * \class QmitkImageNavigatorView
 *
 * \brief Provides a means to scan quickly through a dataset via Axial,
 * Coronal and Sagittal sliders, displaying millimetre location and stepper position.
 *
 * For images, the stepper position corresponds to a voxel index. For other datasets
 * such as a surface, it corresponds to a sub-division of the bounding box.
 *
 * \sa QmitkAbstractView
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
  int ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult) override;

protected slots:

  void OnMillimetreCoordinateValueChanged();
  void OnRefetch();

protected:

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart) override;

  void SetBorderColors();
  void SetBorderColor(QDoubleSpinBox *spinBox, QString colorAsStyleSheetString);
  void SetBorderColor(int axis, QString colorAsStyleSheetString);
  int  GetClosestAxisIndex(mitk::Vector3D normal);
  void SetVisibilityOfTimeSlider(std::size_t timeSteps);

  Ui::QmitkImageNavigatorViewControls m_Controls;

  QmitkStepperAdapter* m_AxialStepperAdapter;
  QmitkStepperAdapter* m_SagittalStepperAdapter;
  QmitkStepperAdapter* m_CoronalStepperAdapter;
  QmitkStepperAdapter* m_TimeStepperAdapter;

  QWidget* m_Parent;

  mitk::IRenderWindowPart* m_IRenderWindowPart;
  /**
   * @brief GetDecorationColorOfGeometry helper method to get the color of a helper geometry node.
   * @param renderWindow The renderwindow of the geometry
   * @return the color for decoration in QString format (#RRGGBB).
   */
  QString GetDecorationColorOfGeometry(QmitkRenderWindow *renderWindow);
};

#endif
