/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDicomPreferencePage_h
#define QmitkDicomPreferencePage_h

#include <QString>
#include <QStringList>

#include "berryIQtPreferencePage.h"
#include <org_mitk_gui_qt_dicombrowser_Export.h>
#include <berryIPreferences.h>

class QWidget;
class QCheckBox;
class QLineEdit;
class QPushButton;

class DICOM_EXPORT QmitkDicomPreferencePage : public QObject, public berry::IQtPreferencePage
{
    Q_OBJECT
    Q_INTERFACES(berry::IPreferencePage)
public:

    QmitkDicomPreferencePage();

    ~QmitkDicomPreferencePage() override;

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

protected:
    QWidget* m_MainControl;
    berry::IPreferences::Pointer m_DicomPreferencesNode;

    QLineEdit* m_PathEdit;
    QPushButton* m_PathSelect;
    QPushButton* m_PathDefault;

protected slots:
    void DefaultButtonPushed();
    void PathSelectButtonPushed();
};
#endif // QmitkQmitkDicomPreferencePage_h
