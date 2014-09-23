/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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

  void Init(berry::IWorkbench::Pointer workbench);

  void CreateQtControl(QWidget* widget);

  QWidget* GetQtControl() const;

  ///
  /// \see IPreferencePage::PerformOk()
  ///
  virtual bool PerformOk();

  ///
  /// \see IPreferencePage::PerformCancel()
  ///
  virtual void PerformCancel();

  ///
  /// \see IPreferencePage::Update()
  ///
  virtual void Update();

  protected slots:
    virtual void UrlChanged();
    virtual void DownloadPathChanged();

    ///
    /// Toggles the Connection in the Service Registry from opened to closed or the other way around.
    ///
    virtual void ToggleConnection();

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
