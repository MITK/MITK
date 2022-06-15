/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentationTaskWidget_h
#define QmitkSegmentationTaskWidget_h

#include <mitkSegmentationTask.h>

#include <MitkSegmentationUIExports.h>

#include <QmitkSingleNodeSelectionWidget.h>

#include <QWidget>

#include <optional>

class QFileSystemWatcher;

namespace Ui
{
  class QmitkSegmentationTaskWidget;
}

class QmitkSegmentationTaskWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkSegmentationTaskWidget(QWidget* parent = nullptr);
  ~QmitkSegmentationTaskWidget() override;

  mitk::SegmentationTask* GetTask() const;
  std::optional<size_t> GetActiveSubtaskIndex() const;
  size_t GetCurrentSubtaskIndex() const;
  mitk::DataNode* GetSegmentationDataNode(size_t index) const;

signals:
  void ActiveSubtaskChanged(const std::optional<size_t>& index);
  void CurrentSubtaskChanged(size_t index);

private:
  void OnSelectionChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes);
  void ResetControls();
  void SetTask(mitk::SegmentationTask* task);
  void ResetFileSystemWatcher();
  void OnResultDirectoryChanged(const QString&);
  void UpdateProgressBar();
  void OnTaskChanged(mitk::SegmentationTask* task);
  void OnPreviousButtonClicked();
  void OnNextButtonClicked();
  void OnSubtaskChanged();
  void UpdateLoadButton();
  void UpdateDetailsLabel();
  void OnLoadButtonClicked();
  mitk::DataNode* GetImageDataNode(size_t index) const;
  void UnloadSubtasks(const mitk::DataNode* skip = nullptr);
  void LoadSubtask(mitk::DataNode::Pointer imageNode = nullptr);
  void SetActiveSubtaskIndex(const std::optional<size_t>& index);
  void SetCurrentSubtaskIndex(size_t index);

  Ui::QmitkSegmentationTaskWidget* m_Ui;
  QFileSystemWatcher* m_FileSystemWatcher;
  mitk::SegmentationTask::Pointer m_Task;
  size_t m_CurrentSubtaskIndex;
  std::optional<size_t> m_ActiveSubtaskIndex;
};

#endif
