/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKIMAGENAVIGATORVIEW_H_INCLUDED
#define _QMITKIMAGENAVIGATORVIEW_H_INCLUDED

#include <berryQtViewPart.h>
#include "berryISizeProvider.h"

#include <QmitkDataNodeSelectionProvider.h>
#include <QmitkDnDFrameWidget.h>
#include <QmitkStdMultiWidgetEditor.h>
#include "QmitkStepperAdapter.h"

#include <string>

#include "ui_QmitkImageNavigatorViewControls.h"
#include "berryISizeProvider.h"


/*!
 * \ingroup org_mitk_gui_qt_imagenavigator_internal
 *
 * \class QmitkImageNavigatorView
 *
 * \brief Provides a means to scan quickly through a dataset via Transversal,
 * Coronal and Sagittal sliders, displaying millimetre location and stepper position.
 *
 * For images, the stepper position corresponds to a voxel index. For other datasets
 * such as a surface, it corresponds to a sub-division of the bounding box.
 *
 * \sa QmitkFunctionality
 */
class QmitkImageNavigatorView : public berry::QtViewPart, public berry::ISizeProvider
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkImageNavigatorView();
  QmitkImageNavigatorView(const QmitkImageNavigatorView& other)
  {
     Q_UNUSED(other)
     throw std::runtime_error("Copy constructor not implemented");
  }

  virtual ~QmitkImageNavigatorView();

  virtual void CreateQtPartControl(QWidget *parent);

  QmitkStdMultiWidget* GetActiveStdMultiWidget();

  void SetFocus();

  virtual int GetSizeFlags(bool width);
  virtual int ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult);

protected slots:

  void OnMillimetreCoordinateValueChanged();
  void OnRefetch();

protected:

  friend class ImageNavigatorPartListener;

  void SetMultiWidget(QmitkStdMultiWidget* multiWidget);
  void SetBorderColors();
  void SetBorderColor(QDoubleSpinBox *spinBox, QString colorAsStyleSheetString);
  void SetBorderColor(int axis, QString colorAsStyleSheetString);
  void SetStepSizes();
  void SetStepSize(int axis);
  void SetStepSize(int axis, double stepSize);
  int  GetClosestAxisIndex(mitk::Vector3D normal);

  Ui::QmitkImageNavigatorViewControls m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
  QmitkStepperAdapter* m_TransversalStepper;
  QmitkStepperAdapter* m_SagittalStepper;
  QmitkStepperAdapter* m_FrontalStepper;
  QmitkStepperAdapter* m_TimeStepper;

  berry::IPartListener::Pointer multiWidgetListener;
};




#endif // _QMITKIMAGENAVIGATORVIEW_H_INCLUDED

