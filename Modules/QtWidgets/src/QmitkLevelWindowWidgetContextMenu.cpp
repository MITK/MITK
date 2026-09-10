/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkLevelWindowWidgetContextMenu.h>

// mitk core
#include <mitkRenderingManager.h>

// mitk qt widgets
#include <QmitkLevelWindowPresetDefinitionDialog.h>
#include <QmitkLevelWindowRangeChangeDialog.h>

// qt
#include <QActionGroup>
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
    m_Manager->SetMode(mitk::LevelWindowManager::Mode::TopMostImage);
  }
  else if (imageAction == m_SelectedImagesAction)
  {
    m_Manager->SetMode(mitk::LevelWindowManager::Mode::SelectedImage);
  }
  else if (auto image = m_Images.find(imageAction); image != m_Images.end())
  {
    m_Manager->SetLevelWindowProperty(image->second);
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

    m_PresetSubmenu = new QMenu(contextMenu);
    m_PresetSubmenu->setTitle(tr("Presets"));
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
    m_ImageSubmenu = new QMenu(contextMenu);
    m_ImageSubmenu->setTitle(tr("Images"));

    const auto mode = m_Manager->GetMode();
    auto *imageGroup = new QActionGroup(m_ImageSubmenu);

    m_AutoTopmostAction = m_ImageSubmenu->addAction(tr("Topmost visible image"));
    m_AutoTopmostAction->setCheckable(true);
    m_AutoTopmostAction->setChecked(mitk::LevelWindowManager::Mode::TopMostImage == mode);
    imageGroup->addAction(m_AutoTopmostAction);

    m_SelectedImagesAction = m_ImageSubmenu->addAction(tr("Selected image"));
    m_SelectedImagesAction->setCheckable(true);
    m_SelectedImagesAction->setChecked(mitk::LevelWindowManager::Mode::SelectedImage == mode);
    imageGroup->addAction(m_SelectedImagesAction);

    m_ImageSubmenu->addSeparator();

    m_Images.clear();
    const auto *currentNode = m_Manager->GetCurrentNode();

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
        QAction *imageAction = m_ImageSubmenu->addAction(QString::fromStdString(node->GetName()));
        imageAction->setCheckable(true);
        imageAction->setChecked(mitk::LevelWindowManager::Mode::ExplicitImage == mode && node == currentNode);
        imageGroup->addAction(imageAction);
        m_Images[imageAction] = levelWindowProperty;
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
