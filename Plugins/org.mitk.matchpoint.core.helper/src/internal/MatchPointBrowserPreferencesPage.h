/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __MATCHPOINT_BROWSER_PREFERENCE_PAGE_H
#define __MATCHPOINT_BROWSER_PREFERENCE_PAGE_H

#include "berryIQtPreferencePage.h"

class QWidget;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QmitkDirectoryListWidget;
class QmitkFileListWidget;
class ctkDirectoryButton;

class berryIPreferences;

/**
 * \class MatchPointBrowserPreferencesPage
 * \brief Preference page for the MatchPoint Browser plugin
 */
class MatchPointBrowserPreferencesPage : public QObject, public berry::IQtPreferencePage
{
    Q_OBJECT
        Q_INTERFACES(berry::IPreferencePage)

public:
    MatchPointBrowserPreferencesPage();
    ~MatchPointBrowserPreferencesPage() override;

    /**
     * \brief Called by framework to initialise this preference page, but currently does nothing.
     * \param workbench The workbench.
     */
    void Init(berry::IWorkbench::Pointer workbench) override;

    /**
     * \brief Called by framework to create the GUI, and connect signals and slots.
     * \param widget The Qt widget that acts as parent to all GUI components, as this class itself is not derived from QWidget.
     */
    void CreateQtControl(QWidget* widget) override;

    /**
     * \brief Required by framework to get hold of the GUI.
     * \return QWidget* the top most QWidget for the GUI.
     */
    QWidget* GetQtControl() const override;

    /**
     * \see IPreferencePage::PerformOk
     */
    bool PerformOk() override;

    /**
     * \see IPreferencePage::PerformCancel
     */
    void PerformCancel() override;

    /**
     * \see IPreferencePage::Update
     */
    void Update() override;

    public slots:

protected:

    QWidget*                  m_MainControl;
    QCheckBox*                m_DebugOutput;
    QmitkDirectoryListWidget* m_AlgDirectories;
    QmitkFileListWidget*      m_AlgFiles;
    QCheckBox*                m_LoadFromHomeDir;
    QCheckBox*                m_LoadFromCurrentDir;
    QCheckBox*                m_LoadFromApplicationDir;
    QCheckBox*                m_LoadFromAutoLoadPathDir;

    berry::IPreferences::Pointer m_BrowserPreferencesNode;

private:

    QString ConvertToString(const QStringList& list);
};

#endif // COMMANDLINEMODULESPREFERENCESPAGE_H
