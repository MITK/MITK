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

#ifndef _QMITKIMAGENAVIGATORVIEW_H_INCLUDED
#define _QMITKIMAGENAVIGATORVIEW_H_INCLUDED

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

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
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
  void UpdateStatusBar();

protected:

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart) override;

  void SetBorderColors();
  void SetBorderColor(QDoubleSpinBox *spinBox, QString colorAsStyleSheetString);
  void SetBorderColor(int axis, QString colorAsStyleSheetString);
  void SetStepSizes();
  void SetStepSize(int axis);
  void SetStepSize(int axis, double stepSize);
  int  GetClosestAxisIndex(mitk::Vector3D normal);
  void SetVisibilityOfTimeSlider(std::size_t timeSteps);

  Ui::QmitkImageNavigatorViewControls m_Controls;

  QmitkStepperAdapter* m_AxialStepper;
  QmitkStepperAdapter* m_SagittalStepper;
  QmitkStepperAdapter* m_FrontalStepper;
  QmitkStepperAdapter* m_TimeStepper;

  QWidget* m_Parent;

  mitk::IRenderWindowPart* m_IRenderWindowPart;
  /**
   * @brief GetDecorationColorOfGeometry helper method to get the color of a helper geometry node.
   * @param renderWindow The renderwindow of the geometry
   * @return the color for decoration in QString format (#RRGGBB).
   */
  QString GetDecorationColorOfGeometry(QmitkRenderWindow *renderWindow);
};

#endif // _QMITKIMAGENAVIGATORVIEW_H_INCLUDED
