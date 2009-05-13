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

#ifndef QMITKLEVELWINDOWWIDGET_H_
#define QMITKLEVELWINDOWWIDGET_H_

#include "ui_QmitkLevelWindowWidget.h"

#include <QWidget>

class QMITK_EXPORT QmitkLevelWindowWidget : public QWidget, public Ui::QmitkLevelWindow
{
  Q_OBJECT
public:
  QmitkLevelWindowWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
  mitk::LevelWindowManager* GetManager();
  
public slots:
  void SetDataStorage( mitk::DataStorage* ds );
  
protected:
  //unsigned long m_ObserverTag;
  mitk::LevelWindowManager::Pointer m_Manager;
};
#endif /*QMITKLEVELWINDOWWIDGET_H_*/
