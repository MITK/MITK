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

#ifndef _QmitkPythonSnippets_H
#define _QmitkPythonSnippets_H

#include <berryQtViewPart.h>
#include "berryISizeProvider.h"

#include <QmitkDataNodeSelectionProvider.h>
#include <QmitkDnDFrameWidget.h>
#include <QmitkStdMultiWidgetEditor.h>
#include "QmitkStepperAdapter.h"
#include "ui_QmitkPythonSnippets.h"

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
 * \brief QmitkPythonSnippets
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkPythonSnippets : public QObject, public berry::QtViewPart, public berry::ISizeProvider
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;
  static std::map<std::string, std::string> CreateDefaultSnippets();
  static const std::map<std::string, std::string> DEFAULT_SNIPPETS;

  QmitkPythonSnippets();
  virtual ~QmitkPythonSnippets();

  virtual void CreateQtPartControl(QWidget *parent);

  QmitkStdMultiWidget* GetActiveStdMultiWidget();

  void SetFocus();

  virtual int GetSizeFlags(bool width);
  virtual int ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult);

protected slots:
  void on_Name_currentIndexChanged(int i);
  void on_RenameSnippet_clicked();
  void on_AddNewSnippet_clicked();
  void on_RemoveSnippet_clicked();
  void on_PasteNow_clicked();
  void on_RestoreDefaultSnippets_clicked();
  void on_Content_textChanged();

protected:
  void Update();
  void CreateUniqueName( QString& name );
  QmitkStdMultiWidget* m_MultiWidget;
  QTableWidget *m_tableWidget;
  Ui::QmitkPythonSnippets* m_Controls;
  std::map<std::string, std::string> m_Snippets;
  QString m_NameToSelect;
};




#endif // _QmitkPythonSnippets_H_INCLUDED

