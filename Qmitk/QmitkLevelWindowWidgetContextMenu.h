/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#ifndef QMITKLEVELWINDOWWIDGETCONTEXTMENU_H
#define QMITKLEVELWINDOWWIDGETCONTEXTMENU_H

#include <mitkLevelWindowPreset.h>
#include <mitkLevelWindowManager.h>
#include <qpopupmenu.h>

/**
  \class QmitkLevelWindowWidgetContextMenu QmitkLevelWindowWidgetContextMenu.h QmitkLevelWindowWidgetContextMenu.h
  \ingroup Widgets

  \brief Provides a contextmenu for Level/Window functionality. Either creates a new contextmenu with standard functions or adds Level/Window standard functions to an predefined contextmenu.

  */

class QmitkLevelWindowWidgetContextMenu : public QWidget {

  Q_OBJECT

public:

  /// constructor
  QmitkLevelWindowWidgetContextMenu(QWidget * parent, const char * name, WFlags f );
  virtual ~QmitkLevelWindowWidgetContextMenu();
  
  /*!
  *	data structure which reads and writes presets defined in a XML-file
  */
  mitk::LevelWindowPreset* m_LevelWindowPreset;

  /*!
  *	data structure which stores the values manipulated
  *	by a QmitkLevelWindowWidgetContextMenu
  */
  mitk::LevelWindow m_LevelWindow;

  /// submenu with all presets for contextmenu
  QPopupMenu* m_PresetSubmenu;

  /// submenu with all images for contextmenu
  QPopupMenu* m_ImageSubmenu;

  /// pointer to the object which manages all Level/Window changes on images and holds the LevelWindowProperty
  /// of the current image
  mitk::LevelWindowManager* m_Manager;

  /// map to hold all image-properties, one can get the image which is selected in the contextmenu
  /// with the integernumber representing the image for the contextmenu
  std::map<int, mitk::LevelWindowProperty::Pointer> m_Images;

  /*!
  * returns the contextmenu with standard functions for Level/Window
  *
  * input is a prefilled contextmenu to which standard functions will be added
  */
  void getContextMenu(QPopupMenu* contextmenu);

  /// returns the contextmenu with standard functions for Level/Window
  void getContextMenu();

  /// lets this object know about the LevelWindowManager to get all images and tell about changes
  void setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager);

protected:

  /// ID of preset selected in contextmenu
  int m_PresetID;

  /// ID of image selected in contextmenu
  int m_ImageID;

protected slots:

  /// sets level and window value of the current image to the values defined for the selected preset
  void setPreset(int presetID);

  /// calls the mitkLevelWindow SetAuto method with guessByCentralSlice false, so that the greyvalues from whole image will be considered
  void useAllGreyvaluesFromImage();

  /// sets the level window slider to be fixed
  void setFixed();

  /// adds a new Preset for presets-contextmenu
  void addPreset();

  /// resets the current images Level/Window to its default values
  void setDefaultLevelWindow();

  /// resets the current images scalerange to its default values
  void setDefaultScaleRange();

  /// changes the current images scalerange
  void changeScaleRange();

  /// sets the selected image or the topmost layer image to the new current image
  void setImage(int imageID);

  /// sets the window to its maximum Size to fit the scalerange
  void setMaximumWindow();

};
#endif
