/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUndoRedoPreferencePage.h"
#include <ui_QmitkUndoRedoPreferencePage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkUndoController.h>
#include <mitkUndoModel.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    auto* systemPref = preferencesService->GetSystemPreferences();
    return nullptr != systemPref ? systemPref->Node("/General/UndoRedo") : nullptr;
  }
}

QmitkUndoRedoPreferencePage::QmitkUndoRedoPreferencePage()
  : m_Ui(std::make_unique<Ui::QmitkUndoRedoPreferencePage>()),
    m_Control(nullptr)
{
}

QmitkUndoRedoPreferencePage::~QmitkUndoRedoPreferencePage()
{
}

void QmitkUndoRedoPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

// Build the widgets from the .ui file and wire the checkbox so it enables/disables the spin box.
void QmitkUndoRedoPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  connect(m_Ui->limitCheckBox, &QAbstractButton::toggled,
          m_Ui->limitSpinBox, &QWidget::setEnabled);

  this->Update();
}

QWidget* QmitkUndoRedoPreferencePage::GetQtControl() const
{
  return m_Control;
}

void QmitkUndoRedoPreferencePage::PerformCancel()
{
}

// Persist the chosen limit (0 = unlimited) and apply it to the running undo model so the
// change takes effect immediately. The Undo/Redo Inspector picks it up via UndoStackEvent.
bool QmitkUndoRedoPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();
  if (prefs == nullptr)
    return true;

  const unsigned int limit = m_Ui->limitCheckBox->isChecked()
    ? static_cast<unsigned int>(m_Ui->limitSpinBox->value())
    : 0u;

  prefs->PutInt("UndoLimit", limit);

  if (auto* model = mitk::UndoController::GetCurrentUndoModel())
    model->SetUndoLimit(limit);

  return true;
}

// Load the widget state from the saved preference. Default 50 matches the undo controller's
// DEFAULT_UNDO_REDO_LIMIT so a fresh install shows what the model actually has.
void QmitkUndoRedoPreferencePage::Update()
{
  auto* prefs = GetPreferences();
  const int limit = (prefs != nullptr) ? prefs->GetInt("UndoLimit", 50) : 50;

  const bool limited = (limit > 0);
  m_Ui->limitCheckBox->setChecked(limited);
  m_Ui->limitSpinBox->setEnabled(limited);
  m_Ui->limitSpinBox->setValue(limited ? limit : 100);
}
