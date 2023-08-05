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

class QTableWidgetItem;

namespace Ui
{
  class QmitkFindSegmentationTaskDialog;
}

class QmitkFindSegmentationTaskDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkFindSegmentationTaskDialog(QWidget* parent = nullptr);
  ~QmitkFindSegmentationTaskDialog() override;

  void SetTaskList(const mitk::SegmentationTaskList* taskList);
  std::optional<size_t> GetSelectedTask() const;
  bool LoadSelectedTask() const;

private:
  void OnItemSelectionChanged();
  void OnItemDoubleClicked(QTableWidgetItem* item);

  Ui::QmitkFindSegmentationTaskDialog* m_Ui;
  mitk::SegmentationTaskList::ConstPointer m_TaskList;
  std::optional<size_t> m_SelectedTask;
};

#endif
