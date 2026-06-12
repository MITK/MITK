/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUndoRedoPreferencePage.h"
#include <mitkUndoRedoPreferenceHelper.h>
#include <ui_QmitkUndoRedoPreferencePage.h>

#include <mitkLog.h>
#include <mitkUndoController.h>
#include <mitkUndoModel.h>

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
  auto* prefs = mitk::UndoRedoPreferenceHelper::GetPreferences();
  if (prefs == nullptr)
  {
    MITK_WARN << "Could not access undo/redo preferences; the undo limit was not saved.";
    return false;
  }

  const int limit = m_Ui->limitCheckBox->isChecked()
    ? m_Ui->limitSpinBox->value()
    : 0;

  mitk::UndoRedoPreferenceHelper::StoreLimit(limit);

  if (auto* model = mitk::UndoController::GetCurrentUndoModel())
    model->SetUndoLimit(static_cast<std::size_t>(limit));

  return true;
}

// Load the widget state from the saved preference. The default matches the undo controller's
// mitk::DEFAULT_UNDO_REDO_LIMIT so a fresh install shows what the model actually has. When no
// limit is active, the (disabled) spin box pre-fills with the last positive limit the user chose.
void QmitkUndoRedoPreferencePage::Update()
{
  const int activeLimit = mitk::UndoRedoPreferenceHelper::GetActiveLimit();
  const bool limited = (activeLimit > 0);

  m_Ui->limitCheckBox->setChecked(limited);
  m_Ui->limitSpinBox->setEnabled(limited);
  m_Ui->limitSpinBox->setValue(limited ? activeLimit
                                        : mitk::UndoRedoPreferenceHelper::GetLastPositiveLimit());
}
