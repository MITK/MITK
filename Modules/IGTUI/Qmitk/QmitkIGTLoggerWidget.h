/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkIGTLoggerWidget_H
#define QmitkIGTLoggerWidget_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include "mitkNavigationTool.h"
#include <mitkDataStorage.h>
#include "mitkNavigationDataRecorder.h"

//ui header
#include "ui_QmitkIGTLoggerWidgetControls.h"

 /** Documentation:
  *   \brief GUI to access the IGT recorder. 
  *   User can specify the file name where the output shall be stored and
  *   how long the recording shall be performed.
  *
  *   \ingroup IGTUI
  */
class MitkIGTUI_EXPORT QmitkIGTLoggerWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkIGTLoggerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkIGTLoggerWidget();

    void SetDataStorage(mitk::DataStorage* dataStorage);
    void SetRecorder(mitk::NavigationDataRecorder::Pointer recorder);


  protected slots:    
    void OnLoadDir();
    void OnStartRecording();    
    void OnRecording();
    void UpdateFilename();
    void UpdateRecordingTime();
    void StopRecording();


  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);
    void SetDefaultRecordingSettings();
    Ui::QmitkIGTLoggerWidgetControls* m_Controls;

    /** @brief holds the DataStorage */
    mitk::DataStorage::Pointer m_DataStorage;

    mitk::NavigationDataRecorder::Pointer m_Recorder; ///< records NDs to a XML file

    QString m_CmpFilename;
    QString m_Dir;
    QTimer* m_RecordingTimer;
    QString m_MilliSeconds;
    QString m_Samples;
    bool m_RecordingActivated;
};
#endif