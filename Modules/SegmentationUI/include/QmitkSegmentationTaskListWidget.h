/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentationTaskListWidget_h
#define QmitkSegmentationTaskListWidget_h

#include <mitkForm.h>
#include <mitkSegmentationTaskList.h>

#include <MitkSegmentationUIExports.h>

#include <QmitkSingleNodeSelectionWidget.h>

#include <QWidget>

#include <optional>
#include <memory>

class QFileSystemWatcher;

namespace Ui
{
  class QmitkSegmentationTaskListWidget;
}

/**
 * \brief Widget for managing segmentation task lists.
 *
 * Provides navigation through a list of segmentation tasks, loading/unloading of
 * task images and segmentations, saving results, and tracking task progress.
 *
 * \sa mitk::SegmentationTaskList
 */
class MITKSEGMENTATIONUI_EXPORT QmitkSegmentationTaskListWidget : public QWidget
{
  Q_OBJECT

public:
  /** \brief Constructs the segmentation task list widget. */
  explicit QmitkSegmentationTaskListWidget(QWidget* parent = nullptr);
  ~QmitkSegmentationTaskListWidget() override;

  /** \brief Sets the data storage used for loading task data nodes. */
  void SetDataStorage(mitk::DataStorage* dataStorage);
  /** \brief Checks data storage consistency, optionally after a node removal. */
  void CheckDataStorage(const mitk::DataNode* removedNode = nullptr);

  /** \brief Returns true if the currently active task is shown in the widget. */
  bool ActiveTaskIsShown() const;
  /** \brief Loads the next unfinished task from the task list. */
  void LoadNextUnfinishedTask();
  /** \brief Saves the currently active task, optionally as an interim result. */
  void SaveActiveTask(bool saveAsInterimResult = false);
  /** \brief Called during application shutdown; prompts for unsaved changes if needed. Returns false to cancel shutdown. */
  bool OnPreShutdown();

signals:
  /** \brief Emitted when the active task changes. The index is std::nullopt when no task is active. */
  void ActiveTaskChanged(const std::optional<size_t>& index);
  /** \brief Emitted when the current (browsed) task changes. The index is std::nullopt when no task is current. */
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
  void OnFindButtonClicked();
  void OnCurrentTaskChanged();
  void UpdateLoadButton();
  void UpdateNavigationButtons();
  void UpdateDetailsLabel();
  void UpdateFormWidget();
  void OnFormSubmission();
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
  void OnUnsavedChangesSaved();
  void OnPreviousTaskShortcutActivated();
  void OnNextTaskShortcutActivated();
  void OnFindTaskShortcutActivated();
  void OnLoadTaskShortcutActivated();
  void OnStoreInterimResultShortcutActivated();
  void OnAcceptSegmentationShortcutActivated();
  void OnStoreButtonClicked();
  void OnAcceptButtonClicked();

  std::unique_ptr<Ui::QmitkSegmentationTaskListWidget> m_Ui;
  QFileSystemWatcher* m_FileSystemWatcher;
  mitk::DataStorage* m_DataStorage;
  mitk::SegmentationTaskList::Pointer m_TaskList;
  mitk::Forms::Form m_Form;
  mitk::DataNode::Pointer m_TaskListNode;
  mitk::DataNode* m_ImageNode;
  mitk::DataNode* m_SegmentationNode;
  std::optional<size_t> m_CurrentTaskIndex;
  std::optional<size_t> m_ActiveTaskIndex;
  std::optional<unsigned long> m_SegmentationModifiedObserverTag;
  bool m_UnsavedChanges;
};

#endif
