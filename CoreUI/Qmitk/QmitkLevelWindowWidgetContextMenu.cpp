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

#include <QmitkLevelWindowWidgetContextMenu.h>
#include "QmitkLevelWindowPresetDefinitionDialog.h"
#include "QmitkLevelWindowRangeChangeDialog.h"
#include <QCursor>
#include <mitkRenderingManager.h>

QmitkLevelWindowWidgetContextMenu::QmitkLevelWindowWidgetContextMenu(QWidget * parent, Qt::WindowFlags f )
: QWidget( parent, f )
{
  m_LevelWindowPreset = mitk::LevelWindowPreset::New();
  m_LevelWindowPreset->LoadPreset();
}

QmitkLevelWindowWidgetContextMenu::~QmitkLevelWindowWidgetContextMenu()
{
  m_LevelWindowPreset->Delete();
}

void QmitkLevelWindowWidgetContextMenu::setPreset(QAction* presetAction)
{
  QString item = presetAction->text();
  if (!(presetAction == m_PresetAction))
  {
    double dlevel = m_LevelWindowPreset->getLevel(item.toStdString());
    double dwindow = m_LevelWindowPreset->getWindow(item.toStdString());
    if ((dlevel + dwindow/2) > m_LevelWindow.GetRangeMax())
    {
      double lowerBound = (dlevel - dwindow/2);
      if (!(lowerBound > m_LevelWindow.GetRangeMax()))
      {
        dwindow = m_LevelWindow.GetRangeMax() - lowerBound;
        dlevel = lowerBound + dwindow/2;
      }
      else
      {
        dlevel = m_LevelWindow.GetRangeMax() - 1;
        dwindow = 2;
      }
    }
    else if ((dlevel - dwindow/2) < m_LevelWindow.GetRangeMin())
    {
      double upperBound = (dlevel + dwindow/2);
      if (!(upperBound < m_LevelWindow.GetRangeMin()))
      {
        dwindow = m_LevelWindow.GetRangeMin() + upperBound;
        dlevel = upperBound - dwindow/2;
      }
      else
      {
        dlevel = m_LevelWindow.GetRangeMin() + 1;
        dwindow = 2;
      }
    }
    m_LevelWindow.SetLevelWindow(dlevel, dwindow);
    m_Manager->SetLevelWindow(m_LevelWindow);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLevelWindowWidgetContextMenu::setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager)
{
  m_Manager = levelWindowManager;
}

void QmitkLevelWindowWidgetContextMenu::addPreset()
{
  QmitkLevelWindowPresetDefinitionDialog addPreset(this);
  addPreset.setPresets(m_LevelWindowPreset->getLevelPresets(), m_LevelWindowPreset->getWindowPresets(), QString::number( (int) m_LevelWindow.GetLevel() ), QString::number( (int) m_LevelWindow.GetWindow() ));
  if(addPreset.exec())
  {
    m_LevelWindowPreset->newPresets(addPreset.getLevelPresets(), addPreset.getWindowPresets());
  }
}

void QmitkLevelWindowWidgetContextMenu::setFixed()
{
  m_LevelWindow.SetFixed(!m_LevelWindow.GetFixed());
  m_Manager->SetLevelWindow(m_LevelWindow);
}

void QmitkLevelWindowWidgetContextMenu::useAllGreyvaluesFromImage()
{
  m_LevelWindow.SetAuto(m_Manager->GetCurrentImage(), true, false);
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLevelWindowWidgetContextMenu::setDefaultLevelWindow()
{
  m_LevelWindow.ResetDefaultLevelWindow();
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLevelWindowWidgetContextMenu::setMaximumWindow()
{
  m_LevelWindow.SetToMaxWindowSize();
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLevelWindowWidgetContextMenu::setDefaultScaleRange()
{
  m_LevelWindow.ResetDefaultRangeMinMax();
  m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel(), m_LevelWindow.GetWindow());
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLevelWindowWidgetContextMenu::changeScaleRange()
{
  QmitkLevelWindowRangeChangeDialog changeRange(this);
  changeRange.setLowerLimit((int)m_LevelWindow.GetRangeMin());
  changeRange.setUpperLimit((int)m_LevelWindow.GetRangeMax());
  if(changeRange.exec())
  {
    m_LevelWindow.SetRangeMinMax(changeRange.getLowerLimit(), changeRange.getUpperLimit());
    m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel(), m_LevelWindow.GetWindow());
    m_Manager->SetLevelWindow(m_LevelWindow);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLevelWindowWidgetContextMenu::setImage(QAction* imageAction)
{
  if (imageAction == m_ImageAction)
    if ( m_Manager->isAutoTopMost() == false)
      m_Manager->SetAutoTopMostImage(true);
    else
      m_Manager->SetAutoTopMostImage(false);
  else
    m_Manager->SetLevelWindowProperty(m_Images[imageAction]);
}

void QmitkLevelWindowWidgetContextMenu::getContextMenu(QMenu* contextmenu)
{
  try
  {
    m_LevelWindow = m_Manager->GetLevelWindow();

    QMenu* contextMenu = contextmenu;
    Q_CHECK_PTR( contextMenu );
    //contextMenu->setCheckable(true);
    QAction* sliderFixed = contextMenu->addAction(tr("Set Slider Fixed"), this, SLOT(setFixed()));
    sliderFixed->setCheckable(true);
    sliderFixed->setChecked(m_LevelWindow.IsFixed());
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Use Whole Image Greyvalues"), this, SLOT(useAllGreyvaluesFromImage()));
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Set Maximum Window"), this, SLOT(setMaximumWindow()));
    contextMenu->addAction(tr("Default Level/Window"), this, SLOT(setDefaultLevelWindow()));
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Change Scale Range"), this, SLOT(changeScaleRange()));
    contextMenu->addAction(tr("Default Scale Range"), this, SLOT(setDefaultScaleRange()));
    contextMenu->addSeparator();

    m_PresetSubmenu = new QMenu( this );
    Q_CHECK_PTR( m_PresetSubmenu );
    m_PresetSubmenu->setTitle("Presets");
    m_PresetAction = m_PresetSubmenu->addAction(tr("Preset Definition"), this, SLOT(addPreset()));
    m_PresetSubmenu->addSeparator();
    std::map<std::string, double> preset = m_LevelWindowPreset->getLevelPresets();
    for( std::map<std::string, double>::iterator iter = preset.begin(); iter != preset.end(); iter++ ) {
      QString item = ((*iter).first.c_str());
      m_PresetSubmenu->addAction(item);
    }
    connect(m_PresetSubmenu, SIGNAL(triggered(QAction*)), this, SLOT(setPreset(QAction*)));
    contextMenu->addMenu(m_PresetSubmenu);
    contextMenu->addSeparator();

    m_ImageSubmenu = new QMenu( this );
    m_ImageSubmenu->setTitle("Images");
    //m_ImageSubmenu->setCheckable(true);
    m_ImageAction = m_ImageSubmenu->addAction(tr("Set Topmost Image"));
    m_ImageAction->setCheckable(true);
    if (m_Manager->isAutoTopMost())
      m_ImageAction->setChecked(true);
    m_ImageSubmenu->addSeparator();
    Q_CHECK_PTR( m_ImageSubmenu );
    mitk::DataStorage::SetOfObjects::ConstPointer allObjects = m_Manager->GetRelevantNodes();
    for ( mitk::DataStorage::SetOfObjects::ConstIterator objectIter = allObjects->Begin(); objectIter != allObjects->End(); ++objectIter)
    {
      mitk::DataNode* node = objectIter->Value();
      if (node)
      {
        if (node->IsVisible(NULL) == false)
          continue;
        mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow"));
        if (levelWindowProperty.IsNotNull())
        {
          std::string name;
          node->GetName(name);
          QString item = name.c_str();
          QAction* id = m_ImageSubmenu->addAction(item);
          id->setCheckable(true);
          m_Images[id] = levelWindowProperty;
          if (levelWindowProperty == m_Manager->GetLevelWindowProperty())
          {
            id->setChecked(true);
          }
        }
      }
    }
    connect(m_ImageSubmenu, SIGNAL(triggered(QAction*)), this, SLOT(setImage(QAction*)));
    contextMenu->addMenu( m_ImageSubmenu );

    contextMenu->exec( QCursor::pos() );
    delete contextMenu;
  }
  catch(...)
  {
  }
}

void QmitkLevelWindowWidgetContextMenu::getContextMenu()
{
  try
  {
    m_LevelWindow = m_Manager->GetLevelWindow();

    QMenu* contextMenu = new QMenu( this );
    Q_CHECK_PTR( contextMenu );
    //contextMenu->setCheckable(true);
    QAction* sliderFixed = contextMenu->addAction(tr("Set Slider Fixed"), this, SLOT(setFixed()));
    sliderFixed->setCheckable(true);
    sliderFixed->setChecked(m_LevelWindow.IsFixed());
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Use Whole Image Greyvalues"), this, SLOT(useAllGreyvaluesFromImage()));
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Set Maximum Window"), this, SLOT(setMaximumWindow()));
    contextMenu->addAction(tr("Default Level/Window"), this, SLOT(setDefaultLevelWindow()));
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Change Scale Range"), this, SLOT(changeScaleRange()));
    contextMenu->addAction(tr("Default Scale Range"), this, SLOT(setDefaultScaleRange()));
    contextMenu->addSeparator();

    m_PresetSubmenu = new QMenu( this );
    Q_CHECK_PTR( m_PresetSubmenu );
    m_PresetSubmenu->setTitle("Presets");
    m_PresetAction = m_PresetSubmenu->addAction(tr("Preset Definition"), this, SLOT(addPreset()));
    m_PresetSubmenu->addSeparator();
    std::map<std::string, double> preset = m_LevelWindowPreset->getLevelPresets();
    for( std::map<std::string, double>::iterator iter = preset.begin(); iter != preset.end(); iter++ ) {
      QString item = ((*iter).first.c_str());
      m_PresetSubmenu->addAction(item);
    }
    connect(m_PresetSubmenu, SIGNAL(triggered(QAction*)), this, SLOT(setPreset(QAction*)));
    contextMenu->addMenu( m_PresetSubmenu );
    contextMenu->addSeparator();

    m_ImageSubmenu = new QMenu( this );
    m_ImageSubmenu->setTitle("Images");
    //m_ImageSubmenu->setCheckable(true);
    m_ImageAction = m_ImageSubmenu->addAction(tr("Set Topmost Image"));
    m_ImageAction->setCheckable(true);
    if (m_Manager->isAutoTopMost())
      m_ImageAction->setChecked(true);
    m_ImageSubmenu->addSeparator();
    Q_CHECK_PTR( m_ImageSubmenu );
    mitk::DataStorage::SetOfObjects::ConstPointer allObjects = m_Manager->GetRelevantNodes();
    for ( mitk::DataStorage::SetOfObjects::ConstIterator objectIter = allObjects->Begin(); objectIter != allObjects->End(); ++objectIter)
    {
      mitk::DataNode* node = objectIter->Value();
      if (node)
      {
        mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow"));
        if (levelWindowProperty.IsNotNull())
        {
          std::string name;
          node->GetName(name);
          QString item = name.c_str();
          QAction* id = m_ImageSubmenu->addAction(item);
          id->setCheckable(true);
          m_Images[id] = levelWindowProperty;
          if (levelWindowProperty == m_Manager->GetLevelWindowProperty())
          {
            id->setChecked(true);
          }
        }
      }
    }
    connect(m_ImageSubmenu, SIGNAL(activated(int)), this, SLOT(setImage(int)));
    contextMenu->addMenu( m_ImageSubmenu );

    contextMenu->exec( QCursor::pos() );
    delete contextMenu;
  }
  catch(...)
  {
  }
}
