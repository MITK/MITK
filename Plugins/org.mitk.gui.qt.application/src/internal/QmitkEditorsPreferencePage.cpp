/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkEditorsPreferencePage.h"
#include <ui_QmitkEditorsPreferencePage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <array>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.editors");
  }

  const std::string MENU_SIZE_PREFERENCE = "render window menu size";
  const std::string SUBDUE_MENUS_PREFERENCE = "subdue render window menus";
  const std::string ANIMATION_FRAME_RATE_PREFERENCE = "animation frame rate";

  constexpr std::array<int, 4> ANIMATION_FRAME_RATES = { 12, 15, 30, 60 };
  constexpr int DEFAULT_ANIMATION_FRAME_RATE = 60;
}

QmitkEditorsPreferencePage::QmitkEditorsPreferencePage()
  : m_Control(nullptr),
    m_Ui(std::make_unique<Ui::QmitkEditorsPreferencePage>())
{
}

QmitkEditorsPreferencePage::~QmitkEditorsPreferencePage() = default;

void QmitkEditorsPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkEditorsPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  for (const auto framesPerSecond : ANIMATION_FRAME_RATES)
    m_Ui->m_AnimationFrameRateComboBox->addItem(QString("%1 fps").arg(framesPerSecond), framesPerSecond);

  this->Update();
}

QWidget* QmitkEditorsPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkEditorsPreferencePage::PerformOk()
{
  auto prefs = GetPreferences();

  const bool constrainZoomingAndPanning = m_Ui->m_ConstrainZoomingAndPanningCheckBox->isChecked();
  prefs->PutBool("Use constrained zooming and panning", constrainZoomingAndPanning);

  prefs->PutInt("max TS", m_Ui->m_MaxTSSpinBox->value());
  prefs->PutInt(ANIMATION_FRAME_RATE_PREFERENCE, m_Ui->m_AnimationFrameRateComboBox->currentData().toInt());

  std::string menuSize = "default";

  if (m_Ui->m_SmallerMenusRadioButton->isChecked())
  {
    menuSize = "smaller";
  }
  else if (m_Ui->m_LargerMenusRadioButton->isChecked())
  {
    menuSize = "larger";
  }

  prefs->Put(MENU_SIZE_PREFERENCE, menuSize);
  prefs->PutBool(SUBDUE_MENUS_PREFERENCE, m_Ui->m_SubdueMenusCheckBox->isChecked());

  return true;
}

void QmitkEditorsPreferencePage::PerformCancel()
{
}

void QmitkEditorsPreferencePage::Update()
{
  auto prefs = GetPreferences();

  const bool constrainZoomingAndPanning = prefs->GetBool("Use constrained zooming and panning", true);
  m_Ui->m_ConstrainZoomingAndPanningCheckBox->setChecked(constrainZoomingAndPanning);

  const auto maxTS = prefs->GetInt("max TS", 50);
  m_Ui->m_MaxTSSpinBox->setValue(maxTS);

  // Only a hand-edited preference holds a rate not offered here.
  auto frameRateIndex = m_Ui->m_AnimationFrameRateComboBox->findData(prefs->GetInt(ANIMATION_FRAME_RATE_PREFERENCE, DEFAULT_ANIMATION_FRAME_RATE));

  if (frameRateIndex < 0)
    frameRateIndex = m_Ui->m_AnimationFrameRateComboBox->findData(DEFAULT_ANIMATION_FRAME_RATE);

  m_Ui->m_AnimationFrameRateComboBox->setCurrentIndex(frameRateIndex);

  // Anything unknown falls back to the default size, as the menus do.
  const auto menuSize = prefs->Get(MENU_SIZE_PREFERENCE, "default");

  if ("smaller" == menuSize)
  {
    m_Ui->m_SmallerMenusRadioButton->setChecked(true);
  }
  else if ("larger" == menuSize)
  {
    m_Ui->m_LargerMenusRadioButton->setChecked(true);
  }
  else
  {
    m_Ui->m_DefaultMenusRadioButton->setChecked(true);
  }

  m_Ui->m_SubdueMenusCheckBox->setChecked(prefs->GetBool(SUBDUE_MENUS_PREFERENCE, true));
}
