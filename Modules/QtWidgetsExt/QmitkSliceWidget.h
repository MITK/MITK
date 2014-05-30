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

#ifndef QMITKSLICEWIDGET_H_
#define QMITKSLICEWIDGET_H_

#include "ui_QmitkSliceWidget.h"
#include "MitkQtWidgetsExtExports.h"

#include "QmitkRenderWindow.h"
#include "mitkSliceNavigationController.h"
#include "mitkDataStorage.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkSlicedGeometry3D.h"
#include <QWidget>

class MitkQtWidgetsExt_EXPORT QmitkSliceWidget : public QWidget, public Ui::QmitkSliceWidgetUi
{
  Q_OBJECT

public:

  QmitkSliceWidget(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags f = 0);


  mitk::VtkPropRenderer* GetRenderer();

  QFrame* GetSelectionFrame();

  void UpdateGL();

  void mousePressEvent( QMouseEvent * e );

  void setPopUpEnabled( bool b );

  void SetDataStorage( mitk::StandaloneDataStorage::Pointer storage );

  mitk::StandaloneDataStorage* GetDataStorage();

  QmitkSliderNavigatorWidget* GetNavigatorWidget();

  bool IsLevelWindowEnabled();

  QmitkRenderWindow* GetRenderWindow();

  mitk::SliceNavigationController* GetSliceNavigationController() const;

  mitk::CameraRotationController* GetCameraRotationController() const;

  mitk::BaseController* GetController() const;


public slots:

 void SetData(mitk::DataStorage::SetOfObjects::ConstIterator it);

 void SetData(mitk::DataStorage::SetOfObjects::ConstIterator it, mitk::SliceNavigationController::ViewDirection view);

 void SetData( mitk::DataNode::Pointer node  );

  void SetData( mitk::DataNode::Pointer node, mitk::SliceNavigationController::ViewDirection view );

  void InitWidget( mitk::SliceNavigationController::ViewDirection viewDirection );

  void wheelEvent( QWheelEvent * e );

  void ChangeView(QAction* val);

  void SetLevelWindowEnabled( bool enable );


protected:

  QmitkRenderWindow* m_RenderWindow;
  mitk::SliceNavigationController::ViewDirection m_View;

private:

  bool popUpEnabled;
  mitk::VtkPropRenderer::Pointer m_Renderer;
  mitk::SlicedGeometry3D::Pointer m_SlicedGeometry;
  mitk::StandaloneDataStorage::Pointer m_DataStorage;

  QMenu* popUp;

};

#endif

