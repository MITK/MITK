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

#include <qobject.h>
#include <qtimer.h>
#include <list>
#include <string>
#include <mitkCommon.h>

class QmitkFctMediator;
class QTimer;

class QMITK_EXPORT QmitkFunctionalityTesting : public QObject
{
  Q_OBJECT
public:
  QmitkFunctionalityTesting( QmitkFctMediator* qfm, QObject * parent = 0, const char * name = 0 );
  ~QmitkFunctionalityTesting();
  QTimer m_CloseMessagesTimer;
  unsigned int m_NumberOfFunctionalitiesFailed;
  std::list<std::string> m_NamesOfFailedFunctionalities;
protected slots:
  virtual void ActivateNextFunctionality();
  virtual void CloseFirstMessageBox();
protected:
  QmitkFctMediator* m_QmitkFctMediator;
  QTimer m_ActivateTimer;
};

int QMITK_EXPORT StartQmitkFunctionalityTesting(QmitkFctMediator* qfm);
