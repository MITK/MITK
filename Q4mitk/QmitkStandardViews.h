/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13900 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkStandardViews_h_included
#define QmitkStandardViews_h_included

#include "mitkCommon.h"
#include "mitkCameraController.h"

#include <QWidget>

class QClickableLabel;
class vtkRenderWindow;

class QMITK_EXPORT QmitkStandardViews : public QWidget 
{

  Q_OBJECT

public:

  QmitkStandardViews( QWidget * parent = 0, Qt::WindowFlags f = 0 );

  virtual ~QmitkStandardViews();

  void SetCameraController( mitk::CameraController* controller );
  void SetCameraControllerFromRenderWindow( vtkRenderWindow* window );
  
signals:
    
  void StandardViewDefined(mitk::CameraController::StandardView view);

protected slots:

  void hotspotClicked(const QString& s);

protected:

  QClickableLabel* m_ClickablePicture;
    
  mitk::CameraController::Pointer m_CameraController;

};

#endif 
