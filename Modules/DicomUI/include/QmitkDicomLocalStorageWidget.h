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
#include <MitkDicomUIExports.h>

// include ctk
#include <ctkDICOMDatabase.h>
#include <ctkDICOMIndexer.h>
#include <ctkFileDialog.h>

//include QT
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QWidget>

class QProgressDialog;
class QLabel;

/**
* \brief QmitkDicomLocalStorageWidget is a QWidget providing functionality for dicom storage and import.
*
* \sa QmitkFunctionality
* \ingroup Functionalities
*/
class MITKDICOMUI_EXPORT QmitkDicomLocalStorageWidget : public QWidget
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const std::string Widget_ID;

   /**
    * \brief QmitkDicomLocalStorageWidget(QWidget *parent) constructor.
    *
    * \param parent is a pointer to the parent widget
    */
    QmitkDicomLocalStorageWidget(QWidget *parent);

   /**
    * \brief QmitkDicomExternalDataWidget destructor.
    */
    virtual ~QmitkDicomLocalStorageWidget();

   /**
    * \brief CreateQtPartControl(QWidget *parent) sets the view objects from ui_QmitkDicomExternalDataWidgetControls.h.
    *
    * \param parent is a pointer to the parent widget
    */
    virtual void CreateQtPartControl(QWidget *parent);

   /**
    * \brief SetDatabaseDirectory sets database directory.
    *
    * \param newDatabaseDirectory contains path to new database directoy.
    */
    void SetDatabaseDirectory(QString newDatabaseDirectory);

signals:

    /// @brief emitted when import into database is finished.
    void SignalFinishedImport();

   /**
    * @brief emitted when view button is clicked.
    * @param QHash containing dicom UIDs properties.
    */
    void SignalDicomToDataManager(QHash<QString,QVariant>);

    /// \brief emitted if cancel button is pressed.
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

    /// @brief Called when the selection in the series table has changed
    void OnSeriesSelectionChanged(const QStringList&);

protected:

    void SetDatabase(QString databaseFile);

    /// \brief SetupProgressDialog Sets up progress dialog.
    void SetupProgressDialog(QWidget* parent);

    QProgressDialog* m_ProgressDialog;
    QLabel* m_ProgressDialogLabel;

    ctkDICOMDatabase* m_LocalDatabase;
    ctkDICOMIndexer* m_LocalIndexer;
    Ui::QmitkDicomLocalStorageWidgetControls* m_Controls;
};



#endif // _QmitkDicomLocalStorageWidget_H_INCLUDED

