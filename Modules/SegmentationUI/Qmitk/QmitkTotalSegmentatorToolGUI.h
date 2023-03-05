#ifndef QmitkTotalSegmentatorToolGUI_h_Included
#define QmitkTotalSegmentatorToolGUI_h_Included

#include "QmitkMultiLabelSegWithPreviewToolGUIBase.h"
#include "ui_QmitkTotalSegmentatorGUIControls.h"
#include <MitkSegmentationUIExports.h>
#include <QMessageBox>
#include "QmitknnUNetGPU.h"
#include <QSettings>


class MITKSEGMENTATIONUI_EXPORT QmitkTotalSegmentatorToolGUI : public QmitkMultiLabelSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkTotalSegmentatorToolGUI, QmitkMultiLabelSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots :
 
  /**
   * @brief Qt Slot
   */
  void OnPreviewBtnClicked();

  /**
   * @brief Qt Slot
   */
  void OnPythonPathChanged(const QString &);

  /**
    @brief Qt Slot
   */
  void OnInstallBtnClicked();

protected:
  QmitkTotalSegmentatorToolGUI();
  ~QmitkTotalSegmentatorToolGUI() = default;

  void ConnectNewTool(mitk::SegWithPreviewTool* newTool) override;
  void InitializeUI(QBoxLayout* mainLayout) override;

  void EnableWidgets(bool enabled) override;

  /**
    @brief Enable (or Disable) GUI elements.
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

  const std::string m_WARNING_TOTALSEG_NOT_FOUND = "TotalSegmentator is not detected in the selected python environment.Please select a valid "
                        "python environment or install TotalSegmentator.";
  const QStringList m_VALID_TASKS = {"total", "cerebral_bleed", "hip_implant", "coronary_arteries"};
  const QString m_VENV_NAME = ".totalsegmentator";
  const std::string m_TOTALSEGMENTATOR_VERSION = "1.5.2";
};

#endif
