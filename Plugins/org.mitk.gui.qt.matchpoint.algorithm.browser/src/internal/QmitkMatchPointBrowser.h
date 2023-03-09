/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMatchPointBrowser_h
#define QmitkMatchPointBrowser_h

#include <QmitkAbstractView.h>

#include <berryISelectionListener.h>

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
    berryObjectMacro(QmitkMatchPointBrowser);

        QmitkMatchPointBrowser();
    ~QmitkMatchPointBrowser() override;

    /**
    * \brief Called by the framework to indicate that the preferences have changed.
    * \param prefs not used, as we call RetrievePreferenceValues().
    */
    void OnPreferencesChanged(const mitk::IPreferences* prefs) override;

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
    void CreateQtPartControl(QWidget* parent) override;
    void SetFocus() override;

    Ui::MatchPointBrowserControls m_Controls;

    //! [Qt Selection Provider]
    /** @brief this pointer holds the selection provider*/
    mitk::AlgorithmInfoSelectionProvider::Pointer m_SelectionProvider;
    //! [Qt Selection Provider]

private:
    void SetSelectionProvider() override;

    void Error(QString msg);

    /**
    * \brief Called on startup and by OnPreferencesChanged to load all
    * preferences except the temporary folder into member variables.
    */
    void RetrieveAndStorePreferenceValues();

    void OnInvalidDeploymentEvent(const ::itk::Object *, const itk::EventObject &event);
    void OnValidDeploymentEvent(const ::itk::Object *, const itk::EventObject &event);

    /**
    * \brief Called to get hold of the actual preferences node.
    */
    mitk::IPreferences* RetrievePreferences();

    QWidget* m_Parent;

    ::map::deployment::DLLDirectoryBrowser::DLLInfoListType m_DLLInfoList;

    QStringList m_currentSearchPaths;
    QmitkAlgorithmListModel* m_algModel;
    QSortFilterProxyModel* m_filterProxy;

    ::map::deployment::DLLHandle::Pointer m_LoadedDLLHandle;
    ::map::algorithm::RegistrationAlgorithmBase::Pointer m_LoadedAlgorithm;
};

#endif
