/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKTOOLPAIRNAVIGATIONVIEW_H_INCLUDED
#define _QMITKTOOLPAIRNAVIGATIONVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkToolPairNavigationViewControls.h"
#include <mitkNavigationDataObjectVisualizationFilter.h>
#include <mitkCameraVisualization.h>
#include <mitkTrackingDeviceSource.h>
#include <mitkNavigationDataToMessageFilter.h>

class QmitkNDIConfigurationWidget;

/*!
 * \ingroup org_mitk_gui_qt_igttoolpairnavigation_internal
 *
 * \brief This view can be used to guide one tracking tool to another tracking tool. 
 * The plugin connects to a tracking system and shows the connected/added tools in two 3D widgets (standard 3D widget is interactable, the other (widget2) isn't).
 * Furthermore it calculates the distance between the two tools and displays it in GUI. 
 * In widget 4 the tool connected on port b is shown from the point of view of the tool in port a. 
 *
 * \sa QmitkFunctionality
 */
class QmitkToolPairNavigationView : public QObject, public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkToolPairNavigationView();
  virtual ~QmitkToolPairNavigationView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

protected slots:

  void Disconnected();
  void ToolsAdded(QStringList tools);

  /**
  * @brief starting navigation
  **/
  void OnStartNavigation(); 
  
  /**
  * @brief pausing navigation
  **/
  void OnPauseNavigation(bool pause);
  
  /**
  * @brief stopping navigation
  **/
  void OnStopNavigation();
  void OnShowTrackingVolume(bool on);///< this will show/hide the tracking volume
  void RenderScene();

protected:

  typedef std::map<std::string, unsigned int> NameToIndexMap;
  typedef std::list<std::string> StringList;

  /**
  * @brief setup the IGT pipeline
  **/
  void SetupIGTPipeline();
  
  /**
  * @brief stops the tracking and deletes all pipeline objects
  **/
  void DestroyIGTPipeline();  
  
  /**
  * @brief stop continuous rendering
  **/
  void StopContinuousUpdate();  

  /**
  * @brief start continuous rendering
  **/
  void StartContinuousUpdate(unsigned int frameRate);
  
  /**
  * @brief create objects for visualization
  **/
  mitk::DataNode::Pointer CreateConeAsInstrumentVisualization(const char* label = ""); 
  mitk::DataNode::Pointer CreateSphereAsInstrumentVisualization(const char* label = ""); 
  
  /**
  * @brief display status "valid" of tools
  **/
  void OnDataValidChanged(bool newValue, unsigned int index);  ///< callback method for IGT event filter. This method changes the text/color of the navigation tool to indicate whether the instrument is tracked correctly
  
  /**
  * @brief remove the visualized objects
  **/
  void RemoveVisualizationObjects( mitk::DataStorage* ds );

  /**
  * @brief MultiWidget for this view
  **/
  QmitkStdMultiWidget* m_MultiWidget;

  /**
  * @brief timer
  **/
  QTimer* m_RenderingTimer;
  
  /**
  * @brief source of the tracking system
  **/
  mitk::TrackingDeviceSource::Pointer m_Source; 
  
  /**
  * @brief visualizer
  **/
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer; ///< visualization filter
  
  mitk::CameraVisualization::Pointer m_CameraVisualizer;

  /**
  * @brief a message filter
  **/
  mitk::NavigationDataToMessageFilter::Pointer m_MessageFilter; ///< message filter will call back OnDataValidChanged() every time the instrument tracking becomes invalid/valid  

  //GUI widget to connect to a NDI tracker
  QmitkNDIConfigurationWidget* m_NDIConfigWidget;

  QString m_NavigationOfflineText;

  /**
  *@brief setting up the bundle widgets
  **/
  void CreateBundleWidgets(QWidget* parent);

  Ui::QmitkToolPairNavigationViewControls m_Controls;

  int m_LastMapperIDWidget3;

};




#endif // _QMITKTOOLPAIRNAVIGATIONVIEW_H_INCLUDED

