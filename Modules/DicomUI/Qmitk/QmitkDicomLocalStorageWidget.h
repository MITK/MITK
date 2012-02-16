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

    /* @brief   Initializes the widget. This method has to be called before widget can start. 
    * @param dataStorage The data storage the widget should work with.
    * @param multiWidget The corresponding multiwidget were the ct Image is displayed and the user should define his path.
    * @param imageNode  The image node which will be the base of mitral processing
    */

    void SetDatabaseDirectory(QString newDatabaseDirectory);

signals:
    void FinishedImport(QString);
    
    public slots:

        /// @brief Called when cancel button was clicked.
       void OnViewButtonClicked();

       /// @brief Called when cancel button was clicked.
       void OnCancelButtonClicked();

        /// @brief   Called delete button was clicked.
        void OnDeleteButtonClicked();

        /// @brief   Called when adding a dicom directory. Starts a thread adding the directory.
        void StartDicomImport(QString& dicomData);

        /// @brief   Called when adding a list of dicom files. Starts a thread adding the dicom files.
        void StartDicomImport(QStringList& dicomData);


protected:

    // adds dicom files from a directory containing dicom files to the local storage. 
    void OnAddDICOMData(QString& dicomDirectory);

    // adds dicom files from a string list containing the filepath to the local storage.
    void OnAddDICOMData(QStringList& dicomFiles);

    void SetDatabase(QString databaseFile);

    ctkDICOMDatabase* m_LocalDatabase;
    ctkDICOMModel* m_LocalModel;
    ctkDICOMIndexer* m_LocalIndexer;
    Ui::QmitkDicomLocalStorageWidgetControls* m_Controls;

    QFuture<void> m_Future;
    QFutureWatcher<void> m_Watcher;
    QTimer* m_Timer;

    // Performs a starbust on inputimage, which results in outputimage. Only workds with 3D and 4D Ultrasound images (char Pixeltype)
    //void PerformStarburst(mitk::Image::Pointer inputImage, mitk::Image::Pointer &outputImage, mitk::Point3D startPoint, bool doubleStarburst, bool thinStarburst);



};



#endif // _QmitkDicomLocalStorageWidget_H_INCLUDED

