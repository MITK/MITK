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

#ifndef QMITKSPINBOXLEVELWINDOWWIDGET
#define QMITKSPINBOXLEVELWINDOWWIDGET

#include <QmitkLevelWindowWidgetContextMenu.h>
#include <qlineedit.h>

class QmitkLineEditLevelWindowWidget : public QWidget {

  Q_OBJECT

public:

  QmitkLineEditLevelWindowWidget( QWidget * parent=0, const char * name=0, WFlags f = false );

  QLineEdit* m_LevelInput;
  QLineEdit* m_WindowInput;
  mitk::LevelWindow m_Lw;
  mitk::LevelWindowManager::Pointer m_Manager;
  
  void setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager);
  void setDataTreeIteratorClone( mitk::DataTreeIteratorClone& it );

private:
  void contextMenuEvent ( QContextMenuEvent * );
  void OnPropertyModified(const itk::EventObject& e);

public slots:
  void SetLevelValue();
  void SetWindowValue();
  void setValidator();

protected:
  mitk::DataTreeIteratorClone m_It;
  unsigned long m_ObserverTag;
  bool m_IsObserverTagSet;
  QmitkLevelWindowWidgetContextMenu* m_Contextmenu;
  bool m_SelfCall;

};
#endif
