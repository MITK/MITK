/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKXNATCONNECTIONPREFERENCEPAGE_H_
#define QMITKXNATCONNECTIONPREFERENCEPAGE_H_

#include "berryIQtPreferencePage.h"
#include <berryIPreferences.h>

#include "ui_QmitkXnatConnectionPreferencePageControls.h"

class QWidget;
class QLineEdit;

struct QmitkXnatConnectionPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkXnatConnectionPreferencePage();

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* widget) override;

  QWidget* GetQtControl() const override;

  ///
  /// \see IPreferencePage::PerformOk()
  ///
  bool PerformOk() override;

  ///
  /// \see IPreferencePage::PerformCancel()
  ///
  void PerformCancel() override;

  ///
  /// \see IPreferencePage::Update()
  ///
  void Update() override;

  protected slots:
    virtual void UrlChanged();
    virtual void DownloadPathChanged();
  void OnDownloadPathButtonClicked();

    ///
    /// Toggles the Connection in the Service Registry from opened to closed or the other way around.
    ///
    virtual void TestConnection();

  virtual void onUseNetworkProxy(bool);

protected:

  Ui::QmitkXnatConnectionPreferencePageControls m_Controls;

  QWidget* m_Control;

  berry::IPreferences::WeakPtr m_XnatConnectionPreferencesNode;

private:

  ///
  /// Checks if the entered user information is empty.
  ///
  virtual bool UserInformationEmpty();

};

#endif /* QMITKXNATCONNECTIONPREFERENCEPAGE_H_ */
