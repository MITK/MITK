/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include <QmitkDataTreeNodeSelectionProvider.h>
#include <QmitkDnDFrameWidget.h>
#include <QmitkStdMultiWidgetEditor.h>
#include "QmitkStepperAdapter.h"

#include <string>

#include "ui_QmitkImageNavigatorViewControls.h"
#include "berryISizeProvider.h"


/*!
 * \ingroup org_mitk_gui_qt_imagenavigator_internal
 *
 * \brief QmitkImageNavigatorView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkImageNavigatorView : public QObject, public berry::QtViewPart, public berry::ISizeProvider
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkImageNavigatorView();
  virtual ~QmitkImageNavigatorView();

  virtual void CreateQtPartControl(QWidget *parent);

  QmitkStdMultiWidget* GetActiveStdMultiWidget();

  void SetFocus();

  virtual int GetSizeFlags(bool width);
  virtual int ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredResult);

protected slots:

protected:

  Ui::QmitkImageNavigatorViewControls m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
  QmitkStepperAdapter* m_TransversalStepper;
  QmitkStepperAdapter* m_SagittalStepper;
  QmitkStepperAdapter* m_FrontalStepper;
  QmitkStepperAdapter* m_TimeStepper;
};




#endif // _QMITKIMAGENAVIGATORVIEW_H_INCLUDED

