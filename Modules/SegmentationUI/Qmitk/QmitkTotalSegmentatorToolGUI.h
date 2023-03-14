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

class QmitkTotalSegmentatorToolInstaller : public QmitkSetupVirtualEnvUtil
{
public:
  const QString m_VENV_NAME = ".totalsegmentator";
  const QString m_TOTALSEGMENTATOR_VERSION = "1.5.3";
  const std::vector<QString> m_PACKAGES = {QString("Totalsegmentator==") + m_TOTALSEGMENTATOR_VERSION,
                                           QString("scipy==1.9.1")};
  const QString m_STORAGE_DIR;
  inline QmitkTotalSegmentatorToolInstaller(
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() 
                            + qApp->organizationName() + QDir::separator())
    : QmitkSetupVirtualEnvUtil(baseDir), m_STORAGE_DIR(baseDir){};
  bool SetupVirtualEnv(const QString &) override;
  QString GetVirtualEnvPath() override;
};

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

  void EnableWidgets(bool enabled) override;

  /**
   * @brief Enable (or Disable) GUI elements.
   */
  void EnableAll(bool);

  /**
   * @brief Searches and parses paths of python virtual enviroments
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
   * In case, there aren't any GPUs avaialble, the combo box will be
   * rendered editable.
   */
  void SetGPUInfo();

  /**
   * @brief Returns GPU id of the selected GPU from the Combo box.
   *
   * @return unsigned int
   */
  unsigned int FetchSelectedGPUFromUI();

  bool SetUpTotalSegmentator(const QString &);
  bool SetUpTotalSegmentatorWIN(const QString &);

  QString GetPythonPathFromUI(const QString &);

  QString GetExactPythonPath(const QString &);

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

  const std::string m_WARNING_TOTALSEG_NOT_FOUND =
    "TotalSegmentator is not detected in the selected python environment.Please select a valid "
    "python environment or install TotalSegmentator.";
  const QStringList m_VALID_TASKS = {"total", "cerebral_bleed", "hip_implant", "coronary_arteries"};
  QmitkTotalSegmentatorToolInstaller m_Installer;
};
#endif
