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

#ifndef QMITKSLICEWIDGET_H_
#define QMITKSLICEWIDGET_H_

#include "ui_QmitkSliceWidget.h"
#include "QmitkExtExports.h"

#include "QmitkRenderWindow.h"
#include "mitkSliceNavigationController.h"
#include "mitkDataStorage.h"
#include "mitkStandaloneDataStorage.h"

#include <QWidget>

class QmitkExt_EXPORT QmitkSliceWidget : public QWidget, public Ui::QmitkSliceWidgetUi
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

 void SetData( mitk::DataTreeNode::Pointer node  );

 // void AddData( mitk::DataTreeNode::Pointer node);

  void SetData( mitk::DataTreeNode::Pointer node, mitk::SliceNavigationController::ViewDirection view );

  void InitWidget( mitk::SliceNavigationController::ViewDirection viewDirection );

  void wheelEvent( QWheelEvent * e );

  void ChangeView(QAction* val);

  void SetLevelWindowEnabled( bool enable );


protected:

  QmitkRenderWindow* m_RenderWindow;
  mitk::SliceNavigationController::ViewDirection m_View;
  //int newVariable;

private:

  bool popUpEnabled;
  mitk::VtkPropRenderer::Pointer m_Renderer;
  mitk::SlicedGeometry3D::Pointer m_SlicedGeometry;
  mitk::StandaloneDataStorage::Pointer m_DataStorage;

  QMenu* popUp;

};

#endif /*QMITKSLICEWIDGET_H_*/
