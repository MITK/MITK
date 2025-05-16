/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTotalSegmentatorPreferencePage_h
#define QmitkTotalSegmentatorPreferencePage_h

#include <berryIQtPreferencePage.h>
#include "mitkIPreferences.h"
#include <ui_QmitkTotalSegmentatorPreferencePage.h>
#include "QmitkTotalSegmentatorToolInstaller.h"
#include <QMessageBox>
#include <QmitknnUNetGPU.h>
#include "QmitkToolInstallDialog.h"

namespace Ui
{
  class QmitkTotalSegmentatorPreferencePage;
}

class QmitkTotalSegmentatorPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkTotalSegmentatorPreferencePage();
  ~QmitkTotalSegmentatorPreferencePage() override = default;

  void Init(berry::IWorkbench::Pointer workbench) override;
  void CreateQtControl(QWidget *parent) override;
  QWidget *GetQtControl() const override;
  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:
 /**
   * @brief Creates a QMessage object and shows on screen.
   */
  void ShowErrorMessage(const QString &, QMessageBox::Icon = QMessageBox::Critical);

  /**
   * @brief Writes any message in white on the tool pane.
   */
  void WriteStatusMessage(const QString &);

  /**
   * @brief Writes any message in red on the tool pane.
   */
  void WriteErrorMessage(const QString &message);

  /**
   * @brief Adds GPU information to the device combo box.
   * In case, there aren't any GPUs available, the combo box will be
   * rendered editable and "cpu" will be added.
   */
  void SetDeviceInfo();

  /**
   * @brief Sets Buttons/Settings corresponding to wether TotalSegmentator
   */
  void UpdateStatusLabel();

  /**
   * @brief Get the virtual env path from UI combobox removing any
   * extra special characters.
   *
   * @return QString
   */
  QString GetPythonPathFromUI(const QString &pyUI) const;

  /**
   * @brief Checks if TotalSegmentator command is valid in the selected python virtual environment
   *  and if correct version is installed.
   * 
   * @return bool
   */
  bool IsTotalSegmentatorInstalled(const QString &pythonPath);

  /**
   * @brief Returns device id of the selected device from the Combo box.
   */
  int FetchSelectedDeviceFromUI() const;

  /**
   * @brief Applies license string to Totalsegmentator to faciliate use of licensed tasks.
   * Empty license string argument will remove the existing license.
   */
  void AddOrRemoveLicense(const QString &licenseText);


  void UpdateTotalSegPreferencePath();

  Ui::QmitkTotalSegmentatorPreferencePage *m_Ui;
  QWidget *m_Control;
  QString m_SysPythonPath;
  bool m_IsInstalled; // manual installed or not
  QmitkTotalSegmentatorToolInstaller m_Installer;
  QmitkGPULoader m_DeviceLoader;
  static const QString WARNING_TOTALSEG_NOT_FOUND;
  static const QString WARNING_PYTHON_NOT_FOUND;
  mitk::IPreferences *m_Preferences;
  const int LICENCE_KEY_LENGTH = 18;

protected slots:

  /**
   * @brief Qt Slot
   */
  void OnPythonPathChanged(const QString &pyEnv);

  /**
   * @brief Qt Slot
   */
  void OnSystemPythonChanged(const QString &pyEnv);

  /**
   * @brief Qt Slot
   */
  void OnOverrideBoxChecked(int state);

  /**
   * @brief Qt Slot
   */
  void OnClearButtonClicked();

  /**
   * @brief Qt Slot
   */
  void OnInstallButtonClicked();
  
  /**
   * @brief Qt Slot
   */
  void OnApplyButtonClicked();
};

#endif
