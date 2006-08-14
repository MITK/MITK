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

#include <QmitkLevelWindowWidgetContextMenu.h>
#include <QmitkLevelWindowPresetDefinition.h>
#include <QmitkLevelWindowRangeChange.h>
#include <qcursor.h>

QmitkLevelWindowWidgetContextMenu::QmitkLevelWindowWidgetContextMenu(QWidget * parent, const char * name, WFlags f )
: QWidget( parent, name, f ), m_Pre()
{
  m_Pre.LoadPreset();
}

void QmitkLevelWindowWidgetContextMenu::setPreset(int id)
{
  QString item = m_PresetSubmenu->text(id);
  if (!(id == m_PresetID))
  {
    double dlevel = m_Pre.getLevel(std::string((const char*)item));
    double dwindow = m_Pre.getWindow(std::string((const char*)item));
    if ((dlevel + dwindow/2) > m_Lw.GetRangeMax())
    {
      double lowerBound = (dlevel - dwindow/2);
      if (!(lowerBound > m_Lw.GetRangeMax()))
      {
        dwindow = m_Lw.GetRangeMax() - lowerBound;
        dlevel = lowerBound + dwindow/2;
      }
      else
      {
        dlevel = m_Lw.GetRangeMax() - 1;
        dwindow = 2;
      }
    }
    else if ((dlevel - dwindow/2) < m_Lw.GetRangeMin())
    {
      double upperBound = (dlevel + dwindow/2);
      if (!(upperBound < m_Lw.GetRangeMin()))
      {
        dwindow = m_Lw.GetRangeMin() + upperBound;
        dlevel = upperBound - dwindow/2;
      }
      else
      {
        dlevel = m_Lw.GetRangeMin() + 1;
        dwindow = 2;
      }
    }
    m_Lw.SetLevelWindow(dlevel, dwindow);
    m_Manager->SetLevelWindow(m_Lw);
  }
}

void QmitkLevelWindowWidgetContextMenu::setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager)
{
  m_Manager = levelWindowManager;
}

void QmitkLevelWindowWidgetContextMenu::addPreset()
{
  QmitkLevelWindowPresetDefinition addPreset(this, "newPreset", TRUE);
  addPreset.setPresets(m_Pre.getLevelPresets(), m_Pre.getWindowPresets(), QString::number( (int) m_Lw.GetLevel() ), QString::number( (int) m_Lw.GetWindow() ));
  if(addPreset.exec())
  {
    m_Pre.newPresets(addPreset.getLevelPresets(), addPreset.getWindowPresets());
  }
}

void QmitkLevelWindowWidgetContextMenu::setDefaultLevelWindow()
{
  m_Lw.SetLevelWindow(m_Lw.GetDefaultLevel(), m_Lw.GetDefaultWindow());
  m_Manager->SetLevelWindow(m_Lw);
}

void QmitkLevelWindowWidgetContextMenu::setDefaultScaleRange()
{
  m_Lw.SetRangeMinMax(m_Lw.GetDefaultRangeMin(), m_Lw.GetDefaultRangeMax());
  m_Lw.SetLevelWindow(m_Lw.GetLevel(), m_Lw.GetWindow());
  m_Manager->SetLevelWindow(m_Lw);
}

void QmitkLevelWindowWidgetContextMenu::changeScaleRange()
{
  QmitkLevelWindowRangeChange changeRange(this, "changeRange", TRUE);
  changeRange.setLowerLimit((int)m_Lw.GetRangeMin());
  changeRange.setUpperLimit((int)m_Lw.GetRangeMax());
  if(changeRange.exec())
  {
    m_Lw.SetRangeMinMax(changeRange.getLowerLimit(), changeRange.getUpperLimit());
    m_Lw.SetLevelWindow(m_Lw.GetLevel(), m_Lw.GetWindow());
    m_Manager->SetLevelWindow(m_Lw);
  }
}

void QmitkLevelWindowWidgetContextMenu::setImage(int id)
{
  if (id == m_ImageID)
    m_Manager->SetAutoTopMostImage();
  else
    m_Manager->SetLevWinProp(m_Images[id]);
}

void QmitkLevelWindowWidgetContextMenu::getContextMenu(QPopupMenu* cm)
{
  try
  {
    m_Lw = m_Manager->GetLevelWindow();
    mitk::DataTreeIteratorClone dataTreeIteratorClone = m_Manager->GetDataTreeIteratorClone();
    QPopupMenu* contextMenu = cm;
    Q_CHECK_PTR( contextMenu );
    contextMenu->insertItem(tr("Default Level/Window"), this, SLOT(setDefaultLevelWindow()));
    contextMenu->insertSeparator();
    contextMenu->insertItem(tr("Change Scale Range"), this, SLOT(changeScaleRange()));
    contextMenu->insertItem(tr("Default Scale Range"), this, SLOT(setDefaultScaleRange()));
    contextMenu->insertSeparator();
    
    m_PresetSubmenu = new QPopupMenu( this );
    Q_CHECK_PTR( m_PresetSubmenu );
    m_PresetID = m_PresetSubmenu->insertItem(tr("Preset Definition"), this, SLOT(addPreset()));
    m_PresetSubmenu->insertSeparator();
    std::map<std::string, double> pres = m_Pre.getLevelPresets();
    for( std::map<std::string, double>::iterator iter = pres.begin(); iter != pres.end(); iter++ ) {
      QString item = ((*iter).first.c_str());
      m_PresetSubmenu->insertItem(item);
    }
    connect(m_PresetSubmenu, SIGNAL(activated(int)), this, SLOT(setPreset(int)));
    contextMenu->insertItem( "Presets",  m_PresetSubmenu );
    contextMenu->insertSeparator();

    m_ImageSubmenu = new QPopupMenu( this );
    m_ImageSubmenu->setCheckable(TRUE);
    m_ImageID = m_ImageSubmenu->insertItem(tr("Set Topmost Image"));
    if (m_Manager->isAutoTopMost())
      m_ImageSubmenu->setItemChecked(m_ImageID, true);
    m_ImageSubmenu->insertSeparator();
    Q_CHECK_PTR( m_ImageSubmenu );
    dataTreeIteratorClone->GoToBegin();
    while ( !dataTreeIteratorClone->IsAtEnd() )
    {
      if ( (dataTreeIteratorClone->Get().IsNotNull()) && (dataTreeIteratorClone->Get()->IsVisible(NULL)) )
      {
        bool binary = false;
        dataTreeIteratorClone->Get()->GetBoolProperty("binary", binary);
        if( binary == false)
        { 
          mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(dataTreeIteratorClone->Get()->GetProperty("levelwindow").GetPointer());
          if (levWinProp.IsNotNull())
          {
            std::string name;
            dataTreeIteratorClone->Get()->GetName(name);
            QString item = name.c_str();
            int id = m_ImageSubmenu->insertItem(item);
            m_Images[id] = levWinProp;
            if (levWinProp == m_Manager->GetLevWinProp())
            {
              m_ImageSubmenu->setItemChecked(id, true);
            }
          }
        }
      }
      ++dataTreeIteratorClone;
    }
    connect(m_ImageSubmenu, SIGNAL(activated(int)), this, SLOT(setImage(int)));
    contextMenu->insertItem( "Images",  m_ImageSubmenu );

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
    m_Lw = m_Manager->GetLevelWindow();
    mitk::DataTreeIteratorClone dataTreeIteratorClone = m_Manager->GetDataTreeIteratorClone();
    QPopupMenu* contextMenu = new QPopupMenu( this );
    Q_CHECK_PTR( contextMenu );
    contextMenu->insertItem(tr("Default Level/Window"), this, SLOT(setDefaultLevelWindow()));
    contextMenu->insertSeparator();
    contextMenu->insertItem(tr("Change Scale Range"), this, SLOT(changeScaleRange()));
    contextMenu->insertItem(tr("Default Scale Range"), this, SLOT(setDefaultScaleRange()));
    contextMenu->insertSeparator();
    
    m_PresetSubmenu = new QPopupMenu( this );
    Q_CHECK_PTR( m_PresetSubmenu );
    m_PresetID = m_PresetSubmenu->insertItem(tr("Preset Definition"), this, SLOT(addPreset()));
    m_PresetSubmenu->insertSeparator();
    std::map<std::string, double> pres = m_Pre.getLevelPresets();
    for( std::map<std::string, double>::iterator iter = pres.begin(); iter != pres.end(); iter++ ) {
      QString item = ((*iter).first.c_str());
      m_PresetSubmenu->insertItem(item);
    }
    connect(m_PresetSubmenu, SIGNAL(activated(int)), this, SLOT(setPreset(int)));
    contextMenu->insertItem( "Presets",  m_PresetSubmenu );
    contextMenu->insertSeparator();

    m_ImageSubmenu = new QPopupMenu( this );
    m_ImageSubmenu->setCheckable(TRUE);
    m_ImageID = m_ImageSubmenu->insertItem(tr("Set Topmost Image"));
    if (m_Manager->isAutoTopMost())
      m_ImageSubmenu->setItemChecked(m_ImageID, true);
    m_ImageSubmenu->insertSeparator();
    Q_CHECK_PTR( m_ImageSubmenu );
    dataTreeIteratorClone->GoToBegin();
    while ( !dataTreeIteratorClone->IsAtEnd() )
    {
      if ( (dataTreeIteratorClone->Get().IsNotNull()) && (dataTreeIteratorClone->Get()->IsVisible(NULL)) )
      {
        bool binary = false;
        dataTreeIteratorClone->Get()->GetBoolProperty("binary", binary);
        if( binary == false)
        { 
          mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(dataTreeIteratorClone->Get()->GetProperty("levelwindow").GetPointer());
          if (levWinProp.IsNotNull())
          {
            std::string name;
            dataTreeIteratorClone->Get()->GetName(name);
            QString item = name.c_str();
            int id = m_ImageSubmenu->insertItem(item);
            m_Images[id] = levWinProp;
            if (levWinProp == m_Manager->GetLevWinProp())
            {
              m_ImageSubmenu->setItemChecked(id, true);
            }
          }
        }
      }
      ++dataTreeIteratorClone;
    }
    connect(m_ImageSubmenu, SIGNAL(activated(int)), this, SLOT(setImage(int)));
    contextMenu->insertItem( "Images",  m_ImageSubmenu );

    contextMenu->exec( QCursor::pos() );
    delete contextMenu;
  }
  catch(...)
  {
  }
}
