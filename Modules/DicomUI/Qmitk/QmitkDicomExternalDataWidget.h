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

#ifndef QmitkDicomExternalDataWidget_h
#define QmitkDicomExternalDataWidget_h

// #include <QmitkFunctionality.h>
#include "ui_QmitkDicomExternalDataWidgetControls.h"
#include "mitkDicomUIExports.h"

// include ctk
#include <ctkDICOMDatabase.h>
#include <ctkDICOMModel.h>
#include <ctkDICOMIndexer.h>

//include QT
#include <QWidget>
#include <QString>
#include <QStringList>
#include <QModelIndex>
//For running dicom import in background
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <QProgressDialog>
#include <QLabel>
#include <QHash>
#include <QVariant>

/*!
\brief QmitkDicomExternalDataWidget

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class MITK_DICOMUI_EXPORT QmitkDicomExternalDataWidget : public QWidget
{
   // this is needed for all Qt objects that should have a Qt meta-object
   // (everything that derives from QObject and wants to have signal/slots)
   Q_OBJECT

public:

   static const std::string Widget_ID;

   QmitkDicomExternalDataWidget(QWidget *parent);
   virtual ~QmitkDicomExternalDataWidget();

   virtual void CreateQtPartControl(QWidget *parent);

   /* @brief   Initializes the widget. This method has to be called before widget can start.
   * @param dataStorage The data storage the widget should work with.
   * @param multiWidget The corresponding multiwidget were the ct Image is displayed and the user should define his path.
   * @param imageNode  The image node which will be the base of mitral processing
   */
   void Initialize();

signals:

    /// @brief emitted when import into database is finished.
    void SignalStartDicomImport(const QStringList&);

        /// @brief emitted when import into database is finished.
    void SignalFinishedImport();

    /// @brief emitted when view button is clicked.
    void SignalDicomToDataManager(QHash<QString,QVariant>);
    void SignalProgress(int);
    void SignalProcessingFile(QString);
    void SignalCancelImport();

public slots:

    /// @brief Called when download button was clicked.
    void OnDownloadButtonClicked();

    /// @brief Called when view button was clicked.
    void OnViewButtonClicked();

    /// @brief   Called when search parameters change.
    void OnSearchParameterChanged();

    /// @brief   Called when adding a dicom directory. Starts a thread adding the directory.
    void OnStartDicomImport(const QString&);

    /// @brief Called when indexing into database is finished.
    /// In this slot the models database with new imports is set.
    /// This causes a model update.
    void OnFinishedImport();

protected:

    /// \brief Get the list of filepath from current selected index in TreeView. All file paths referring to the index will be returned.
    QStringList GetFileNamesFromIndex();

    ctkDICOMDatabase* m_ExternalDatabase;
    ctkDICOMModel* m_ExternalModel;
    ctkDICOMIndexer* m_ExternalIndexer;

    Ui::QmitkDicomExternalDataWidgetControls* m_Controls;

    QString m_LastImportDirectory;

};



#endif // _QmitkDicomExternalDataWidget_H_INCLUDED

