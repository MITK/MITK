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

class QmitkLevelWindowWidgetContextMenu : public QWidget {

  Q_OBJECT

public:
  QmitkLevelWindowWidgetContextMenu(QWidget * parent, const char * name, WFlags f );
  
  mitk::LevelWindowPreset m_Pre;
  mitk::LevelWindow m_Lw;
  QPopupMenu* m_PresetSubmenu;
  QPopupMenu* m_ImageSubmenu;
  mitk::LevelWindowManager* m_Manager;
  std::map<int, mitk::LevelWindowProperty::Pointer> m_Images;

  void getContextMenu(QPopupMenu* cm);
  void getContextMenu();
  void setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager);

protected:
  int m_PresetID;
  int m_ImageID;

protected slots:

  void setPreset(int id);
  void addPreset();
  void setDefaultLevelWindow();
  void setDefaultScaleRange();
  void changeScaleRange();
  void setImage(int id);

};
#endif
