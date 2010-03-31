/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKVIEWINITIALIZATIONVIEW_H_INCLUDED
#define _QMITKVIEWINITIALIZATIONVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

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
class QmitkViewInitializationView : public QObject, public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkViewInitializationView();
  virtual ~QmitkViewInitializationView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

protected:

  vtkRenderWindow* GetSelectedRenderWindow();

  void InitRenderWindowSelector();

  void UpdateRendererList();

protected slots:

  virtual void OnApply();

  virtual void OnResetAll();
  

protected:

  Ui::QmitkViewInitializationViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
};




#endif // _QMITKVIEWINITIALIZATIONVIEW_H_INCLUDED

