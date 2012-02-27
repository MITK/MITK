/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKPYTHONVARIABLESTACK_H
#define _QMITKPYTHONVARIABLESTACK_H

#include <berryQtViewPart.h>
#include "berryISizeProvider.h"

#include <QmitkDataNodeSelectionProvider.h>
#include <QmitkDnDFrameWidget.h>
#include <QmitkStdMultiWidgetEditor.h>
#include "QmitkStepperAdapter.h"
#include "ui_QmitkPythonVariableStack.h"
#include "QmitkPythonVariableStackTreeWidget.h"

#include <string>

#include "berryISizeProvider.h"

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

/*!
 * \ingroup org_mitk_gui_qt_imagenavigator_internal
 *
 * \brief QmitkPythonVariableStack
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkPythonVariableStack : public berry::QtViewPart, public berry::ISizeProvider
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkPythonVariableStack();
  QmitkPythonVariableStack(const QmitkPythonVariableStack& other)
  {
     Q_UNUSED(other)
     throw std::runtime_error("Copy constructor not implemented");
  }
  virtual ~QmitkPythonVariableStack();

  virtual void CreateQtPartControl(QWidget *parent);

  QmitkStdMultiWidget* GetActiveStdMultiWidget();

  void SetFocus();

  virtual int GetSizeFlags(bool width);
  virtual int ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult);
  QmitkPythonVariableStackTreeWidget* getModel();


public slots:

protected slots:

protected:
  QmitkStdMultiWidget* m_MultiWidget;
  QTableWidget *m_tableWidget;
  Ui::QmitkPythonConsoleViewControls* m_Controls;
  QmitkPythonVariableStackTreeWidget* m_treeModel;

private:

};




#endif // _QmitkPythonVariableStack_H_INCLUDED

