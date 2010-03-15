/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-06 10:44:43 +0200 (Mi, 06 Mai 2009) $
Version:   $Revision: 17109 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QmitkIGTRecorderView_H__INCLUDED)
#define QmitkIGTRecorderView_H__INCLUDED

#include "QmitkFunctionality.h"
#include "mitkTestingConfig.h"

#include "mitkTrackingDeviceSource.h"
#include "mitkTrackingDevice.h"
#include "mitkNavigationDataRecorder.h"
#include "mitkNavigationDataPlayer.h"
#include "mitkNavigationDataToPointSetFilter.h"
#include "mitkNavigationDataToMessageFilter.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"

#include "ui_QmitkIGTRecorderViewControls.h"
#include "../IgtexampleDll.h"

class QmitkStdMultiWidget;
class QmitkIGTRecorderViewControls;
class QTextEdit;
class QmitkNDIConfigurationWidget;
/*!
\brief QmitkIGTRecorderView 

Example functionality that shows the usage of the MITK-IGT component

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkIGTRecorderView : public QObject, public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  /*!  
  \brief default constructor  
  */  
  QmitkIGTRecorderView(QObject *parent=0, const char *name=0);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkIGTRecorderView();

  /*!  
  \brief method for creating the widget containing the application controls, like sliders, buttons etc.  
  */  
  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

protected slots:  

  void OnStartRecording();
  void OnStopRecording();
  void OnPauseRecording(bool pause);
  void OnStartReplaying();
  void OnStopReplaying();
  void RecordFrame();
  void OnConnect();
  void OnDisconnect();

protected:  

  mitk::TrackingDevice::Pointer ConfigureTrackingDevice();  ///< create the selected tracker object and configure it (using values from m_Controls)
  void SetupIGTPipeline(mitk::TrackingDevice::Pointer tracker, QString fileName);
  mitk::DataNode::Pointer CreateInstrumentVisualization(const char* label);
 
  QmitkStdMultiWidget * m_MultiWidget; ///< default render widget
  Ui::QmitkIGTRecorderControls * m_Controls; ///< GUI widget for this functionality

  mitk::TrackingDeviceSource::Pointer m_Source; ///< first filter in the pipeline
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer;
  mitk::NavigationDataRecorder::Pointer m_Recorder; ///< records NDs to a XML file
  mitk::NavigationDataToPointSetFilter::Pointer m_PointSetFilter; ///< has a NDs as input and a PointSet as output
  mitk::NavigationDataToMessageFilter::Pointer m_MessageFilter; ///< calls OnErrorValueChanged when the error value of its input changes

  //mitk::NavigationDataPlayer::Pointer m_Player; ///< plays a XML file

  QTimer* m_Timer; ///< timer for continuous tracking update
  QTimer* m_RecordingTimer; ///< timer for continuous recording
  QTimer* m_PlayingTimer; ///< timer for continuous playing
  QStringList m_ToolList; ///< list to the tool description files
  QmitkNDIConfigurationWidget* m_ConfigWidget;
};
#endif // !defined(QmitkIGTRecorderView_H__INCLUDED)
