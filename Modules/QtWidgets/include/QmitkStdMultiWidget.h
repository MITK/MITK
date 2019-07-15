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

#ifndef QmitkStdMultiWidget_h
#define QmitkStdMultiWidget_h

#include "MitkQtWidgetsExports.h"

#include <mitkLogoOverlay.h>
#include <mitkDataStorage.h>

#include <mitkMouseModeSwitcher.h>

#include <qwidget.h>
#include <qsplitter.h>
#include <QFrame>

#include <mitkCrosshairManager.h>
#include <QmitkRenderWindow.h>
#include <QmitkLevelWindowWidget.h>

#include <mitkBaseRenderer.h>

class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;
class QmitkLevelWindowWidget;
class QmitkRenderWindow;
class vtkCornerAnnotation;
class vtkMitkRectangleProp;

namespace mitk {
  class RenderingManager;
}

/// \ingroup QmitkModule
class MITKQTWIDGETS_EXPORT QmitkStdMultiWidget : public QWidget
{
  Q_OBJECT

private:

  bool m_drawTextInStatusBar;
  QTimer m_ResizeTimer;

  void UpdateAnnotationFonts();

public:
  QmitkStdMultiWidget(QWidget* parent = 0, Qt::WindowFlags f = 0, mitk::RenderingManager* renderingManager = 0, mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard, bool useFXAA = true, const QString& name = "stdmulti", bool crosshairVisibility3D = true);
  virtual ~QmitkStdMultiWidget();

  mitk::SliceNavigationController*
  GetTimeNavigationController();

  void RequestUpdate();

  void ForceImmediateUpdate();

  mitk::MouseModeSwitcher* GetMouseModeSwitcher();
  void setMouseMode(mitk::MouseModeSwitcher::MouseMode mode, const Qt::MouseButton& button);

  QmitkRenderWindow* GetRenderWindow1() const;

  QmitkRenderWindow* GetRenderWindow2() const;

  QmitkRenderWindow* GetRenderWindow3() const;

  QmitkRenderWindow* GetRenderWindow4() const;

  void setShadowWidget1Visible(bool visible) const;
  void setShadowWidget2Visible(bool visible) const;
  void setShadowWidget3Visible(bool visible) const;
  void setShadowWidget4Visible(bool visible) const;

  QWidget* getShadowWidget1() const;
  QWidget* getShadowWidget2() const;
  QWidget* getShadowWidget3() const;
  QWidget* getShadowWidget4() const;

  void setDisplayPositionText(bool draw = true);

  /**
  * \deprecatedSince{2015_05} GetLastLeftClickPosition is deprecated. Function will be removed and funtionality will move into mitk::DisplayInteractor.
  */

  const mitk::Point3D &
  GetLastLeftClickPosition() const;

  const mitk::Point3D
  GetCrossPosition() const;

  void EnablePositionTracking();

  void DisablePositionTracking();

  int GetLayout() const;

  bool GetGradientBackgroundFlag() const;

  bool IsColoredRectanglesEnabled() const;

  bool IsDepartmentLogoEnabled() const;

  void InitializeWidget(bool showPlanesIn3d);

  /// called when the StdMultiWidget is closed to remove the 3 widget planes and the helper node from the DataStorage
  void RemovePlanesFromDataStorage();

  void AddPlanesToDataStorage();

  void SetDataStorage( mitk::DataStorage* ds );

  /** \brief Listener to the CrosshairPositionEvent

    Ensures the CrosshairPositionEvent is handled only once and at the end of the Qt-Event loop
  */
  void HandleCrosshairPositionEvent();

  /// activate Menu Widget. true: activated, false: deactivated
  void ActivateMenuWidget( bool state );

  bool IsMenuWidgetEnabled() const;

  void SetCornerAnnotationVisibility(bool visibility);

  bool IsCornerAnnotationVisible(void) const;

  void setDisplayMetaInfo(bool metainfo);

  void setDisplayMetaInfoEx(bool metainfo);

  void setDisplayPatientInfo(bool patientinfo);

  void setDisplayPatientInfoEx(bool patientinfo);

  void setDisplayPositionInfo(bool positioninfo);

  void setSelectionMode(bool selection);

  void setAnnotationVisibility(std::vector<bool>& visibility);

  typedef void (QmitkStdMultiWidget::*FunctionSet)(bool);
  std::vector<FunctionSet> getFuncSetDisplayAnnotation();

  void UpdateFullSreenMode();

protected:

  bool m_displayMetaInfo;
  bool m_displayMetaInfoEx;
  bool m_displayPatientInfo;
  bool m_displayPatientInfoEx;
  bool m_displayPositionInfo;

  void UpdateAllWidgets();
  void HideAllWidgetToolbars();

  mitk::PropertyList::Pointer imageProperties;
  unsigned long m_ImageMTime;
  std::string m_ImageName;

  vtkCornerAnnotation* cornerText[4];
  vtkTextProperty* textProp[4];
  vtkRenderer* ren[4];

  void setCornerAnnotation(int corner, int i, const char* text);
  void setCornerAnnotationMaxText(int corner, int i, const char* text);

  mitk::DataNode::Pointer GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes);

public slots:

  /// Receives the signal from HandleCrosshairPositionEvent, executes the StatusBar update
  void HandleCrosshairPositionEventDelayed();

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

  void changeLayoutTo2DUpAnd3DDown();

  void changeLayoutToColumnWidget1And2();

  void changeLayoutToAxialLeft2DRight(int state = 0);

  void Fit();

  void InitPositionTracking();

  void AddDisplayPlaneSubTree(const std::string& parentWidget);

  void EnableStandardLevelWindow();

  void DisableStandardLevelWindow();

  bool InitializeStandardViews( const mitk::Geometry3D * geometry );

  void wheelEvent( QWheelEvent * e ) override;

  void mousePressEvent(QMouseEvent * e) override;

  void moveEvent( QMoveEvent* e ) override;

  void OnWindowResized();

  void OnResizeStoped();

  void EnsureDisplayContainsPoint(mitk::BaseRenderer *renderer, const mitk::Point3D&p);

  void MoveCrossToPosition(const mitk::Point3D& newPosition);

  //void EnableNavigationControllerEventListening();

  //void DisableNavigationControllerEventListening();

  void EnableGradientBackground();

  void DisableGradientBackground();

  void EnableDepartmentLogo();

  void DisableDepartmentLogo();

  void EnableColoredRectangles();

  void DisableColoredRectangles();

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

  void OnLayoutDesignChanged( int layoutDesignIndex, int activeProjection = 0 );

  void ResetCrosshair();

  void resetThickSlice();

signals:

  void LeftMouseClicked(mitk::Point3D pointValue);
  void WheelMoved(QWheelEvent*);
  void WidgetPlanesRotationLinked(bool);
  void WidgetPlanesRotationEnabled(bool);
  void ViewsInitialized();
  void WidgetPlaneModeSlicing(bool);
  void WidgetPlaneModeRotation(bool);
  void WidgetPlaneModeSwivel(bool);
  void WidgetPlaneModeChange(int);
  void WidgetNotifyNewCrossHairMode(int);
  void Moved();

  void savePlaneVisibility3D(bool visibility);

public:

  /** Define RenderWindow (public)*/
  QmitkRenderWindow* mitkWidget1;
  QmitkRenderWindow* mitkWidget2;
  QmitkRenderWindow* mitkWidget3;
  QmitkRenderWindow* mitkWidget4;
  QmitkLevelWindowWidget* levelWindowWidget;
  /********************************/

  enum { PLANE_MODE_SLICING = 0, PLANE_MODE_ROTATION, PLANE_MODE_SWIVEL };
  enum { LAYOUT_DEFAULT = 0, LAYOUT_2D_IMAGES_UP, LAYOUT_2D_IMAGES_LEFT,
         LAYOUT_BIG_3D, LAYOUT_WIDGET1, LAYOUT_WIDGET2, LAYOUT_WIDGET3,
         LAYOUT_2X_2D_AND_3D_WIDGET, LAYOUT_ROW_WIDGET_3_AND_4,
         LAYOUT_COLUMN_WIDGET_3_AND_4, LAYOUT_ROW_WIDGET_SMALL3_AND_BIG4 ,
         LAYOUT_SMALL_UPPER_WIDGET2_BIG3_AND4,LAYOUT_2D_AND_3D_LEFT_2D_RIGHT_WIDGET,
         LAYOUT_2D_UP_AND_3D_DOWN, LAYOUT_COLUMN_WIDGET_1_AND_2,
         LAYOUT_AXIAL_LEFT_2D_RIGHT};

  enum {
    AXIAL,
    SAGITTAL,
    CORONAL,
    THREE_D
  };

  /**
   * @brief SetCornerAnnotation Create a corner annotation for a widget.
   * @param text The text of the annotation.
   * @param color The color.
   * @param widgetNumber The widget (0-3).
   */
  void SetDecorationProperties(std::string text, mitk::Color color, int widgetNumber);
  /**
   * @brief GetRenderWindow convinience method to get a widget.
   * @param number of the widget (0-3)
   * @return The renderwindow widget.
   */
  QmitkRenderWindow *GetRenderWindow(unsigned int number);

  /**
   * @brief SetGradientBackgroundColorForRenderWindow background for a widget.
   *
   * If two different input colors are, a gradient background is generated.
   *
   * @param upper Upper color of the gradient background.
   * @param lower Lower color of the gradient background.
   * @param widgetNumber The widget (0-3).
   */
  void SetGradientBackgroundColorForRenderWindow(const mitk::Color &upper, const mitk::Color &lower, unsigned int widgetNumber);

  /**
   * @brief GetDecorationColorForWidget Get the color for annotation, crosshair and rectangle.
   * @param widgetNumber Number of the renderwindow (0-3).
   * @return Color in mitk format.
   */
  mitk::Color GetDecorationColor(unsigned int widgetNumber);

  /**
   * @brief SetDecorationColor Set the color of the decoration of the 4 widgets.
   *
   * This is used to color the frame of the renderwindow and the corner annatation.
   * For the first 3 widgets, this color is a property of the helper object nodes
   * which contain the respective plane geometry. For widget 4, this is a member,
   * since there is no data node for this widget.
   */
  void SetDecorationColor(unsigned int widgetNumber, mitk::Color color);

  /**
   * @brief GetCornerAnnotationText Getter for corner annotation text.
   * @param widgetNumber the widget number (0-3).
   * @return The text in the corner annotation.
   */
  std::string GetCornerAnnotationText(unsigned int widgetNumber);

  // get main split
  QSplitter* GetMainSplit();

  /**
   * @brief GetGradientColors Getter for gradientbackground colors.
   * @param widgetNumber the widget number (0-3).
   * @return A pair of colors. First: upper, second: lower.
   */
  std::pair<mitk::Color, mitk::Color> GetGradientColors(unsigned int widgetNumber);
  mitkCrosshairManager* crosshairManager;

protected:
  void setViewDirectionAnnontation(mitk::Image* image, int slice, int index);

  QHBoxLayout* QmitkStdMultiWidgetLayout;

  int m_Layout;
  int m_PlaneMode;

  mitk::RenderingManager* m_RenderingManager;

  mitk::LogoOverlay::Pointer m_LogoRendering;

  bool m_GradientBackgroundFlag;

  mitk::MouseModeSwitcher::Pointer m_MouseModeSwitcher;
  mitk::SliceNavigationController* m_TimeNavigationController;

  mitk::DataStorage::Pointer m_DataStorage;

  /**
   * @brief m_DecorationColorWidget4 color for annotation and rectangle of widget 4.
   *
   * For other widgets1-3, the color is a property of the respective data node.
   * There is no node for widget 4, hence, we need an extra member.
   */
  mitk::Color m_DecorationColorWidget4;

  /**
   * @brief m_GradientBackgroundColors Contains the colors of the gradient background.
   *
   */
  std::pair<mitk::Color, mitk::Color> m_GradientBackgroundColors[4];

  QSplitter *m_MainSplit;
  QSplitter *m_LayoutSplit;
  QSplitter *m_SubSplit1;
  QSplitter *m_SubSplit2;

  QWidget *mitkWidget1Container;
  QWidget *mitkWidget2Container;
  QWidget *mitkWidget3Container;
  QWidget *mitkWidget4Container;

  QWidget *m_ShadowWidgets[4];
  mutable bool m_ShadowWidgetVisible[4];

  vtkSmartPointer<vtkCornerAnnotation> m_CornerAnnotations[4];
  vtkSmartPointer<vtkMitkRectangleProp> m_RectangleProps[4];

  bool m_PendingCrosshairPositionEvent;
  bool m_CrosshairNavigationEnabled;

  QString m_Name;

  /**
   * @brief CreateCornerAnnotation helper method to create a corner annotation.
   * @param text of the annotation.
   * @param color of the annotation.
   * @return the complete CornerAnnotation.
   */
  vtkSmartPointer<vtkCornerAnnotation> CreateCornerAnnotation(std::string text, mitk::Color color);

  /**
   * @brief FillGradientBackgroundWithBlack Internal helper method to initialize the
   * gradient background colors with black.
   */
  void FillGradientBackgroundWithBlack();

  void ResetTransformation(mitk::VtkPropRenderer* renderer);

  QWidget* createShadowWidget(QWidget* parent);
};
#endif /*QmitkStdMultiWidget_h*/
