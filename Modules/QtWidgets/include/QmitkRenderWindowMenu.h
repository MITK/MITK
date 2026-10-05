/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowMenu_h
#define QmitkRenderWindowMenu_h

// mitk qtwidgets module
#include <MitkQtWidgetsExports.h>
#include <QmitkCrosshairRotationMode.h>
#include <QmitkMultiWidgetLayoutManager.h>

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkVtkPropRenderer.h>

// qt
#include <QAction>
#include <QEvent>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QToolButton>
#include <QWidget>

class QMenu;
class QmitkRenderWindowMenuBar;

namespace mitk
{
  class IPreferences;
}

/**
 * \ingroup QmitkModule
 * \brief The menu of a QmitkRenderWindow, shown while the mouse cursor is
 * over the window.
 *
 * Its buttons are grouped into bars docked to the top corners of the
 * window: the lighting button of 3D windows to the upper left, the
 * crosshair, full-screen and layout buttons to the upper right. Each bar
 * rests smaller and translucent until the mouse cursor gets close to it.
 * The menu can be deactivated with ActivateMenuWidget(false) in
 * QmitkRenderWindow.
 *
 * The menu follows these preferences of the "org.mitk.editors" node:
 * - "render window menu size": "smaller", "default" or "larger"
 * - "subdue render window menus": whether the bars rest subdued
 * - "max TS": the upper limit of the thick slices slider
 *
 * \sa QmitkRenderWindow
 *
 */
class MITKQTWIDGETS_EXPORT QmitkRenderWindowMenu : public QObject
{
  Q_OBJECT

public:

  using LayoutIndex = mitk::AnatomicalPlane;
  using LayoutDesign = QmitkMultiWidgetLayoutManager::LayoutDesign;

  /**
   * \param[in] parent The render window the menu belongs to. Its bars are
   *            child widgets of it.
   * \param[in] renderer The renderer of that window.
   *
   * \throws mitk::Exception if \p parent is nullptr.
   */
  QmitkRenderWindowMenu(QWidget *parent, mitk::BaseRenderer *renderer);
  ~QmitkRenderWindowMenu() override;

  /*! Return visibility of settings menu. The menu is connected with m_SettingsButton and includes
  layout direction (axial, coronal .. ) and layout design (standard layout, 2D images top,
  3D bottom ... ). */
  bool GetSettingsMenuVisibilty()
  {
    if (m_LayoutActionsMenu == nullptr)
      return false;
    else
      return m_LayoutActionsMenu->isVisible();
  }

  /*! Set layout index. Defines layout direction (axial, coronal, sagittal or threeD) of the parent. */
  void SetLayoutIndex(LayoutIndex layoutIndex);

  /*! Return layout direction of parent (axial, coronal, sagittal or threeD) */
  LayoutIndex GetLayoutIndex() { return m_Layout; }
  /*! Update list of layout design (standard layout, 2D images top, 3D bottom ..). Set action of current layout design
  to disable and all other to enable. */
  void UpdateLayoutDesignList(LayoutDesign layoutDesign);

  void UpdateCrosshairVisibility(bool visible);

  void UpdateCrosshair3DVisibility(bool visible);

  void UpdateCrosshairRotationMode(QmitkCrosshairRotationMode mode);

  /**
   * \brief The lighting rig last selected from this menu.
   *
   * Not necessarily the rig the renderer currently carries: anything that
   * installs one directly - a volume being rendered - overrides the selection
   * without replacing it. This is what the window falls back to once nothing
   * overrides it any more.
   */
  mitk::VtkPropRenderer::LightingMode GetPreferredLightingMode() const;

  /**
   * \brief Docks the bars to their corners after the window was resized, and
   * shows or hides them depending on whether the mouse cursor is over the
   * window now.
   */
  void MoveWidgetToCorrectPos();

  void ShowMenu();
  void HideMenu();

protected:

  /**
   * \brief Follows the mouse moves over the window, bringing the bars close
   * to the cursor to their full appearance.
   */
  bool eventFilter(QObject *watched, QEvent *event) override;

  void CreateMenuWidget();

  /*! Create settings menu which contains layout direction and the different layout designs. */
  void CreateSettingsWidget();

  /*! Change Icon of full-screen button depending on full-screen mode. */
  void ChangeFullScreenIcon();

Q_SIGNALS:

  void ResetView(); // fits the view(s) to all visible data

  void CrosshairVisibilityChanged(bool);

  void Crosshair3DVisibilityChanged(bool);

  void CrosshairRotationModeChanged(QmitkCrosshairRotationMode);

  void LightingModeChanged(mitk::VtkPropRenderer::LightingMode);

  /*! Emitted once the lighting menu has been rebuilt for an opening and before
      it pops up, so that receivers can add entries for this one opening. */
  void LightingMenuAboutToShow(QMenu* menu);

  /*! emit signal, when layout design changed by the setting menu.*/
  void LayoutDesignChanged(LayoutDesign layoutDesign);

protected Q_SLOTS:

  /// this function is invoked when the auto-rotate action
  /// is clicked
  void OnAutoRotationActionTriggered();

  void OnTSNumChanged(int);

  void OnCrosshairMenuAboutToShow();
  void OnCrosshairVisibilityChanged(bool);
  void OnCrosshair3DVisibilityChanged(bool);
  void OnCrosshairRotationModeSelected(QAction *);

  void OnLightingMenuAboutToShow();
  void OnLightingModeSelected(QAction *);

  /*! slot for activating/deactivating the full-screen mode. The slot is connected to the clicked() event of
  m_FullScreenButton.
  Activating the full-screen maximize the current widget, deactivating restore If layout design changed by the settings
  menu, the full-Screen mode is automatically switched to false. */
  void OnFullScreenButton(bool checked);

  void OnSetLayout(LayoutDesign layoutDesign);

protected:

  QToolButton* m_CrosshairModeButton;

  QToolButton* m_FullScreenButton;

  QToolButton* m_LayoutDesignButton;
  QToolButton* m_LightingModeButton;
  QMenu* m_LayoutActionsMenu;
  QAction* m_DefaultLayoutAction;
  QAction* m_All2DTop3DBottomLayoutAction;
  QAction* m_All2DLeft3DRightLayoutAction;
  QAction* m_OneBigLayoutAction;
  QAction* m_Only2DHorizontalLayoutAction;
  QAction* m_Only2DVerticalLayoutAction;
  QAction* m_OneTop3DBottomLayoutAction;
  QAction* m_OneLeft3DRightLayoutAction;
  QAction* m_AllHorizontalLayoutAction;
  QAction* m_AllVerticalLayoutAction;
  QAction* m_RemoveOneLayoutAction;

  QLabel *m_TSLabel;

  QMenu *m_CrosshairMenu;
  QMenu *m_LightingMenu;

  /*! Flag if full-screen mode is activated or deactivated. */
  bool m_FullScreenMode;

private:

  /** Shows the lighting button on 3D windows only. */
  void UpdateLightingModeButton();

  /** Shows the bars that have buttons to show, leaving out the upper left one if both do not fit. */
  void UpdateBarVisibility();

  void UpdateProximity(const QPoint &cursor);

  void ApplyPreferences();
  void OnPreferencesChanged(const mitk::IPreferences *preferences);

  void SetAutoRotation(bool enabled);

  /** Rotates the camera as far as the auto rotation turns in the given time. */
  void AutoRotate(double seconds);

  QmitkRenderWindowMenuBar *m_TopLeftBar;
  QmitkRenderWindowMenuBar *m_TopRightBar;

  /** Whether one of the popup menus is open. */
  bool m_PopupOpen;

  mitk::BaseRenderer::Pointer m_Renderer;

  /** Of the animation frame observer that rotates the camera, 0 while the auto rotation is off. */
  unsigned long m_AutoRotationObserverTag;

  QWidget *m_Parent;

  //memory because mode is set to default for slice num = 1
  static unsigned int m_DefaultThickMode;

  QmitkCrosshairRotationMode m_CrosshairRotationMode;
  bool m_CrosshairVisibility;
  bool m_Crosshair3DVisibility;

  mitk::VtkPropRenderer::LightingMode m_PreferredLightingMode;

  LayoutIndex m_Layout;
  LayoutDesign m_LayoutDesign;
  LayoutDesign m_OldLayoutDesign;

};

#endif
