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


#ifndef QMITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197
#define QMITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197

#include "mitkRenderingManager.h"
#include <qobject.h>

class QTimer;

//##Documentation
//## @brief ...
//## @ingroup Renderer
//## 
class QmitkRenderingManager : public QObject, public mitk::RenderingManager
{
public:
  mitkClassMacro( QmitkRenderingManager, mitk::RenderingManager );

  //##Documentation
  //## Constructor
  QmitkRenderingManager();

  //##Documentation
  //## Destructor
  ~QmitkRenderingManager();
  
  Q_OBJECT

protected slots:
  virtual void RestartTimer();

  virtual void StopTimer();

  virtual void QUpdateCallback();


private:
  QTimer *m_Timer;

};


#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
