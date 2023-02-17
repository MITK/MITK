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

  void OnPreviewBtnClicked();

  /**
   * @brief Qt Slot
   */
  void OnPythonPathChanged(const QString &);

protected:
  QmitkTotalSegmentatorToolGUI();
  ~QmitkTotalSegmentatorToolGUI() = default;

  void ConnectNewTool(mitk::SegWithPreviewTool* newTool) override;
  void InitializeUI(QBoxLayout* mainLayout) override;

  void EnableWidgets(bool enabled) override;

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
   * @brief For storing values like Python path across sessions.
   */
  QSettings m_Settings;

  QString m_PythonPath;
  QmitkGPULoader m_GpuLoader;
  Ui_QmitkTotalSegmentatorToolGUIControls m_Controls;
  bool m_FirstPreviewComputation = true;
  EnableConfirmSegBtnFunctionType m_SuperclassEnableConfirmSegBtnFnc;
  
};

#endif
