/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUndoRedoPreferencePage_h
#define QmitkUndoRedoPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <memory>

namespace Ui
{
  class QmitkUndoRedoPreferencePage;
}

/**
 * \class QmitkUndoRedoPreferencePage
 * \brief Preferences page for the undo/redo stack size limit.
 *
 * Exposes the same setting as the limit controls in QmitkUndoRedoView via a
 * "Limit undo stack size" checkbox and a spin box for the maximum number of
 * undo steps. Reads and writes the preference at /General/UndoRedo (key
 * "UndoLimit", 0 = unlimited). On Apply it also calls SetUndoLimit on the
 * global undo model so the change takes effect immediately; an open Undo/Redo
 * Inspector view refreshes itself via the model's UndoStackEvent.
 *
 * \sa QmitkUndoRedoView
 * \sa mitk::UndoController
 */
class QmitkUndoRedoPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkUndoRedoPreferencePage();
  ~QmitkUndoRedoPreferencePage() override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  void Init(berry::IWorkbench::Pointer) override;
  void PerformCancel() override;
  bool PerformOk() override;
  void Update() override;

private:
  std::unique_ptr<Ui::QmitkUndoRedoPreferencePage> m_Ui;
  QWidget* m_Control;
};

#endif
