/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentAnythingPreferencePage_h
#define QmitkSegmentAnythingPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <QmitknnUNetGPU.h>
#include <QString>
#include <QStandardPaths>
#include <ui_QmitkSegmentAnythingPreferencePage.h>
#include <QDir>
#include <QMessageBox>
#include <QmitkSegmentAnythingToolInstaller.h>

namespace Ui
{
  class QmitkSegmentAnythingPreferencePage;
}

class QmitkSegmentAnythingPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkSegmentAnythingPreferencePage();
  ~QmitkSegmentAnythingPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;
  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private slots:
  void OnInstallBtnClicked();
  void OnClearInstall();
  void OnSystemPythonChanged(const QString &pyEnv);

protected:
  /**
   * @brief Checks if SegmentAnything is found inside the selected python virtual environment.
   * @return bool
   */
  bool IsSAMInstalled(const QString &pythonPath);

  /**
   * @brief Get the virtual env path from UI combobox removing any
   * extra special characters.
   *
   * @return QString
   */
  QString GetPythonPathFromUI(const QString &) const;

  /**
   * @brief Adds GPU information to the gpu combo box.
   * In case, there aren't any GPUs available, the combo box will be
   * rendered editable.
   */
  void SetDeviceInfo();

  /**
   * @brief Returns GPU id of the selected GPU from the Combo box.
   * @return int
   */
  int FetchSelectedDeviceFromUI() const;

  void WriteStatusMessage(const QString &);
  void WriteErrorMessage(const QString &);

private:

  /**
   * @brief Creates a QMessage object and shows on screen.
   */
  void ShowErrorMessage(const QString &, QMessageBox::Icon = QMessageBox::Critical);

  Ui::QmitkSegmentAnythingPreferencePage* m_Ui;
  QmitkSAMToolInstaller m_Installer;
  QWidget* m_Control;
  QmitkGPULoader m_GpuLoader;
  QString m_PythonPath;
  QString m_SysPythonPath;
  const QString CPU_ID = "cpu";
  const QStringList VALID_MODELS = {"vit_b", "vit_l", "vit_h"};
  static const QString WARNING_PYTHON_NOT_FOUND;

  mitk::IPreferences *m_Preferences;
};

#endif
