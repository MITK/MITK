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

#include "QmitkMxNMultiWidgetEditorPreferencePage.h"
#include <QmitkMxNMultiWidgetEditor.h>

// berry framework
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

QmitkMxNMultiWidgetEditorPreferencePage::QmitkMxNMultiWidgetEditorPreferencePage()
  : m_Preferences(nullptr)
{
  // nothing here
}

QmitkMxNMultiWidgetEditorPreferencePage::~QmitkMxNMultiWidgetEditorPreferencePage()
{
  //nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::Init(berry::IWorkbench::Pointer)
{
  // nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);

  m_Ui.setupUi(m_MainControl);

  berry::IPreferencesService* preferenceService = berry::Platform::GetPreferencesService();
  Q_ASSERT(preferenceService);
  m_Preferences = preferenceService->GetSystemPreferences()->Node(QmitkMxNMultiWidgetEditor::EDITOR_ID);

  connect(m_Ui.m_RenderingModeComboBox, SIGNAL(activated(int)), SLOT(ChangeRenderingMode(int)));
  connect(m_Ui.m_ColormapComboBox, SIGNAL(activated(int)), SLOT(ChangeColormap(int)));
  connect(m_Ui.m_ResetButton, SIGNAL(clicked()), SLOT(ResetPreferencesAndGUI()));

  Update();
}

QWidget* QmitkMxNMultiWidgetEditorPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkMxNMultiWidgetEditorPreferencePage::PerformOk()
{
  m_Preferences->PutBool("Use constrained zooming and panning", m_Ui.m_EnableFlexibleZooming->isChecked());
  m_Preferences->PutBool("Show level/window widget", m_Ui.m_ShowLevelWindowWidget->isChecked());
  m_Preferences->PutBool("PACS like mouse interaction", m_Ui.m_PACSLikeMouseMode->isChecked());

  m_Preferences->PutInt("Rendering Mode", m_Ui.m_RenderingModeComboBox->currentIndex());

  m_Preferences->PutInt("Render window widget colormap", m_Ui.m_ColormapComboBox->currentIndex());
  m_Preferences->PutBool("Render window individual decorations", m_Ui.m_IndividualDecorations->isChecked());

  m_Preferences->PutInt("crosshair gap size", m_Ui.m_CrosshairGapSize->value());

  return true;
}

void QmitkMxNMultiWidgetEditorPreferencePage::PerformCancel()
{
  // nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::Update()
{
  m_Ui.m_EnableFlexibleZooming->setChecked(m_Preferences->GetBool("Use constrained zooming and panning", true));
  m_Ui.m_ShowLevelWindowWidget->setChecked(m_Preferences->GetBool("Show level/window widget", true));
  m_Ui.m_PACSLikeMouseMode->setChecked(m_Preferences->GetBool("PACS like mouse interaction", false));

  int renderingMode = m_Preferences->GetInt("Rendering Mode", 0);
  m_Ui.m_RenderingModeComboBox->setCurrentIndex(renderingMode);

  int colormap = m_Preferences->GetInt("Render window widget colormap", 0);
  m_Ui.m_ColormapComboBox->setCurrentIndex(colormap);

  m_Ui.m_IndividualDecorations->setChecked(m_Preferences->GetBool("Render window individual decorations", false));

  m_Ui.m_CrosshairGapSize->setValue(m_Preferences->GetInt("crosshair gap size", 32));
}

void QmitkMxNMultiWidgetEditorPreferencePage::ResetPreferencesAndGUI()
{
  m_Preferences->Clear();
  Update();
}

void QmitkMxNMultiWidgetEditorPreferencePage::ChangeRenderingMode(int i)
{
  if (0 == i)
  {
    m_CurrentRenderingMode = "Standard";
  }
  else if (1 == i)
  {
    m_CurrentRenderingMode = "Multisampling";
  }
  else if (2 == i)
  {
    m_CurrentRenderingMode = "DepthPeeling";
  }
}

void QmitkMxNMultiWidgetEditorPreferencePage::ChangeColormap(int i)
{
  if (0 == i)
  {
    m_CurrentColormap = "Black and white";
  }
}