/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkStandardViews_h_included
#define QmitkStandardViews_h_included

#include "mitkCommon.h"
#include "MitkQtWidgetsExtExports.h"
#include "mitkCameraController.h"

#include <QWidget>

class QClickableLabel;
class vtkRenderWindow;

class MitkQtWidgetsExt_EXPORT QmitkStandardViews : public QWidget
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

