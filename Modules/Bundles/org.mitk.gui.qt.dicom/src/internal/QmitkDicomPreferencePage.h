/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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

class DICOM_EXPORT QmitkDicomPreferencePage : public QObject, public berry::IQtPreferencePage
{
    Q_OBJECT
    Q_INTERFACES(berry::IPreferencePage)
public:

    QmitkDicomPreferencePage();
    QmitkDicomPreferencePage(const QmitkDicomPreferencePage& other)
    {
        Q_UNUSED(other)
            throw std::runtime_error("Copy constructor not implemented");
    }

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
    berry::IPreferences::Pointer m_DicomPreferencesNode;
    QWidget* m_MainControl;
    QCheckBox* m_EnableSingleEditing;
    QCheckBox* m_PlaceNewNodesOnTop;
    QCheckBox* m_ShowHelperObjects;
    QCheckBox* m_ShowNodesContainingNoData;
    QCheckBox* m_UseSurfaceDecimation;
    QLineEdit* m_DatabaseDirectory;
    QLineEdit* m_ListenerDirectory;

};
#endif // QmitkQmitkDicomPreferencePage_h