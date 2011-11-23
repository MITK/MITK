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

#ifndef _QMITKCTKPYTHONSHELL_H
#define _QMITKCTKPYTHONSHELL_H

#include <ctkPythonConsole.h>
#include <ctkAbstractPythonManager.h>
#include "QmitkPythonMediator.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>


/*!
 * \ingroup org_mitk_gui_qt_imagenavigator_internal
 *
 * \brief QmitkPythonVariableStack
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkCTKPythonShell : public ctkPythonConsole, public QmitkPythonPasteClient
{
  Q_OBJECT
public:
  QmitkCTKPythonShell(ctkAbstractPythonManager* pythonManager, QWidget* _parent = 0);
  ~QmitkCTKPythonShell();

  virtual void paste(const QString& command);


protected:
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);
  virtual bool canInsertFromMimeData( const QMimeData *source ) const;
  virtual void executeCommand(const QString& command);

signals:
  void executeCommandSignal(const QString&);
private:
  ctkAbstractPythonManager* m_PythonManager;
};




#endif // _QmitkPythonVariableStack_H_INCLUDED

