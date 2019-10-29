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

#include <QmitkLevelWindowWidgetContextMenu.h>

// mitk core
#include <mitkRenderingManager.h>

// mitk qt widgets
#include "QmitkLevelWindowPresetDefinitionDialog.h"
#include "QmitkLevelWindowRangeChangeDialog.h"

// qt
#include <QCursor>

QmitkLevelWindowWidgetContextMenu::QmitkLevelWindowWidgetContextMenu(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_LevelWindowPreset = mitk::LevelWindowPreset::New();
  m_LevelWindowPreset->LoadPreset();
}

QmitkLevelWindowWidgetContextMenu::~QmitkLevelWindowWidgetContextMenu()
{
  m_LevelWindowPreset->Delete();
}

void QmitkLevelWindowWidgetContextMenu::OnSetPreset(const QAction *presetAction)
{
  QString item = presetAction->text();
  if (!(presetAction == m_PresetAction))
  {
    double dlevel = m_LevelWindowPreset->getLevel(item.toStdString());
    double dwindow = m_LevelWindowPreset->getWindow(item.toStdString());
    if ((dlevel + dwindow / 2) > m_LevelWindow.GetRangeMax())
    {
      double lowerBound = (dlevel - dwindow / 2);
      if (!(lowerBound > m_LevelWindow.GetRangeMax()))
      {
        dwindow = m_LevelWindow.GetRangeMax() - lowerBound;
        dlevel = lowerBound + dwindow / 2;
      }
      else
      {
        dlevel = m_LevelWindow.GetRangeMax() - 1;
        dwindow = 2;
      }
    }
    else if ((dlevel - dwindow / 2) < m_LevelWindow.GetRangeMin())
    {
      double upperBound = (dlevel + dwindow / 2);
      if (!(upperBound < m_LevelWindow.GetRangeMin()))
      {
        dwindow = m_LevelWindow.GetRangeMin() + upperBound;
        dlevel = upperBound - dwindow / 2;
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

void QmitkLevelWindowWidgetContextMenu::SetLevelWindowManager(mitk::LevelWindowManager *levelWindowManager)
{
  m_Manager = levelWindowManager;
}

void QmitkLevelWindowWidgetContextMenu::OnAddPreset()
{
  QmitkLevelWindowPresetDefinitionDialog addPreset(this);
  addPreset.setPresets(m_LevelWindowPreset->getLevelPresets(),
                       m_LevelWindowPreset->getWindowPresets(),
                       QString::number((int)m_LevelWindow.GetLevel()),
                       QString::number((int)m_LevelWindow.GetWindow()));
  if (addPreset.exec())
  {
    m_LevelWindowPreset->newPresets(addPreset.getLevelPresets(), addPreset.getWindowPresets());
  }
}

void QmitkLevelWindowWidgetContextMenu::OnSetFixed()
{
  m_LevelWindow.SetFixed(!m_LevelWindow.GetFixed());
  m_Manager->SetLevelWindow(m_LevelWindow);
}

void QmitkLevelWindowWidgetContextMenu::OnUseAllGreyvaluesFromImage()
{
  m_LevelWindow.SetToImageRange(m_Manager->GetCurrentImage());
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLevelWindowWidgetContextMenu::OnUseOptimizedLevelWindow()
{
  m_LevelWindow.SetAuto(m_Manager->GetCurrentImage(), false, false);
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLevelWindowWidgetContextMenu::OnSetDefaultLevelWindow()
{
  m_LevelWindow.ResetDefaultLevelWindow();
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLevelWindowWidgetContextMenu::OnSetMaximumWindow()
{
  m_LevelWindow.SetToMaxWindowSize();
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLevelWindowWidgetContextMenu::OnSetDefaultScaleRange()
{
  m_LevelWindow.ResetDefaultRangeMinMax();
  m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel(), m_LevelWindow.GetWindow());
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLevelWindowWidgetContextMenu::OnChangeScaleRange()
{
  QmitkLevelWindowRangeChangeDialog changeRange(this);
  changeRange.setLowerLimit((mitk::ScalarType)m_LevelWindow.GetRangeMin());
  changeRange.setUpperLimit((mitk::ScalarType)m_LevelWindow.GetRangeMax());
  if (changeRange.exec())
  {
    m_LevelWindow.SetRangeMinMax(changeRange.getLowerLimit(), changeRange.getUpperLimit());
    m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel(), m_LevelWindow.GetWindow());
    m_Manager->SetLevelWindow(m_LevelWindow);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLevelWindowWidgetContextMenu::OnSetImage(QAction *imageAction)
{
  if (imageAction == m_AutoTopmostAction)
  {
    if (m_Manager->IsAutoTopMost() == false)
    {
      m_Manager->SetAutoTopMostImage(true);
    }
    else
    {
      m_Manager->SetAutoTopMostImage(false);
    }
  }
  else if(imageAction == m_SelectedImagesAction)
  {
    if (m_Manager->IsSelectedImages() == false)
    {
      m_Manager->SetSelectedImages(true);
    }
    else
    {
      m_Manager->SetSelectedImages(false);
    }
  }
  else
  {
    m_Manager->SetLevelWindowProperty(m_Images.at(imageAction));
  }
}

void QmitkLevelWindowWidgetContextMenu::GetContextMenu(QMenu *contextMenu)
{
  if (nullptr == contextMenu)
  {
    return;
  }

  try
  {
    m_LevelWindow = m_Manager->GetLevelWindow();

    QAction *sliderFixed = contextMenu->addAction(tr("Set slider fixed"), this, &QmitkLevelWindowWidgetContextMenu::OnSetFixed);
    sliderFixed->setCheckable(true);
    sliderFixed->setChecked(m_LevelWindow.IsFixed());
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Use whole image grey values"), this, &QmitkLevelWindowWidgetContextMenu::OnUseAllGreyvaluesFromImage);
    contextMenu->addAction(tr("Use optimized level-window"), this, &QmitkLevelWindowWidgetContextMenu::OnUseOptimizedLevelWindow);
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Set maximum window"), this, &QmitkLevelWindowWidgetContextMenu::OnSetMaximumWindow);
    contextMenu->addAction(tr("Default level-window"), this, &QmitkLevelWindowWidgetContextMenu::OnSetDefaultLevelWindow);
    contextMenu->addSeparator();
    contextMenu->addAction(tr("Change scale range"), this, &QmitkLevelWindowWidgetContextMenu::OnChangeScaleRange);
    contextMenu->addAction(tr("Default scale range"), this, &QmitkLevelWindowWidgetContextMenu::OnSetDefaultScaleRange);
    contextMenu->addSeparator();

    m_PresetSubmenu = new QMenu(this);
    m_PresetSubmenu->setTitle("Presets");
    m_PresetAction = m_PresetSubmenu->addAction(tr("Preset definition"), this, &QmitkLevelWindowWidgetContextMenu::OnAddPreset);
    m_PresetSubmenu->addSeparator();
    std::map<std::string, double> preset = m_LevelWindowPreset->getLevelPresets();
    for (auto iter = preset.begin(); iter != preset.end(); iter++)
    {
      QString item = ((*iter).first.c_str());
      m_PresetSubmenu->addAction(item);
    }

    connect(m_PresetSubmenu, &QMenu::triggered, this, &QmitkLevelWindowWidgetContextMenu::OnSetPreset);
    contextMenu->addMenu(m_PresetSubmenu);
    contextMenu->addSeparator();
    m_ImageSubmenu = new QMenu(this);
    m_ImageSubmenu->setTitle("Images");

    // add action for "auto topmost image" action
    m_AutoTopmostAction = m_ImageSubmenu->addAction(tr("Set topmost image"));
    m_AutoTopmostAction->setCheckable(true);
    if (m_Manager->IsAutoTopMost())
    {
      m_AutoTopmostAction->setChecked(true);
    }

    // add action for "selected images" action
    m_ImageSubmenu->addSeparator();
    m_SelectedImagesAction = m_ImageSubmenu->addAction(tr("Use selected images"));
    m_SelectedImagesAction->setCheckable(true);
    if (m_Manager->IsSelectedImages())
    {
      m_SelectedImagesAction->setChecked(true);
    }

    // add action for individual images
    m_ImageSubmenu->addSeparator();

    mitk::DataStorage::SetOfObjects::ConstPointer allObjects = m_Manager->GetRelevantNodes();
    for (mitk::DataStorage::SetOfObjects::ConstIterator objectIter = allObjects->Begin();
      objectIter != allObjects->End();
      ++objectIter)
    {
      mitk::DataNode *node = objectIter->Value();
      if (nullptr == node)
      {
        continue;
      }

      bool isHelperObject = false;
      node->GetBoolProperty("helper object", isHelperObject);

      if (isHelperObject)
      {
        continue;
      }

      if (!node->IsVisible(nullptr))
      {
        continue;
      }

      mitk::LevelWindowProperty::Pointer levelWindowProperty =
        dynamic_cast<mitk::LevelWindowProperty *>(node->GetProperty("levelwindow"));

      if (levelWindowProperty.IsNotNull())
      {
        std::string name;
        node->GetName(name);
        QString item = name.c_str();
        QAction *id = m_ImageSubmenu->addAction(item);
        id->setCheckable(true);
        m_Images[id] = levelWindowProperty;
        if (levelWindowProperty == m_Manager->GetLevelWindowProperty())
        {
          id->setChecked(true);
        }
      }
    }

    connect(m_ImageSubmenu, &QMenu::triggered, this, &QmitkLevelWindowWidgetContextMenu::OnSetImage);

    contextMenu->addMenu(m_ImageSubmenu);
    contextMenu->exec(QCursor::pos());
  }
  catch (...)
  {
  }
}

void QmitkLevelWindowWidgetContextMenu::GetContextMenu()
{
  auto contextMenu = new QMenu(this);
  GetContextMenu(contextMenu);
  delete contextMenu;
}
