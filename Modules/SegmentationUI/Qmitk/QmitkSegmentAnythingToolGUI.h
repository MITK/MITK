/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentAnythingToolGUI_h
#define QmitkSegmentAnythingToolGUI_h

#include "QmitkSegWithPreviewToolGUIBase.h"
#include <MitkSegmentationUIExports.h>
#include "ui_QmitkSegmentAnythingGUIControls.h"
#include "QmitknnUNetGPU.h"
#include "QmitkSetupVirtualEnvUtil.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkReply>


/**
 * @brief Installer class for SegmentAnythingModel Tool.
 * Class specifies the virtual environment name, install version, packages required to pip install
 * and implements SetupVirtualEnv method.
 *
 */
class QmitkSegmentAnythingToolInstaller : public QmitkSetupVirtualEnvUtil
{
public:
  const QString VENV_NAME = ".sam";
  const QString SAM_VERSION = "1.0"; //currently, unused
  const std::vector<QString> PACKAGES = {QString("numpy"),
                                         QString("opencv-python"),
                                         QString("git+https://github.com/facebookresearch/segment-anything.git"),
                                         QString("SimpleITK")};
  const QString STORAGE_DIR;
  inline QmitkSegmentAnythingToolInstaller(
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() +
                            qApp->organizationName() + QDir::separator())
    : QmitkSetupVirtualEnvUtil(baseDir), STORAGE_DIR(baseDir){};
  bool SetupVirtualEnv(const QString &) override;
  QString GetVirtualEnvPath() override;
};


/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::PickingTool.
\sa mitk::PickingTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkSegmentAnythingToolGUI : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkSegmentAnythingToolGUI, QmitkSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots:
  /**
   * @brief Qt Slot
   */
  void OnResetPicksClicked();

  /**
   * @brief Qt Slot
   */
  void OnActivateBtnClicked();

  /**
   * @brief Qt Slot
   */
  void OnInstallBtnClicked();

  /**
   * @brief Qt Slot
   */
  QString OnSystemPythonChanged(const QString&);

  /**
   * @brief Qt Slot
   */
  void OnClearInstall();

  /**
   * @brief Qt Slot
   */
  void FileDownloaded(QNetworkReply *); 

  /**
  * @brief Qt Slot
  */
  void OnParametersChanged(const QString &); 

protected:
  QmitkSegmentAnythingToolGUI();
  ~QmitkSegmentAnythingToolGUI() = default;

  void InitializeUI(QBoxLayout *mainLayout) override;
  
  /**
   * @brief Writes any message in white on the tool pane.
   */
  void WriteStatusMessage(const QString &);

  /**
   * @brief Writes any message in red on the tool pane.
   */
  void WriteErrorMessage(const QString &);

  /**
   * @brief Creates a QMessage object and shows on screen.
   */
  void ShowErrorMessage(const std::string &, QMessageBox::Icon = QMessageBox::Critical);

  /**
   * @brief Adds GPU information to the gpu combo box.
   * In case, there aren't any GPUs avaialble, the combo box will be
   * rendered editable.
   */
  void SetGPUInfo();

  /**
   * @brief Searches and parses paths of python virtual enviroments
   * from predefined lookout locations
   */
  void AutoParsePythonPaths();

  /**
   * @brief Returns GPU id of the selected GPU from the Combo box.
   * @return unsigned int
   */
  unsigned int FetchSelectedGPUFromUI() const;

  /**
   * @brief Checks if TotalSegmentator command is valid in the selected python virtual environment.
   * @return bool
   */
  bool IsSAMInstalled(const QString &);

  /**
   * @brief Get the Exact Python Path for any OS
   * from the virtual environment path.
   * @return QString
   */
  QString GetExactPythonPath(const QString &) const;

  /**
   * @brief Enable (or Disable) GUI elements.
   */
  void EnableAll(bool);

  /**
   * @brief Get the virtual env path from UI combobox removing any
   * extra special characters.
   *
   * @return QString
   */
  QString GetPythonPathFromUI(const QString &) const;
  
  /**
  * 
  */
  bool DownloadModel(const QString &);

  /**
  * 
  */
  void ShowProgressBar(bool);

  void ActivateSAMDaemon();


private:
  QmitkSegmentAnythingToolInstaller m_Installer;
  QNetworkAccessManager m_Manager;
  Ui_QmitkSegmentAnythingGUIControls m_Controls;
  QString m_PythonPath;
  QmitkGPULoader m_GpuLoader;
  bool m_FirstPreviewComputation = true;
  bool m_IsInstalled = false;
  const std::string WARNING_SAM_NOT_FOUND =
    "SAM is not detected in the selected python environment.Please reinstall SAM.";
  const QMap<QString, QUrl> VALID_MODELS_URL_MAP = {
    {"vit_b", QUrl("https://dl.fbaipublicfiles.com/segment_anything/sam_vit_b_01ec64.pth")},
    //{"vit_b", QUrl("http://www.google.ru/images/srpr/logo3w.png")},
    {"vit_l", QUrl("https://dl.fbaipublicfiles.com/segment_anything/sam_vit_l_0b3195.pth")},
    {"vit_h", QUrl("https://dl.fbaipublicfiles.com/segment_anything/sam_vit_h_4b8939.pth")}};
};
#endif
