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

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkImageNavigatorViewControls.h"



/*!
 * \ingroup org_mitk_gui_qt_imagenavigator_internal
 *
 * \brief QmitkImageNavigatorView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkImageNavigatorView : public QObject, public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkImageNavigatorView();
  virtual ~QmitkImageNavigatorView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

protected slots:

protected:

  Ui::QmitkImageNavigatorViewControls m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
};




#endif // _QMITKIMAGENAVIGATORVIEW_H_INCLUDED

