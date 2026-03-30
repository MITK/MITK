/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFindSegmentationTaskDialog_h
#define QmitkFindSegmentationTaskDialog_h

#include <QDialog>
#include <mitkSegmentationTaskList.h>
#include <memory>

class QTableWidgetItem;

namespace Ui
{
  class QmitkFindSegmentationTaskDialog;
}

/**
 * \brief Dialog for finding and selecting a segmentation task from a task list.
 *
 * Displays a table of tasks with columns for number, name, status, image,
 * segmentation, result, and description. Supports filtering and selection.
 */
class QmitkFindSegmentationTaskDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Construct the find task dialog.
   * \param parent Optional parent widget.
   */
  explicit QmitkFindSegmentationTaskDialog(QWidget* parent = nullptr);
  ~QmitkFindSegmentationTaskDialog() override;

  /**
   * \brief Set the task list to display in the dialog.
   * \param taskList The segmentation task list.
   */
  void SetTaskList(const mitk::SegmentationTaskList* taskList);

  /** \brief Return the index of the selected task, or std::nullopt if none is selected. */
  std::optional<size_t> GetSelectedTask() const;

  /** \brief Return true if the selected task should be loaded immediately. */
  bool LoadSelectedTask() const;

private:
  enum Column
  {
    Number,
    Name,
    Status,
    Image,
    Segmentation,
    Result,
    Description
  };

  void OnItemSelectionChanged();
  void OnItemDoubleClicked(QTableWidgetItem* item);
  void OnFinished(int result);

  void ApplyFilter();
  bool ContainsNumber(int row) const;
  bool ContainsName(int row) const;
  bool HasStatus(int row) const;

  std::unique_ptr<Ui::QmitkFindSegmentationTaskDialog> m_Ui;
  mitk::SegmentationTaskList::ConstPointer m_TaskList;
  std::optional<size_t> m_SelectedTask;
};

#endif
