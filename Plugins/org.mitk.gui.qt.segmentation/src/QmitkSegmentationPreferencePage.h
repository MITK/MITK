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
  const std::vector<QString> PACKAGES = {QString("numpy"),
                                         QString("opencv-python"),
                                         QString("git+https://github.com/facebookresearch/segment-anything.git"),
                                         QString("SimpleITK")};
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

protected:

  Ui::QmitkSegmentationPreferencePageControls* m_Ui;
  QmitkSAMInstaller m_Installer;
  QWidget* m_Control;

  bool m_Initializing;
};

#endif
