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

#ifndef QMITKSPINBOXLEVELWINDOWWIDGET
#define QMITKSPINBOXLEVELWINDOWWIDGET

#include <qwidget.h>
#include <mitkLevelWindowManager.h>

class QmitkLevelWindowWidgetContextMenu;
class QLineEdit;

/**
  \class QmitkLineEditLevelWindowWidget QmitkLineEditLevelWindowWidget.h QmitkLineEditLevelWindowWidget.h
  \ingroup Widgets

  \brief Provides a widget with two lineedit fields, one to change the window value of the current image and one to change the level value of the current image.

  */

class QMITK_EXPORT QmitkLineEditLevelWindowWidget : public QWidget {

  Q_OBJECT

public:

  /// constructor
  QmitkLineEditLevelWindowWidget( QWidget * parent=0, const char * name=0, WFlags f = false );

  /// destructor
  ~QmitkLineEditLevelWindowWidget();
  
  /// inputfield for level value
  QLineEdit* m_LevelInput;

  /// inputfield for window value
  QLineEdit* m_WindowInput;

  /*!
  *	data structure which stores the values manipulated
  *	by a QmitkLineEditLevelWindowWidget
  */
  mitk::LevelWindow m_LevelWindow;

  /// manager who is responsible to collect and deliver changes on Level/Window
  mitk::LevelWindowManager::Pointer m_Manager;
  
  /// sets the manager who is responsible to collect and deliver changes on Level/Window
  void setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager);

  /// sets the DataTree which holds all image-nodes
  void setDataTree( mitk::DataTree* tree );

  /// returns the manager who is responsible to collect and deliver changes on Level/Window
  mitk::LevelWindowManager* GetManager();

private:

  /// creates the contextmenu for this widget from class QmitkLevelWindowWidgetContextMenu
  void contextMenuEvent ( QContextMenuEvent * );

  /// change notifications from the mitkLevelWindowManager
  void OnPropertyModified(const itk::EventObject& e);

  /*!
  * tests if new level + window/2 <= maxRange, if not level would be set to maxRange - window/2
  *
  * tests if new level - window/2 >= maxRange, if not level would be set to minRange + window/2
  *
  * window keeps its old value
  */
  void validLevel();

  /*!
  * tests if current level + window/2 <= maxRange, if not window/2 would be set to maxRange - level
  *
  * tests if current level - window/2 >= minRange, if not window/2 would be set to level - minRange
  *
  * level keeps its old value
  */
  void validWindow();

public slots:

  /// called when return is pressed in levelinput field
  void SetLevelValue();

  /// called when return is pressed in windowinput field
  void SetWindowValue();
  
  // validator to accept only possible values for Level/Window in lineedits
  //void setValidator();

protected:
  unsigned long m_ObserverTag;
  bool m_IsObserverTagSet;

  /*!
  *	data structure which creates the contextmenu for QmitkLineEditLevelWindowWidget
  */
  QmitkLevelWindowWidgetContextMenu* m_Contextmenu;

};
#endif
