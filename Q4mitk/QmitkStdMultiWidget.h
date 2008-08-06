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

#ifndef QMITKSTDMULTIWIDGET_H_
#define QMITKSTDMULTIWIDGET_H_

#include "ui_QmitkStdMultiWidget.h"

#include "mitkPositionTracker.h"
#include "mitkDisplayVectorInteractor.h"
#include "mitkSlicesRotator.h"
#include "mitkSlicesSwiveller.h"
#include "mitkColoredRectangleRendering.h"
#include "mitkLogoRendering.h"
#include "mitkGradientBackground.h"
#include "mitkCoordinateSupplier.h"

#include <QWidget>

class QHBoxLayout;

class QMITK_EXPORT QmitkStdMultiWidget : public QWidget, public Ui::QmitkStdMultiWidgetUi
{
  Q_OBJECT
  
public:
  
  QmitkStdMultiWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  
  mitk::SliceNavigationController* 
  GetTimeNavigationController();
  
  void RequestUpdate();

  void ForceImmediateUpdate();
  
  mitk::DisplayVectorInteractor* GetMoveAndZoomInteractor();
  
  QmitkRenderWindow* GetRenderWindow1() const;

  QmitkRenderWindow* GetRenderWindow2() const;

  QmitkRenderWindow* GetRenderWindow3() const;

  QmitkRenderWindow* GetRenderWindow4() const;

  const mitk::Point3D &
  GetLastLeftClickPosition() const;

  const mitk::Point3D
  GetCrossPosition() const;

  void EnablePositionTracking();

  void DisablePositionTracking();
  
  int GetLayout() const;
  
  mitk::SlicesRotator * GetSlicesRotator() const;

  mitk::SlicesSwiveller * GetSlicesSwiveller() const;

  
public slots:

  void changeLayoutTo2DImagesUp();
  
  void changeLayoutTo2DImagesLeft();
  
  void changeLayoutToDefault();
  
  void changeLayoutToBig3D();
  
  void changeLayoutToWidget1();
  
  void changeLayoutToWidget2();
  
  void changeLayoutToWidget3();
  
  void changeLayoutToRowWidget3And4();
  
  void changeLayoutToColumnWidget3And4();
  
  void changeLayoutToRowWidgetSmall3andBig4();
  
  void changeLayoutToSmallUpperWidget2Big3and4();
  
  void changeLayoutTo2x2Dand3DWidget();
  
  void changeLayoutToLeft2Dand3DRight2D();
  
  void SetData( mitk::DataTreeIteratorBase* it );
  
  void Fit();
  
  void AddPositionTrackingPointSet(
    mitk::DataTreeIteratorBase* treeIterator);
  
  void AddDisplayPlaneSubTree(mitk::DataTreeIteratorBase* it);
  
  void EnableStandardLevelWindow();
  
  void DisableStandardLevelWindow();
  
  bool InitializeStandardViews(
    mitk::DataTreeIteratorBase * it);
  
  bool InitializeStandardViews( const mitk::Geometry3D * geometry );
  
  void wheelEvent( QWheelEvent * e );
  
  void EnsureDisplayContainsPoint(
    mitk::DisplayGeometry* displayGeometry, const mitk::Point3D& p);
  
  void MoveCrossToPosition(const mitk::Point3D& newPosition);
  
  void EnableNavigationControllerEventListening();
  
  void DisableNavigationControllerEventListening();
  
  void EnableGradientBackground();
  
  void DisableGradientBackground();
  
  void EnableDepartmentLogo();
  
  void DisableDepartmentLogo();
  
  void SetWidgetPlaneVisibility(const char* widgetName, bool visible, mitk::BaseRenderer *renderer=NULL);
   
  void SetWidgetPlanesVisibility(bool visible, mitk::BaseRenderer *renderer=NULL);
  
  void SetWidgetPlanesLocked(bool locked);
  
  void SetWidgetPlanesRotationLocked(bool locked);
  
  void SetWidgetPlanesRotationLinked( bool link );
  
  void SetWidgetPlaneMode( int mode );
  
  void SetGradientBackgroundColors( const mitk::Color & upper, const mitk::Color & lower );
  
  void SetDepartmentLogoPath( const char * path );
  
  void SetWidgetPlaneModeToSlicing( bool activate );
  
  void SetWidgetPlaneModeToRotation( bool activate );
  
  void SetWidgetPlaneModeToSwivel( bool activate );
  
signals:
  
  void WheelMoved( QWheelEvent* );
  void WidgetPlanesVisibilityChanged(bool);
  void WidgetPlanesLockedChanged(bool);
  void WidgetPlanesRotationLockedChanged(bool);
  void WidgetPlanesRotationLinked(bool);
  void WidgetPlanesRotationEnabled(bool);
  void ViewsInitialized();
  void WidgetPlaneModeSlicing(bool);
  void WidgetPlaneModeRotation(bool);
  void WidgetPlaneModeSwivel(bool);
  
public:
  
  enum { PLANE_MODE_SLICING = 0, PLANE_MODE_ROTATION, PLANE_MODE_SWIVEL };
  enum { LAYOUT_DEFAULT = 0, LAYOUT_2D_IMAGES_UP, LAYOUT_2D_IMAGES_LEFT, 
    LAYOUT_BIG_3D, LAYOUT_WIDGET1, LAYOUT_WIDGET2, LAYOUT_WIDGET3, 
    LAYOUT_2X_2D_AND_3D_WIDGET, LAYOUT_ROW_WIDGET_3_AND_4, 
    LAYOUT_COLUMN_WIDGET_3_AND_4, LAYOUT_ROW_WIDGET_SMALL3_AND_BIG4 ,
    LAYOUT_SMALL_UPPER_WIDGET2_BIG3_AND4,LAYOUT_2D_AND_3D_LEFT_2D_RIGHT_WIDGET };
            
  
protected:
  
  QHBoxLayout* QmitkStdMultiWidgetLayout;
  
  mitk::ColoredRectangleRendering::Pointer m_RectangleRendering3;
  int m_PlaneMode;
  mitk::ColoredRectangleRendering::Pointer m_RectangleRendering1;
  mitk::LogoRendering::Pointer m_LogoRendering4;
  mitk::GradientBackground::Pointer m_GradientBackground4;
  mitk::GradientBackground::Pointer m_GradientBackground3;
  int m_Layout;
  mitk::DisplayVectorInteractor::Pointer m_MoveAndZoomInteractor;
  mitk::DataTreeIteratorClone planesIterator;
  mitk::SliceNavigationController::Pointer timeNavigationController;
  mitk::CoordinateSupplier::Pointer m_LastLeftClickPositionSupplier;
  mitk::PositionTracker::Pointer m_PositionTracker;
  mitk::DataTreeNode::Pointer m_PositionTrackerNode;
  mitk::DataTreeBase::Pointer m_Tree;
  mitk::SlicesRotator *m_SlicesRotator;
  mitk::SlicesSwiveller *m_SlicesSwiveller;
  mitk::GradientBackground::Pointer m_GradientBackground1;
  mitk::GradientBackground::Pointer m_GradientBackground2;
  mitk::LogoRendering::Pointer m_LogoRendering1;
  mitk::LogoRendering::Pointer m_LogoRendering2;
  mitk::LogoRendering::Pointer m_LogoRendering3;
  mitk::ColoredRectangleRendering::Pointer m_RectangleRendering2;
  mitk::ColoredRectangleRendering::Pointer m_RectangleRendering4;

};

#endif /*QMITKSTDMULTIWIDGET_H_*/
