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

#if !defined(QMITKIGTEXAMPLE_H__INCLUDED)
#define QMITKIGTEXAMPLE_H__INCLUDED

#include "QmitkFunctionality.h"
#include "mitkTestingConfig.h"

#include "mitkTrackingDeviceSource.h"
#include "mitkNavigationDataDisplacementFilter.h"
#include "mitkNavigationDataVisualizationByBaseDataTransformFilter.h"
#include "mitkTrackingDevice.h"
#include "mitkNavigationDataRecorder.h"
#include "mitkNavigationDataPlayer.h"
#include "mitkNavigationDataToPointSetFilter.h"

class QmitkStdMultiWidget;
class QmitkIGTExampleControls;
class QTextEdit;
/*!
\brief QmitkIGTExample 

Example functionality that shows the usage of the MITK-IGT component

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkIGTExample : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  /*!  
  \brief default constructor  
  */  
  QmitkIGTExample(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkIGTExample();

  /*!  
  \brief method for creating the widget containing the application controls, like sliders, buttons etc.  
  */  
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /*!  
  \brief method for creating the applications main widget  
  */  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

  virtual void AddToFunctionalityOptionsList(mitk::PropertyList* p);

  /*!  
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();

#ifdef BUILD_TESTING
  /**
  * \brief This method performs an automated functionality test. 
  */
  virtual int TestYourself();
#endif

protected slots:  
  void TreeChanged();

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

   /**Documentation
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

   void OnStartRecording();

   void OnStartPlaying();

   void OnRecording();

   void OnPlaying();

protected:  

  mitk::TrackingDevice::Pointer ConfigureTrackingDevice();  ///< create the selected tracker object and configure it (using values from m_Controls)

  QmitkStdMultiWidget * m_MultiWidget; ///< default render widget
  QmitkIGTExampleControls * m_Controls; ///< GUI widget for this functionality

  mitk::TrackingDeviceSource::Pointer m_Source; ///< first filter in the pipeline
  mitk::NavigationDataDisplacementFilter::Pointer m_Displacer;  ///< displacement filter that adds an offset to NDs
  mitk::NavigationDataToNavigationDataFilter::Pointer m_EndOfPipeline;  // Pointer to last filter in the pipeline
  mitk::NavigationDataRecorder::Pointer m_Recorder; ///< records NDs to a XML file
  mitk::NavigationDataPlayer::Pointer m_Player; ///< plays a XML file
  mitk::NavigationDataToPointSetFilter::Pointer m_PointSetFilter; ///< has a NDs as input and a PointSet as output
  mitk::PointSet::Pointer m_PointSet; ///< stores the output of the pointsetfilter
  QTextEdit* out;   ///< pointer to output widget 

  QTimer* m_Timer; ///< timer for continuous tracking update
  QTimer* m_RecordingTimer; ///< timer for continuous recording
  QTimer* m_PlayingTimer; ///< timer for continuous playing
};
#endif // !defined(QMITKIGTEXAMPLE_H__INCLUDED)
