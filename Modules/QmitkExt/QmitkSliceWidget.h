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

#include "QmitkRenderWindow.h"
#include "mitkSliceNavigationController.h"

#include <QWidget>

class QMITKEXT_EXPORT QmitkSliceWidget : public QWidget, public Ui::QmitkSliceWidgetUi
{
  Q_OBJECT
  
public:
  
  QmitkSliceWidget(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags f = 0);


  mitk::VtkPropRenderer* GetRenderer();

  QFrame* GetSelectionFrame();
  
  void UpdateGL();

  void mousePressEvent( QMouseEvent * e );

  void setPopUpEnabled( bool b );

  mitk::DataTreeIteratorBase* GetDataTreeIterator();

  mitk::DataTree::Pointer GetDataTree();

  QmitkSliderNavigatorWidget* GetNavigatorWidget();

  bool IsLevelWindowEnabled();

  QmitkRenderWindow* GetRenderWindow();

  mitk::SliceNavigationController* GetSliceNavigationController() const;

  mitk::CameraRotationController* GetCameraRotationController() const;

  mitk::BaseController* GetController() const;

  
public slots:

  void SetData( mitk::DataTreeIteratorBase * it );

  void SetData( mitk::DataTreeNode::Pointer node  );

  void AddData( mitk::DataTreeNode::Pointer node);

  void SetData( mitk::DataTreeIteratorBase* it, mitk::SliceNavigationController::ViewDirection view );

  void InitWidget( mitk::SliceNavigationController::ViewDirection viewDirection );

  void wheelEvent( QWheelEvent * e );

  void ChangeView(QAction* val);
  
  void SetLevelWindowEnabled( bool enable );
  
  
protected:
  
  QmitkRenderWindow* m_RenderWindow;
  mitk::SliceNavigationController::ViewDirection m_View;
  //int newVariable;
  
private:
  
  mitk::DataTree::Pointer m_DataTree;
  bool popUpEnabled;
  mitk::DataTreeIteratorClone m_DataTreeIterator;
  mitk::VtkPropRenderer::Pointer m_Renderer;
  mitk::SlicedGeometry3D::Pointer m_SlicedGeometry;
  QMenu* popUp;
  
};

#endif /*QMITKSLICEWIDGET_H_*/
