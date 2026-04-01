/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLevelWindowWidgetContextMenu_h
#define QmitkLevelWindowWidgetContextMenu_h

#include <MitkQtWidgetsExports.h>

#include <QMenu>
#include <mitkLevelWindowManager.h>
#include <mitkLevelWindowPreset.h>

/**
 * \ingroup QmitkModule
 * \brief Provides a context menu for level/window manipulation.
 *
 * This widget creates a context menu offering standard level/window operations
 * such as preset selection, auto-optimization, scale range changes, and image
 * selection. It can either generate a standalone context menu or append its
 * entries to an existing QMenu.
 *
 * \sa QmitkSliderLevelWindowWidget
 * \sa QmitkLineEditLevelWindowWidget
 * \sa QmitkLevelWindowWidget
 * \sa mitk::LevelWindowManager
 */
class MITKQTWIDGETS_EXPORT QmitkLevelWindowWidgetContextMenu : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the context menu widget.
   * \param[in] parent The parent widget.
   * \param[in] f      Optional window flags.
   */
  QmitkLevelWindowWidgetContextMenu(QWidget *parent, Qt::WindowFlags f = {});
  ~QmitkLevelWindowWidgetContextMenu() override;

  /** \brief Handles reading and writing of level/window presets from an XML file. */
  mitk::LevelWindowPreset *m_LevelWindowPreset;

  /** \brief Stores the current level/window values manipulated by the context menu. */
  mitk::LevelWindow m_LevelWindow;

  /** \brief Submenu listing all available presets. */
  QMenu *m_PresetSubmenu;

  /** \brief Submenu listing all available images. */
  QMenu *m_ImageSubmenu;

  /** \brief The LevelWindowManager that tracks level/window changes and holds the current image's LevelWindowProperty. */
  mitk::LevelWindowManager *m_Manager;

  /** \brief Maps context menu actions to their associated LevelWindowProperty for image selection. */
  std::map<QAction *, mitk::LevelWindowProperty::Pointer> m_Images;

  /**
   * \brief Appends standard level/window actions to an existing context menu.
   * \param[in,out] contextMenu The menu to which level/window actions are appended.
   */
  void GetContextMenu(QMenu *contextMenu);

  /**
   * \brief Creates and shows a standalone context menu with standard level/window actions.
   */
  void GetContextMenu();

  /**
   * \brief Sets the LevelWindowManager used for image queries and change notifications.
   * \param[in] levelWindowManager The LevelWindowManager to use.
   */
  void SetLevelWindowManager(mitk::LevelWindowManager *levelWindowManager);

protected:

  QAction *m_PresetAction;
  QAction *m_AutoTopmostAction;
  QAction *m_SelectedImagesAction;

protected Q_SLOTS:

  /// sets level and window value of the current image to the values defined for the selected preset
  void OnSetPreset(const QAction *presetAction);

  /// calls the mitkLevelWindow SetAuto method with guessByCentralSlice false, so that the greyvalues from whole image
  /// will be considered
  void OnUseOptimizedLevelWindow();

  /// calls the mitkLevelWindow SetToImageRange method, so that the greyvalues from whole image will be used
  void OnUseAllGreyvaluesFromImage();

  /// sets the level window slider to be fixed
  void OnSetFixed();

  /// adds a new Preset for presets-contextmenu
  void OnAddPreset();

  /// resets the current images Level/Window to its default values
  void OnSetDefaultLevelWindow();

  /// resets the current images scalerange to its default values
  void OnSetDefaultScaleRange();

  /// changes the current images scalerange
  void OnChangeScaleRange();

  /// sets the selected image or the topmost layer image to the new current image
  void OnSetImage(QAction *imageAction);

  /// sets the window to its maximum Size to fit the scalerange
  void OnSetMaximumWindow();
};

#endif
