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

#if !defined(QmitkIGTExampleView_H__INCLUDED)
#define QmitkIGTExampleView_H__INCLUDED

#include "QmitkFunctionality.h"
#include "mitkTestingConfig.h"

#include "mitkTrackingDeviceSource.h"
#include "mitkNavigationDataDisplacementFilter.h"
#include "mitkTrackingDevice.h"
#include "mitkNavigationDataRecorder.h"
#include "mitkNavigationDataPlayer.h"
#include "mitkNavigationDataToPointSetFilter.h"
#include "mitkNavigationDataToMessageFilter.h"
#include "QmitkPlotWidget.h"

#include "ui_QmitkIGTExampleViewControls.h"
#include "../IgtexampleDll.h"

class QmitkStdMultiWidget;
class QmitkIGTExampleViewControls;
class QTextEdit;
/*!
\brief QmitkIGTExampleView 

Example functionality that shows the usage of the MITK-IGT component

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkIGTExampleView : public QObject, public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  /*!  
  \brief default constructor  
  */  
  QmitkIGTExampleView(QObject *parent=0, const char *name=0);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkIGTExampleView();

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

  virtual void AddToFunctionalityOptionsList(mitk::PropertyList* p);

  /*!  
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  //Port virtual QAction * CreateAction(QActionGroup *parent);

  /*!  
  \brief method is called when the bundle is started
  */  
  virtual void Activated();

  /*!  
  \brief method is called when the bundle is closed
  */  
  virtual void Deactivated();

#ifdef BUILD_TESTING
  /**
  * \brief This method performs an automated functionality test. 
  */
  virtual int TestYourself();
#endif

protected slots:  

  /**Documentation
   * \brief executes MITK-IGT-Tracking code
   *
   * This method will create and initialize a mitk::NDITrackingDevice with one tool.
   * It will start the tracking, read the tracking data from the tool 50 times and 
   * then clean up everything.
   */
   void OnTestTracking();

   /**Documentation
   * \brief executes MITK-IGT-Navigation code
   *
   * This method will create and initialize a mitk::NDITrackingDevice with one tool.
   * Then it builds an example MITK-IGT filter pipeline:
   * - TrackingDeviceSource filter initialized with the NDITrackingDevice as source of the pipeline
   * - NavigationDataDisplacementFilter
   * The Offset parameter of the displacement filter is searched in the functionality's options list.
   * It can be written to that list either by the method OnParametersChanged() that gets called after
   * the user changes the parameter in the GUI or by the persistence mechanism of MITK (the complete 
   * options list will be saved to disk on application exit and restored on the next application restart).
   * if the Offset parameter is not found in the list, a default value is used.
   * After building the filter pipeline, it is initialized and tracking is started, so that following 
   * calls to OnMeasure() can trigger pipeline updates and retrieve NavigationData objects.
   */   
   void OnTestNavigation();
  
   /**Documentation
   * \brief performs one measurement using the navigation pipeline
   *
   * This method uses the MITK-IGT pipeline that was set up by OnTestNavigation() before.
   * it iterates over all outputs of the last filter in the pipeline, updates each and writes
   * its transformed tracking data to the GUI.
   */
   void OnMeasure();

   /**Documentation
   * \brief performs continuous measurements using the navigation pipeline
   *
   * This method calls the above OnMeasure()
   */
   void OnMeasureContinuously();

   /**
   * \brief stops the navigation pipeline and perform clean up
   */
   void OnStop();

   /**Documentation
   * \brief reads the filter parameters from the GUI after they were changed
   *
   * The Control widget will store all parameters of the displacement filter
   * in a mitk::PropertyList. OnParametersChanged() gets called after the user
   * changed the parameters. It will add that list to the functionality's own
   * m_Options PropertyList, overwriting any properties that might exist with 
   * the same name (all MITK-IGT filters should use unique property names).
   * OnTestNavigation() will use the functionality's m_Options list to initialize
   * the DisplacementFilter.
   */
   void OnParametersChanged();

   /**Documentation
   * \brief Starts recording of tracking data
   *
   * This method sets up a IGT pipeline that connects a tracking device
   * with a NavigationDataRecorder filter. It will then start a timer that 
   * updates the recorder periodically until OnStartPlaying() is called.
   */
   void OnStartRecording();

   /**Documentation
   * \brief Stops the recording of tracking data and starts the replay
   *
   * This method stops the recording filter that was set up in OnStartRecording().
   * It sets up a new pipeline that connects a NavigationDataPlayer with a 
   * NavigationDataToPointSetFilter as an example for an alternative visualization
   * method. It then starts a Replay timer that updates the recorder periodically.
   */
   void OnStartPlaying();

   /**Documentation
   * \brief Timer update method for recording of tracking data
   *
   * updates the recording filter and displays a message in the status bar
   */
   void OnRecording();

   /**Documentation
   * \brief Timer update method for replaying of tracking data
   *
   * updates the replay filter, rerenders the 3D render window and 
   * displays a message in the status bar
   */
   void OnPlaying();

   /**Documentation
   * \brief Display a graph and progress bar that shows error values from the first navigation data
   *
   * This method is an example of how to use NavigationDataToMessageFilter to update
   * GUI elements from a IGT pipeline. It creates a NavigationDataToMessageFilter, 
   * sets its input to the first output of the displacement filter
   * and registers the OnErrorValueChanged() method as a callback whenever the
   * error value of the input navigation data changes.
   */
   void OnShowErrorPlot();

   /**Documentation
   * \brief Callback method of the NavigationDataToMessageFilter
   *
   * This method will be called by NavigationDataToMessageFilter when the
   * error value of its input navigation data object changes. 
   * The method calculates an overall error value and adds it both to an
   * error plot widget and to a progress bar. If the error is above a hardcoded
   * threshold, a warning is also displayed in the text output widget
   */
   void OnErrorValueChanged(mitk::NavigationData::CovarianceMatrixType v, unsigned int index);

   /**Documentation
   * \brief Shows a file dialog for choosing tool description files
   *
   * This method is called when the m_ToolBtn is pressed and a tracking
   * device was selected which needs tool description data.
   * 
   */
   void OnLoadTool();

   /**Documentation
   * \brief Chooses the current tracking device
   *
   * This method is called when the m_TrackingDevice selector changed.
   */
   void OnTrackingDeviceTextChanged( const QString & );

protected:  

  mitk::TrackingDevice::Pointer ConfigureTrackingDevice();  ///< create the selected tracker object and configure it (using values from m_Controls)

  QmitkStdMultiWidget * m_MultiWidget; ///< default render widget
  Ui::QmitkIGTExampleControls * m_Controls; ///< GUI widget for this functionality

  mitk::TrackingDeviceSource::Pointer m_Source; ///< first filter in the pipeline
  mitk::NavigationDataDisplacementFilter::Pointer m_Displacer;  ///< displacement filter that adds an offset to NDs
  mitk::NavigationDataToNavigationDataFilter::Pointer m_EndOfPipeline;  // Pointer to last filter in the pipeline
  mitk::NavigationDataRecorder::Pointer m_Recorder; ///< records NDs to a XML file
  mitk::NavigationDataPlayer::Pointer m_Player; ///< plays a XML file
  mitk::NavigationDataToPointSetFilter::Pointer m_PointSetFilter; ///< has a NDs as input and a PointSet as output
  mitk::NavigationDataToMessageFilter::Pointer m_MessageFilter; ///< calls OnErrorValueChanged when the error value of its input changes
  mitk::PointSet::Pointer m_PointSet; ///< stores the output of the pointsetfilter

  QmitkPlotWidget::DataVector m_XValues; ///< X-Values of the error plot (timestamp of navigation data)
  QmitkPlotWidget::DataVector m_YValues; ///< Y-Values of the error plot (error value of navigation data
  QTextEdit* out;   ///< pointer to output widget 

  QTimer* m_Timer; ///< timer for continuous tracking update
  QTimer* m_RecordingTimer; ///< timer for continuous recording
  QTimer* m_PlayingTimer; ///< timer for continuous playing

  QStringList m_ToolList; ///< list to the tool description files
};
#endif // !defined(QmitkIGTExampleView_H__INCLUDED)
