/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTotalSegmentatorToolGUI_h_Included
#define QmitkTotalSegmentatorToolGUI_h_Included

#include "QmitkMultiLabelSegWithPreviewToolGUIBase.h"
#include "QmitkSetupVirtualEnvUtil.h"
#include "QmitknnUNetGPU.h"
#include "ui_QmitkTotalSegmentatorGUIControls.h"
#include <MitkSegmentationUIExports.h>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

/**
 * @brief Installer class for TotalSegmentator Tool.
 * Class specifies the virtual environment name, install version, packages required to pip install
 * and implements SetupVirtualEnv method.
 * 
 */
class QmitkTotalSegmentatorToolInstaller : public QmitkSetupVirtualEnvUtil
{
public:
  const QString VENV_NAME = ".totalsegmentator_v2";
  const QString TOTALSEGMENTATOR_VERSION = "2.2.1";
  const std::vector<QString> PACKAGES = {QString("Totalsegmentator==") + TOTALSEGMENTATOR_VERSION,
                                         QString("numpy<2"),
                                         QString("SimpleITK<=2.3.1"),
                                         QString("p-tqdm<=1.4.0"),
                                         QString("xvfbwrapper<=0.2.9"),
                                         QString("rt_utils<=1.2.7"),
                                         QString("dicom2nifti<=2.4.11"),
                                         QString("pyarrow<=16.1.0"),
                                         QString("setuptools")}; /* just in case */
  const QString STORAGE_DIR;
  inline QmitkTotalSegmentatorToolInstaller(
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() 
                            + qApp->organizationName() + QDir::separator())
    : QmitkSetupVirtualEnvUtil(baseDir), STORAGE_DIR(baseDir){};
  bool SetupVirtualEnv(const QString &) override;
  QString GetVirtualEnvPath() override;
};

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::TotalSegmentatorTool.
  \sa mitk::
*/
class MITKSEGMENTATIONUI_EXPORT QmitkTotalSegmentatorToolGUI : public QmitkMultiLabelSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkTotalSegmentatorToolGUI, QmitkMultiLabelSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots:

  /**
   * @brief Qt Slot
   */
  void OnPreviewBtnClicked();

  /**
   * @brief Qt Slot
   */
  void OnPythonPathChanged(const QString &);

  /**
   * @brief Qt Slot
   */
  std::pair<QString, QString> OnSystemPythonChanged(const QString &);

  /**
   * @brief Qt Slot
   */
  void OnInstallBtnClicked();

  /**
   * @brief Qt Slot
   */
  void OnOverrideChecked(int);

  /**
   * @brief Qt Slot
   */
  void OnClearInstall();

protected:
  QmitkTotalSegmentatorToolGUI();
  ~QmitkTotalSegmentatorToolGUI() = default;

  void ConnectNewTool(mitk::SegWithPreviewTool *newTool) override;
  void InitializeUI(QBoxLayout *mainLayout) override;

  /**
   * @brief Enable (or Disable) GUI elements.
   */
  void EnableAll(bool);

  /**
   * @brief Searches and parses paths of python virtual environments
   * from predefined lookout locations
   */
  void AutoParsePythonPaths();

  /**
   * @brief Checks if TotalSegmentator command is valid in the selected python virtual environment.
   *
   * @return bool
   */
  bool IsTotalSegmentatorInstalled(const QString &);

  /**
   * @brief Creates a QMessage object and shows on screen.
   */
  void ShowErrorMessage(const std::string &, QMessageBox::Icon = QMessageBox::Critical);

  /**
   * @brief Writes any message in white on the tool pane.
   */
  void WriteStatusMessage(const QString &);

  /**
   * @brief Writes any message in red on the tool pane.
   */
  void WriteErrorMessage(const QString &);

  /**
   * @brief Adds GPU information to the gpu combo box.
   * In case, there aren't any GPUs available, the combo box will be
   * rendered editable.
   */
  void SetGPUInfo();

  /**
   * @brief Returns GPU id of the selected GPU from the Combo box.
   *
   * @return unsigned int
   */
  unsigned int FetchSelectedGPUFromUI() const;

  /**
   * @brief Get the virtual env path from UI combobox removing any 
   * extra special characters.
   * 
   * @return QString 
   */
  QString GetPythonPathFromUI(const QString &) const;

  /**
   * @brief For storing values like Python path across sessions.
   */
  QSettings m_Settings;

  QString m_PythonPath;
  QmitkGPULoader m_GpuLoader;
  Ui_QmitkTotalSegmentatorToolGUIControls m_Controls;
  bool m_FirstPreviewComputation = true;
  bool m_IsInstalled = false;
  EnableConfirmSegBtnFunctionType m_SuperclassEnableConfirmSegBtnFnc;

  const std::string WARNING_TOTALSEG_NOT_FOUND =
    "TotalSegmentator is not detected in the selected python environment.Please select a valid "
    "python environment or install TotalSegmentator.";
  const QStringList VALID_TASKS = {
    "total",
    "total_mr",
    "cerebral_bleed",
    "hip_implant",
    "coronary_arteries",
    "body",
    "lung_vessels",
    "pleural_pericard_effusion"
  };
  QmitkTotalSegmentatorToolInstaller m_Installer;
};
#endif
