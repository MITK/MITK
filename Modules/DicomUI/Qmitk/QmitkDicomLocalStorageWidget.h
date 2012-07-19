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
#ifndef QmitkDicomLocalStorageWidget_h
#define QmitkDicomLocalStorageWidget_h

// #include <QmitkFunctionality.h>
#include "ui_QmitkDicomLocalStorageWidgetControls.h"
#include "mitkDicomUIExports.h"

// include ctk
#include <ctkDICOMDatabase.h>
#include <ctkDICOMModel.h>
#include <ctkDICOMIndexer.h>
#include <ctkFileDialog.h>

//include QT
#include <QWidget>
#include <QString>
#include <QStringList>
//For running dicom import in background
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>

#include <mitkStatusBar.h>
#include <mitkProgressBar.h>
/*!
\brief QmitkDicomLocalStorageWidget 

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class MITK_DICOMUI_EXPORT QmitkDicomLocalStorageWidget : public QWidget
{  
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:  

    static const std::string Widget_ID;

    QmitkDicomLocalStorageWidget(QWidget *parent);
    virtual ~QmitkDicomLocalStorageWidget();

    virtual void CreateQtPartControl(QWidget *parent);

    void SetDatabaseDirectory(QString newDatabaseDirectory);

signals:
    void FinishedImport(const QString&);
    void FinishedImport(const QStringList&);
    void SignalDicomToDataManager(const QStringList&);
    
    public slots:

        /// @brief Called when cancel button was clicked.
       void OnViewButtonClicked();

       /// @brief Called when cancel button was clicked.
       void OnCancelButtonClicked();

        /// @brief   Called delete button was clicked.
        void OnDeleteButtonClicked();

        /// @brief   Called when adding a dicom directory. Starts a thread adding the directory.
        void StartDicomImport(const QString& dicomData);

        /// @brief   Called when adding a list of dicom files. Starts a thread adding the dicom files.
        void StartDicomImport(const QStringList& dicomData);

        /// @brief   Called when search parameters change.
        void OnSearchParameterChanged();


protected:

    // adds dicom files from a directory containing dicom files to the local storage. 
    void AddDICOMData(const QString& dicomDirectory);

    // adds dicom files from a string list containing the filepath to the local storage.
    void AddDICOMData(const QStringList& dicomFiles);

    void SetDatabase(QString databaseFile);

    ctkDICOMDatabase* m_LocalDatabase;
    ctkDICOMModel* m_LocalModel;
    ctkDICOMIndexer* m_LocalIndexer;
    Ui::QmitkDicomLocalStorageWidgetControls* m_Controls;

    QFuture<void> m_Future;
    QFutureWatcher<void> m_Watcher;
};



#endif // _QmitkDicomLocalStorageWidget_H_INCLUDED

