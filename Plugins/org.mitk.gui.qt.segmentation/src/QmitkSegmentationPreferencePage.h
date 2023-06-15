/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentationPreferencePage_h
#define QmitkSegmentationPreferencePage_h

#include "org_mitk_gui_qt_segmentation_Export.h"

#include <berryIQtPreferencePage.h>
#include <QmitkSetupVirtualEnvUtil.h>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QmitknnUNetGPU.h>

class QWidget;

namespace Ui
{
  class QmitkSegmentationPreferencePageControls;
}


class QmitkSAMInstaller : public QmitkSetupVirtualEnvUtil
{
public:
  const QString VENV_NAME = ".sam";
  const QString SAM_VERSION = "1.0"; // currently, unused
  const std::vector<QString> PACKAGES = {QString("git+https://github.com/ASHISRAVINDRAN/sam-mitk.git")};
  const QString STORAGE_DIR;
  inline QmitkSAMInstaller(
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() +
                            qApp->organizationName() + QDir::separator())
    : QmitkSetupVirtualEnvUtil(baseDir), STORAGE_DIR(baseDir){};
  bool SetupVirtualEnv(const QString &) override;
  QString GetVirtualEnvPath() override;
};


class MITK_QT_SEGMENTATION QmitkSegmentationPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkSegmentationPreferencePage();
  ~QmitkSegmentationPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* widget) override;

  QWidget* GetQtControl() const override;

  bool PerformOk() override;

  void PerformCancel() override;

  void Update() override;

protected Q_SLOTS:

  void OnLabelSetPresetButtonClicked();
  void OnSuggestionsButtonClicked();
  void OnInstallBtnClicked();
  void OnClearInstall();
  QString OnSystemPythonChanged(const QString&);

protected:
  /**
   * @brief Searches and parses paths of python virtual enviroments
   * from predefined lookout locations
   */
  void AutoParsePythonPaths();
  
  /**
   * @brief Get the virtual env path from UI combobox removing any
   * extra special characters.
   *
   * @return QString
   */
  QString GetPythonPathFromUI(const QString&) const;

  /**
   * @brief Get the Exact Python Path for any OS
   * from the virtual environment path.
   * @return QString
   */
  QString GetExactPythonPath(const QString&) const;

  /**
   * @brief Adds GPU information to the gpu combo box.
   * In case, there aren't any GPUs avaialble, the combo box will be
   * rendered editable.
   */
  void SetGPUInfo();

  /**
   * @brief Returns GPU id of the selected GPU from the Combo box.
   * @return unsigned int
   */
  unsigned int FetchSelectedGPUFromUI() const;

  void WriteStatusMessage(const QString&);
  void WriteErrorMessage(const QString&);
  
  QNetworkAccessManager m_Manager;
  QmitkGPULoader m_GpuLoader;
  Ui::QmitkSegmentationPreferencePageControls* m_Ui;
  QmitkSAMInstaller m_Installer;
  QWidget* m_Control;
  QString m_PythonPath;
  bool m_IsInstalled = false;
  bool m_Initializing;
};

#endif
