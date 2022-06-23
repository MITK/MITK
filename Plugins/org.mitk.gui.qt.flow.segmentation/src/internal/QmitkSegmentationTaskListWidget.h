/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentationTaskListWidget_h
#define QmitkSegmentationTaskListWidget_h

#include <mitkSegmentationTaskList.h>

#include <MitkSegmentationUIExports.h>

#include <QmitkSingleNodeSelectionWidget.h>

#include <QWidget>

#include <optional>

class QFileSystemWatcher;

namespace Ui
{
  class QmitkSegmentationTaskListWidget;
}

class QmitkSegmentationTaskListWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkSegmentationTaskListWidget(QWidget* parent = nullptr);
  ~QmitkSegmentationTaskListWidget() override;

  mitk::SegmentationTaskList* GetTaskList() const;
  std::optional<size_t> GetActiveTaskIndex() const;
  size_t GetCurrentTaskIndex() const;
  mitk::DataNode* GetSegmentationDataNode(size_t index) const;
  void OnUnsavedChangesSaved();

signals:
  void ActiveTaskChanged(const std::optional<size_t>& index);
  void CurrentTaskChanged(size_t index);

private:
  void OnSelectionChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes);
  void ResetControls();
  void SetTaskList(mitk::SegmentationTaskList* task);
  void ResetFileSystemWatcher();
  void OnResultDirectoryChanged(const QString&);
  void UpdateProgressBar();
  void OnTaskListChanged(mitk::SegmentationTaskList* task);
  void OnPreviousButtonClicked();
  void OnNextButtonClicked();
  void OnCurrentTaskChanged();
  void UpdateLoadButton();
  void UpdateDetailsLabel();
  void OnLoadButtonClicked();
  mitk::DataNode* GetImageDataNode(size_t index) const;
  void UnloadTasks(const mitk::DataNode* skip = nullptr);
  void LoadTask(mitk::DataNode::Pointer imageNode = nullptr);
  void SubscribeToActiveSegmentation();
  void UnsubscribeFromActiveSegmentation();
  void OnSegmentationModified();
  void SetActiveTaskIndex(const std::optional<size_t>& index);
  void SetCurrentTaskIndex(size_t index);
  bool ActiveTaskIsShown() const;
  bool HandleUnsavedChanges();
  void SaveActiveTask();

  Ui::QmitkSegmentationTaskListWidget* m_Ui;
  QFileSystemWatcher* m_FileSystemWatcher;
  mitk::SegmentationTaskList::Pointer m_TaskList;
  mitk::DataNode::Pointer m_TaskListNode;
  size_t m_CurrentTaskIndex;
  std::optional<size_t> m_ActiveTaskIndex;
  std::optional<unsigned long> m_SegmentationModifiedObserverTag;
  bool m_UnsavedChanges;
};

#endif
