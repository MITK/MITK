/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNMultiWidgetEditorPreferencePage.h"
#include <QmitkMxNMultiWidgetEditor.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(QmitkMxNMultiWidgetEditor::EDITOR_ID.toStdString());
  }
}

QmitkMxNMultiWidgetEditorPreferencePage::QmitkMxNMultiWidgetEditorPreferencePage()
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
  auto* prefs = GetPreferences();

  prefs->PutBool("Use constrained zooming and panning", m_Ui.m_EnableFlexibleZooming->isChecked());
  prefs->PutBool("Show level/window widget", m_Ui.m_ShowLevelWindowWidget->isChecked());
  prefs->PutBool("PACS like mouse interaction", m_Ui.m_PACSLikeMouseMode->isChecked());
  prefs->PutInt("Render window widget colormap", m_Ui.m_ColormapComboBox->currentIndex());
  prefs->PutBool("Render window individual decorations", m_Ui.m_IndividualDecorations->isChecked());

  prefs->PutInt("crosshair gap size", m_Ui.m_CrosshairGapSize->value());

  return true;
}

void QmitkMxNMultiWidgetEditorPreferencePage::PerformCancel()
{
  // nothing here
}

void QmitkMxNMultiWidgetEditorPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  m_Ui.m_EnableFlexibleZooming->setChecked(prefs->GetBool("Use constrained zooming and panning", true));
  m_Ui.m_ShowLevelWindowWidget->setChecked(prefs->GetBool("Show level/window widget", true));
  m_Ui.m_PACSLikeMouseMode->setChecked(prefs->GetBool("PACS like mouse interaction", false));

  int colormap = prefs->GetInt("Render window widget colormap", 0);
  m_Ui.m_ColormapComboBox->setCurrentIndex(colormap);

  m_Ui.m_IndividualDecorations->setChecked(prefs->GetBool("Render window individual decorations", false));

  m_Ui.m_CrosshairGapSize->setValue(prefs->GetInt("crosshair gap size", 32));
}

void QmitkMxNMultiWidgetEditorPreferencePage::ResetPreferencesAndGUI()
{
  auto* prefs = GetPreferences();

  prefs->Clear();
  Update();
}

void QmitkMxNMultiWidgetEditorPreferencePage::ChangeColormap(int i)
{
  if (0 == i)
  {
    m_CurrentColormap = "Black and white";
  }
}
