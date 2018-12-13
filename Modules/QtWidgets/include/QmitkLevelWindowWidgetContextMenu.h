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

#ifndef QMITKLEVELWINDOWWIDGETCONTEXTMENU_H
#define QMITKLEVELWINDOWWIDGETCONTEXTMENU_H

#include <MitkQtWidgetsExports.h>

#include <QMenu>
#include <mitkLevelWindowManager.h>
#include <mitkLevelWindowPreset.h>

/**
 * \ingroup QmitkModule
 * \brief Provides a contextmenu for Level/Window functionality.
 *
 * Either creates
 * a new contextmenu with standard functions or adds Level/Window standard
 * functions to an predefined contextmenu.
 */
class MITKQTWIDGETS_EXPORT QmitkLevelWindowWidgetContextMenu : public QWidget
{
  Q_OBJECT

public:
  /// constructor
  QmitkLevelWindowWidgetContextMenu(QWidget *parent, Qt::WindowFlags f = nullptr);
  ~QmitkLevelWindowWidgetContextMenu() override;

  /*!
  *  data structure which reads and writes presets defined in a XML-file
  */
  mitk::LevelWindowPreset *m_LevelWindowPreset;

  /*!
  *  data structure which stores the values manipulated
  *  by a QmitkLevelWindowWidgetContextMenu
  */
  mitk::LevelWindow m_LevelWindow;

  /// submenu with all presets for contextmenu
  QMenu *m_PresetSubmenu;

  /// submenu with all images for contextmenu
  QMenu *m_ImageSubmenu;

  /// pointer to the object which manages all Level/Window changes on images and holds the LevelWindowProperty
  /// of the current image
  mitk::LevelWindowManager *m_Manager;

  /// map to hold all image-properties, one can get the image which is selected in the contextmenu
  /// with the QAction representing the image for the contextmenu
  std::map<QAction *, mitk::LevelWindowProperty::Pointer> m_Images;

  /*!
  * returns the contextmenu with standard functions for Level/Window
  *
  * input is a prefilled contextmenu to which standard functions will be added
  */
  void GetContextMenu(QMenu *contextMenu);

  /// returns the contextmenu with standard functions for Level/Window
  void GetContextMenu();

  /// lets this object know about the LevelWindowManager to get all images and tell about changes
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

#endif // QMITKLEVELWINDOWWIDGETCONTEXTMENU_H
