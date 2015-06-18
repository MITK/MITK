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

#ifndef _QMITKVIEWINITIALIZATIONVIEW_H_INCLUDED
#define _QMITKVIEWINITIALIZATIONVIEW_H_INCLUDED

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>

#include <string>
#include "vtkRenderWindow.h"

#include "ui_QmitkViewInitializationViewControls.h"



/*!
 * \ingroup org_mitk_gui_qt_viewinitialization_internal
 *
 * \brief QmitkViewInitializationView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkViewInitializationView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkViewInitializationView();
  virtual ~QmitkViewInitializationView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

private:

  vtkRenderWindow* GetSelectedRenderWindow();

  void InitRenderWindowSelector();

  void UpdateRendererList();

  virtual void Activated() override;
  virtual void Deactivated() override;
  virtual void Visible() override;
  virtual void Hidden() override;

private slots:

  virtual void OnApply();

  virtual void OnResetAll();


private:

  Ui::QmitkViewInitializationViewControls* m_Controls;

  unsigned long m_CommandTag;
};




#endif // _QMITKVIEWINITIALIZATIONVIEW_H_INCLUDED

