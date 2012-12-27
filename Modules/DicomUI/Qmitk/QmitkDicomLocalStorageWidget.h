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
#include <QHash>
#include <QVariant>

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

    /// @brief emitted when import into database is finished.
    void SignalFinishedImport();

    /// @brief emitted when view button is clicked.
    void SignalDicomToDataManager(QHash<QString,QVariant>);
    void SignalProgress(int);
    void SignalProcessingFile(QString);
    void SignalCancelImport();

public slots:

    /// @brief Called when indexing into database is finished.
    /// In this slot the models database with new imports is set.
    /// This causes a model update.
    void OnFinishedImport();

    /// @brief Called when view button was clicked.
   void OnViewButtonClicked();

    /// @brief   Called delete button was clicked.
    void OnDeleteButtonClicked();

    /// @brief   Called when adding a dicom directory. Starts a thread adding the directory.
    void OnStartDicomImport(const QString& dicomData);

    /// @brief   Called when adding a list of dicom files. Starts a thread adding the dicom files.
    void OnStartDicomImport(const QStringList& dicomData);

    /// @brief   Called when search parameters change.
    void OnSearchParameterChanged();

protected:

    void SetDatabase(QString databaseFile);

    ctkDICOMDatabase* m_LocalDatabase;
    ctkDICOMModel* m_LocalModel;
    ctkDICOMIndexer* m_LocalIndexer;
    Ui::QmitkDicomLocalStorageWidgetControls* m_Controls;
};



#endif // _QmitkDicomLocalStorageWidget_H_INCLUDED

