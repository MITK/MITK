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

class MITKSEGMENTATIONUI_EXPORT QmitkSegmentationTaskListWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkSegmentationTaskListWidget(QWidget* parent = nullptr);
  ~QmitkSegmentationTaskListWidget() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);
  void CheckDataStorage(const mitk::DataNode* removedNode = nullptr);

  bool ActiveTaskIsShown() const;
  void LoadNextUnfinishedTask();
  void SaveActiveTask(bool saveAsIntermediateResult = false);
  bool OnPreShutdown();

signals:
  void ActiveTaskChanged(const std::optional<size_t>& index);
  void CurrentTaskChanged(const std::optional<size_t>& index);

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
  void UpdateNavigationButtons();
  void UpdateDetailsLabel();
  void UpdateStoreAndAcceptButtons();
  void OnLoadButtonClicked();
  mitk::DataNode* GetImageDataNode(size_t index) const;
  void UnloadTasks(const mitk::DataNode* skip = nullptr);
  void LoadTask(mitk::DataNode::Pointer imageNode = nullptr);
  void SubscribeToActiveSegmentation();
  void UnsubscribeFromActiveSegmentation();
  void OnSegmentationModified();
  void SetActiveTaskIndex(const std::optional<size_t>& index);
  void SetCurrentTaskIndex(const std::optional<size_t>& index);
  bool HandleUnsavedChanges(const QString& alternativeTitle = QString());
  mitk::DataNode* GetSegmentationDataNode(size_t index) const;
  void OnUnsavedChangesSaved();
  void OnPreviousTaskShortcutActivated();
  void OnNextTaskShortcutActivated();
  void OnLoadTaskShortcutActivated();
  void OnStoreInterimResultShortcutActivated();
  void OnAcceptSegmentationShortcutActivated();
  void OnStoreButtonClicked();
  void OnAcceptButtonClicked();

  Ui::QmitkSegmentationTaskListWidget* m_Ui;
  QFileSystemWatcher* m_FileSystemWatcher;
  mitk::DataStorage* m_DataStorage;
  mitk::SegmentationTaskList::Pointer m_TaskList;
  mitk::DataNode::Pointer m_TaskListNode;
  std::optional<size_t> m_CurrentTaskIndex;
  std::optional<size_t> m_ActiveTaskIndex;
  std::optional<unsigned long> m_SegmentationModifiedObserverTag;
  bool m_UnsavedChanges;
};

#endif
