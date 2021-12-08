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
#include "QmitknnUNetFolderParser.h"
#include "QmitknnUNetGPU.h"
#include "mitknnUnetTool.h"
#include "ui_QmitknnUNetToolGUIControls.h"
#include <MitkSegmentationUIExports.h>
#include <QMessageBox>
#include <QmitkDataStorageComboBox.h>
#include <QmitknnUNetEnsembleLayout.h>

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
  void OnPreviewRequested();

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

  /**
   * @brief Qt Slot
   *
   */
  void OnModalPositionChanged(int);

  /**
   * @brief Qt slot
   *
   */
  void OnRefreshPresssed();

protected:
  QmitknnUNetToolGUI();
  ~QmitknnUNetToolGUI() = default;

  void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool *newTool) override;
  void InitializeUI(QBoxLayout *mainLayout) override;
  void EnableWidgets(bool enabled) override;

private:
  /**
   * @brief Parses the ensemble UI elements and sets to nnUNetTool object pointer.
   *
   */
  void ProcessEnsembleModelsParams(mitk::nnUNetTool::Pointer);

  /**
   * @brief Parses the UI elements and sets to nnUNetTool object pointer.
   *
   */
  void ProcessModelParams(mitk::nnUNetTool::Pointer);

  /**
   * @brief Creates and renders QmitknnUNetTaskParamsUITemplate layout for ensemble input.
   */
  void ShowEnsembleLayout(bool visible = true);

  /**
   * @brief Creates a QMessage object and shows on screen.
   */
  void ShowErrorMessage(const std::string &, QMessageBox::Icon = QMessageBox::Critical);

  /**
   * @brief Searches and parses paths of python virtual enviroments
   * from predefined lookout locations
   */
  void AutoParsePythonPaths();

  /**
   * @brief Check if pretrained model sub folder inside RESULTS FOLDER exist.
   * 
   */
  bool IsModelExists(const QString&, const QString&, const QString&);

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
  std::vector<mitk::Image::ConstPointer> FetchMultiModalImagesFromUI();

  Ui_QmitknnUNetToolGUIControls m_Controls;
  QmitkGPULoader m_GpuLoader;

  /**
   * @brief Stores all dynamically added ctk-path-line-edit UI elements.
   *
   */
  std::vector<QmitkDataStorageComboBox *> m_Modalities;

  std::vector<std::unique_ptr<QmitknnUNetTaskParamsUITemplate>> m_EnsembleParams;

  mitk::NodePredicateBase::Pointer m_MultiModalPredicate;

  /**
   * @brief Stores row count of the "advancedSettingsLayout" layout element. This value helps dynamically add
   * ctk-path-line-edit UI elements at the right place. Forced to initialize in the InitializeUI method since there is
   * no guarantee of retrieving exact row count anywhere else.
   *
   */
  int m_UI_ROWS;

  /**
   * @brief Stores path of the model director (RESULTS_FOLDER appended by "nnUNet").
   *
   */
  std::shared_ptr<QmitknnUNetFolderParser> m_ParentFolder = nullptr;

  /**
   * @brief Valid list of models supported by nnUNet
   *
   */
  const QStringList m_VALID_MODELS = {"2d", "3d_lowres", "3d_fullres", "3d_cascade_fullres", "ensembles"};
};
#endif
