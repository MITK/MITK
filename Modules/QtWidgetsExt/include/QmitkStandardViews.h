/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStandardViews_h
#define QmitkStandardViews_h

#include "MitkQtWidgetsExtExports.h"
#include "mitkCameraController.h"
#include "mitkCommon.h"

#include <QWidget>

class QClickableLabel;
class vtkRenderWindow;

class MITKQTWIDGETSEXT_EXPORT QmitkStandardViews : public QWidget
{
  Q_OBJECT

public:
  QmitkStandardViews(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);

  ~QmitkStandardViews() override;

  void SetCameraController(mitk::CameraController *controller);
  void SetCameraControllerFromRenderWindow(vtkRenderWindow *window);

signals:

  void StandardViewDefined(mitk::CameraController::StandardView view);

protected slots:

  void hotspotClicked(const QString &s);

protected:
  QClickableLabel *m_ClickablePicture;

  mitk::CameraController::Pointer m_CameraController;
};

#endif
