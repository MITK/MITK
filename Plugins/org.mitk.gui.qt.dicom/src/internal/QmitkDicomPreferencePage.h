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

#ifndef QmitkDicomPreferencePage_h
#define QmitkDicomPreferencePage_h

#include <QString>
#include <QStringList>

#include "berryIQtPreferencePage.h"
#include <org_mitk_gui_qt_dicom_Export.h>
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

    virtual ~QmitkDicomPreferencePage();

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