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

#ifndef _QMITKSCENESERIALIZATIONVIEW_H_INCLUDED
#define _QMITKSCENESERIALIZATIONVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkSceneSerializationViewControls.h"



/*!
 * \ingroup org_mitk_gui_qt_sceneserialization_internal
 *
 * \brief QmitkSceneSerializationView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkSceneSerializationView : public QObject, public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkSceneSerializationView();
  virtual ~QmitkSceneSerializationView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

protected slots:

  /// \brief Called when the user clicks the GUI button
  void SerializeSelected();

protected:

  Ui::QmitkSceneSerializationViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
};




#endif // _QMITKSCENESERIALIZATIONVIEW_H_INCLUDED

