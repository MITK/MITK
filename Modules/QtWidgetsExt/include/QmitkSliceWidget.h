/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSLICEWIDGET_H_
#define QMITKSLICEWIDGET_H_

#include "MitkQtWidgetsExtExports.h"
#include "ui_QmitkSliceWidget.h"

#include "QmitkRenderWindow.h"
#include "mitkDataStorage.h"
#include "mitkSliceNavigationController.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkStandaloneDataStorage.h"
#include <QWidget>

class MITKQTWIDGETSEXT_EXPORT QmitkSliceWidget : public QWidget, public Ui::QmitkSliceWidgetUi
{
  Q_OBJECT

public:
  QmitkSliceWidget(QWidget *parent = nullptr, const char *name = nullptr, Qt::WindowFlags f = nullptr);

  mitk::VtkPropRenderer *GetRenderer();

  QFrame *GetSelectionFrame();

  void UpdateGL();

  void mousePressEvent(QMouseEvent *e) override;

  void setPopUpEnabled(bool b);

  void SetDataStorage(mitk::StandaloneDataStorage::Pointer storage);

  mitk::StandaloneDataStorage *GetDataStorage();

  QmitkSliderNavigatorWidget *GetNavigatorWidget();

  bool IsLevelWindowEnabled();

  QmitkRenderWindow *GetRenderWindow();

  mitk::SliceNavigationController *GetSliceNavigationController() const;

  mitk::CameraRotationController *GetCameraRotationController() const;

  mitk::BaseController *GetController() const;

public slots:

  void SetData(mitk::DataStorage::SetOfObjects::ConstIterator it);

  void SetData(mitk::DataStorage::SetOfObjects::ConstIterator it, mitk::SliceNavigationController::ViewDirection view);

  void SetData(mitk::DataNode::Pointer node);

  void SetData(mitk::DataNode::Pointer node, mitk::SliceNavigationController::ViewDirection view);

  void InitWidget(mitk::SliceNavigationController::ViewDirection viewDirection);

  void wheelEvent(QWheelEvent *e) override;

  void ChangeView(QAction *val);

  void SetLevelWindowEnabled(bool enable);

protected:
  QmitkRenderWindow *m_RenderWindow;
  mitk::SliceNavigationController::ViewDirection m_View;

private:
  bool popUpEnabled;
  mitk::VtkPropRenderer::Pointer m_Renderer;
  mitk::SlicedGeometry3D::Pointer m_SlicedGeometry;
  mitk::StandaloneDataStorage::Pointer m_DataStorage;

  QMenu *popUp;
};

#endif
