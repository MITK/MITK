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

#ifndef __Q_MITK_MATCHPOINT_H
#define __Q_MITK_MATCHPOINT_H

#include <QmitkAbstractView.h>

#include <berryISelectionListener.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>

//QT
#include <QSortFilterProxyModel>

// MatchPoint
#include <mapDeploymentDLLHandle.h>
#include <mapRegistrationAlgorithmBase.h>
#include <mapMetaPropertyAlgorithmInterface.h>

#include "ui_QmitkMatchPointBrowserControls.h"
#include "QmitkMAPAlgorithmModel.h"
#include "QmitkAlgorithmListModel.h"
#include "mitkAlgorithmInfoSelectionProvider.h"

/*!
\brief MatchPoint

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkMatchPointBrowser : public QmitkAbstractView
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    /**
    * Creates smartpointer typedefs
    */
    berryObjectMacro(QmitkMatchPointBrowser)

        QmitkMatchPointBrowser();
    virtual ~QmitkMatchPointBrowser();

    /**
    * \brief Called by the framework to indicate that the preferences have changed.
    * \param prefs not used, as we call RetrievePreferenceValues().
    */
    void OnPreferencesChanged(const berry::IBerryPreferences* prefs);

    protected slots:

    /**
    * @brief Connect all GUI elements to its corresponding slots
    */
    virtual void CreateConnections();

    /// \brief Called when the user clicks the GUI button

    void OnSearchFolderButtonPushed();

    void OnAlgoListSelectionChanged(const QModelIndex&);

    void OnSearchChanged(const QString&);

protected:
    virtual void CreateQtPartControl(QWidget* parent);
    virtual void SetFocus();

    Ui::MatchPointBrowserControls m_Controls;

    //! [Qt Selection Provider]
    /** @brief this pointer holds the selection provider*/
    mitk::AlgorithmInfoSelectionProvider::Pointer m_SelectionProvider;
    //! [Qt Selection Provider]

private:
    void SetSelectionProvider();

    void Error(QString msg);

    /**
    * \brief Called on startup and by OnPreferencesChanged to load all
    * preferences except the temporary folder into member variables.
    */
    void RetrieveAndStorePreferenceValues();

    /**
    * \brief Called to get hold of the actual preferences node.
    */
    berry::IBerryPreferences::Pointer RetrievePreferences();

    QWidget* m_Parent;

    ::map::deployment::DLLDirectoryBrowser::DLLInfoListType m_DLLInfoList;

    QStringList m_currentSearchPaths;
    QmitkAlgorithmListModel* m_algModel;
    QSortFilterProxyModel* m_filterProxy;

    ::map::deployment::DLLHandle::Pointer m_LoadedDLLHandle;
    ::map::algorithm::RegistrationAlgorithmBase::Pointer m_LoadedAlgorithm;
};

#endif // MatchPoint_h

