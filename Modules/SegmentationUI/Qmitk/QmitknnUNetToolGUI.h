/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#ifndef QmitknnUNetToolGUI_h_Included
#define QmitknnUNetToolGUI_h_Included

#include "QmitkAutoMLSegmentationToolGUIBase.h"
#include "QmitknnUNetGPU.h"
#include "QmitknnUNetWorker.h"
#include "mitknnUnetTool.h"
#include "ui_QmitknnUNetToolGUIControls.h"
#include <MitkSegmentationUIExports.h>
#include <QCache>
#include <QThread>
#include <ctkPathLineEdit.h>

class MITKSEGMENTATIONUI_EXPORT QmitknnUNetToolGUI : public QmitkAutoMLSegmentationToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitknnUNetToolGUI, QmitkAutoMLSegmentationToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots:
  /**
   * @brief Qt slot
   *
   */
  void OnSettingsAccepted();

  /**
   * @brief Qt slot
   *
   */
  void OnDirectoryChanged(const QString &);

  /**
   * @brief Qt slot
   *
   */
  void OnModelChanged(const QString &);

  /**
   * @brief Qt slot
   *
   */
  void OnTaskChanged(const QString &);

  /**
   * @brief Qt slot
   *
   */
  void OnTrainerChanged(const QString &);

  /**
   * @brief Qt slot
   *
   */
  void OnCheckBoxChanged(int);

  /**
   * @brief Qthread slot to capture failures from thread worker and
   * shows error message
   *
   */
  void SegmentationProcessFailed();

  /**
   * @brief Qthread to capture sucessfull nnUNet segmentation.
   * Further, renders the LabelSet image
   */
  void SegmentationResultHandler(mitk::nnUNetTool *);

  /**
   * @brief Qt Slot
   *
   */
  void OnModalitiesNumberChanged(int);

  /**
   * @brief Qt Slot
   *
   */
  void OnPythonPathChanged(const QString &);

signals:
  /**
   * @brief signal for starting the segmentation which is caught by a worker thread.
   */
  void Operate(mitk::nnUNetTool *);

protected:
  QmitknnUNetToolGUI();
  ~QmitknnUNetToolGUI();

  void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool *newTool) override;
  void InitializeUI(QBoxLayout *mainLayout) override;
  void EnableWidgets(bool enabled) override;

private:
  /**
   * @brief Creates a QMessage object and shows on screen.
   */
  void ShowErrorMessage(std::string &);

  /**
   * @brief Searches and parses paths of python virtual enviroments
   * from predefined lookout locations
   */
  void AutoParsePythonPaths();

  /**
   * @brief Clears all combo boxes
   * Any new combo box added in the future can be featured here for clearance.
   *
   */
  void ClearAllComboBoxes();

  /**
   * @brief Checks if nnUNet_predict command is valid in the selected python virtual environment.
   *
   * @return bool
   */
  bool IsNNUNetInstalled(const QString &);

  /**
   * @brief Mapper function to map QString entries from UI to ModelParam attributes.
   *
   * @return mitk::ModelParams
   */
  mitk::ModelParams MapToRequest(
    const QString &, const QString &, const QString &, const QString &, const std::vector<std::string> &);

  /**
   * @brief Returns checked fold names from the ctk-Checkable-ComboBox.
   *
   * @return std::vector<std::string>
   */
  std::vector<std::string> FetchSelectedFoldsFromUI();

  /**
   * @brief Returns all paths from the dynamically generated ctk-path-line-edit boxes.
   *
   * @return std::vector<std::string>
   */
  std::vector<std::string> FetchMultiModalPathsFromUI();

  /**
   * @brief Template function to fetch all folders inside a given path.
   * The type can be any of stl or Qt containers which supports push_back call.
   *
   * @tparam T
   * @return T
   */
  template <typename T>
  static T FetchFoldersFromDir(const QString &);

  /**
   * @brief Stores path of the model director (RESULTS_FOLDER appended by "nnUNet").
   *
   */
  QString m_ModelDirectory;
  Ui_QmitknnUNetToolGUIControls m_Controls;
  QThread *m_SegmentationThread;
  nnUNetSegmentationWorker *m_Worker;
  QmitkGPULoader m_GpuLoader;

  /**
   * @brief Stores all dynamically added ctk-path-line-edit UI elements.
   *
   */
  std::vector<ctkPathLineEdit *> m_ModalPaths;

  std::vector<QSpinBox *> m_ModalOrder;

  /**
   * @brief Stores row count of the "advancedSettingsLayout" layout element. This value helps dynamically add
   * ctk-path-line-edit UI elements at the right place. Forced to initialize in the InitializeUI method since there is
   * no guarantee of retrieving exact row count anywhere else.
   *
   */
  int m_UI_ROWS;
};

#endif
