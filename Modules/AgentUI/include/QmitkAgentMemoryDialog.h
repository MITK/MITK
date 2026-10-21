/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentMemoryDialog_h
#define QmitkAgentMemoryDialog_h

#include <MitkAgentUIExports.h>

#include <QDialog>

#include <memory>

class QmitkAgentMemory;
class QListWidgetItem;
class QPushButton;

namespace Ui
{
  class QmitkAgentMemoryDialog;
}

/**
 * \brief Shows the notes of the agent's memory, which the user can edit and delete.
 *
 * Changes are stored at once. The memory rejects a note that is empty or too
 * long, which the dialog reports and then shows the stored text again.
 */
class MITKAGENTUI_EXPORT QmitkAgentMemoryDialog : public QDialog
{
public:
  /** \param fontSize In points, like the text of the conversation; 0 for the default font. */
  QmitkAgentMemoryDialog(QmitkAgentMemory& memory, qreal fontSize, QWidget* parent);
  ~QmitkAgentMemoryDialog() override;

  /** \brief Whether the user edited or deleted a note. */
  bool HasChanged() const;

private:
  void OnItemChanged(QListWidgetItem* item);
  void DeleteSelected();

  std::unique_ptr<Ui::QmitkAgentMemoryDialog> m_Ui;
  QmitkAgentMemory* m_Memory;
  QPushButton* m_DeleteButton;
  bool m_Changed;
};

#endif
