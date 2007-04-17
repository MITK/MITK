/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include <qobject.h>
#include <qtimer.h>

class QmitkFctMediator;
class QTimer;

class QmitkFunctionalityTesting : public QObject
{
  Q_OBJECT
public:
  QmitkFunctionalityTesting( QmitkFctMediator* qfm, QObject * parent = 0, const char * name = 0 );
  ~QmitkFunctionalityTesting();
  QTimer m_CloseMessagesTimer;
protected slots:
  virtual void ActivateNextFunctionality();
  virtual void CloseFirstMessageBox();
protected:
  QmitkFctMediator* m_QmitkFctMediator;
  QTimer m_ActivateTimer;
};

int StartQmitkFunctionalityTesting(QmitkFctMediator* qfm);
