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


#ifndef QmitkIGTTrackingLabView_h
#define QmitkIGTTrackingLabView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkIGTTrackingLabViewControls.h"

#include <mitkNavigationDataToPointSetFilter.h>
#include <mitkNavigationDataLandmarkTransformFilter.h>
#include <mitkNavigationDataObjectVisualizationFilter.h>
#include <mitkNavigationDataToPointSetFilter.h>
#include <mitkTrackingDeviceSource.h>
#include <mitkSurface.h>
#include <mitkCameraVisualization.h>

#include <QToolBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class QmitkNDIConfigurationWidget;
class QmitkFiducialRegistrationWidget;
class QmitkUpdateTimerWidget;
class QmitkToolTrackingStatusWidget;


/*!
  \brief QmitkIGTTrackingLabView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
*/
class QmitkIGTTrackingLabView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    static const std::string VIEW_ID;

    /**
    \brief default constructor
    */
    QmitkIGTTrackingLabView();
    
    /**
    \brief default destructor
    */
    virtual ~QmitkIGTTrackingLabView();

    virtual void CreateQtPartControl(QWidget *parent);

  protected slots:
    /**
    \brief This method adds a new fiducial to the tracker fiducials PointSet.
    */
    void OnAddRegistrationTrackingFiducial();
    /**
    \brief This method calls the fiducial registration.
    */
    void OnRegisterFiducials();
    /**
    \brief This method should be called if deconnecting the tracker.
    */
    void OnTrackerDisconnected();
    /**
    \brief This method sets up the navigation pipeline before tracking is started.
    */
    void OnSetupNavigation();
    /**
    \brief This method changes the name of a specific tool.
    */
    void OnChangeToolName(int index, QString name);
    /**
    \brief This method creates the surface representation for a tool loaded from file.
    */
    void OnToolLoaded(int index, mitk::DataNode::Pointer toolNode);
    /**
    \brief This method starts the navigation.
    */
    void OnStartNavigation();
    /**
    \brief This method stops the navigation.
    */
    void OnStopNavigation();
    /**
    \brief This method performs the visualisation of all action.
    */
    void RenderScene();
    /**
    \brief This method should be called if new tools have been added to the tracking device.
    */
    void OnToolsAdded(QStringList toolsList);
    /**
    \brief This method reacts on toolbox item changes.
    */
    void OnToolBoxCurrentChanged(int index);
    /**
    \brief This method initializes the registration for the FiducialRegistrationWidget.
    */
    void InitializeRegistration();
    /**
    \brief This method reacts on tool surface changes.
    */
    void ChangeToolRepresentation( int toolID , mitk::Surface::Pointer surface );
    /**
    \brief This method starts the PointSet recording.
    */
    void OnPointSetRecording(bool record);
    /**
    \brief This method activates the virtual camera.
    */
    void OnVirtualCameraChanged(int toolNr);

  protected:


    enum ToolBoxElement  // enums for the different ToolBox item tabs.
    {
      NDIConfigurationWidget = 0,
      RegistrationWidget = 1,
      PointSetRecording = 2,
      VirtualCamera = 3
    };


    Ui::QmitkIGTTrackingLabViewControls m_Controls;
    /**
    \brief This method creates all widgets this bundle needs.
    */
    void CreateBundleWidgets( QWidget* parent );
    /**
    \brief This method creates the SIGNAL SLOT connections.
    */
    void CreateConnections();

    /**
    \brief This method sets up the filter pipeline.
    */
    void SetupIGTPipeline();
    /**
    \brief This method initializes all needed filters.
    */
    void InitializeFilters();
    /**
    \brief This method destroys the filter pipeline.
    */
    void DestroyIGTPipeline();
    /**
    \brief This method starts the continuos update of the tracking device and the rendering.
    */
    void StartContinuousUpdate();
    /**
    \brief This method stops the continuos update of the tracking device and the rendering.
    */
    void StopContinuousUpdate();

    /**
    \brief This method creates a DataNode for fiducial PointSets.
    */
    mitk::DataNode::Pointer CreateRegistrationFiducialsNode( const std::string& label, const mitk::Color& color);
    /**
    \brief This method creates the visualization for all connected instruments.
    */
    mitk::DataNode::Pointer CreateInstrumentVisualization(mitk::DataStorage* ds, const char* toolName); // create 3D models for all connected tools
    /**
    \brief This method creates a DataNode of a 3D cone object.
    */
    mitk::DataNode::Pointer CreateConeRepresentation(const char* label = ""); // create a 3D cone as representation for a tool
    /**
    \brief This method destroys the instrument visualization.
    */
    void DestroyInstrumentVisualization(mitk::DataStorage* ds, mitk::TrackingDevice::Pointer tracker);

    /**
    \brief This method creates a widget with all input objects needed for the PointSet recording.
    */
    QWidget* CreatePointSetRecordingWidget(QWidget* parent);
    /**
    \brief This method creates a widget with all input objects needed for the virtual camera view.
    */
    QWidget* CreateVirtualViewWidget(QWidget* parent);


    mitk::TrackingDeviceSource::Pointer m_Source; ///< source that connects to the tracking device

    mitk::NavigationDataLandmarkTransformFilter::Pointer m_FiducialRegistrationFilter; ///< this filter transforms from tracking coordinates into mitk world coordinates
    mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer; ///< visualization filter
    mitk::CameraVisualization::Pointer m_VirtualView; ///< filter to update the vtk camera according to the reference navigation data

    mitk::Vector3D  m_DirectionOfProjectionVector;///< vector for direction of projection of instruments



private:

  QToolBox* m_ToolBox;
  QComboBox* m_PSRecToolSelectionComboBox;
  QComboBox* m_VirtualViewToolSelectionComboBox;
  QPushButton* m_PointSetRecordPushButton;
  QCheckBox* m_VirtualViewCheckBox;

  mitk::PointSet::Pointer m_PSRecordingPointSet;

  QmitkNDIConfigurationWidget* m_NDIConfigWidget;  // tracking device configuration widget
  QmitkFiducialRegistrationWidget* m_RegistrationWidget; // landmark registration widget
  QmitkUpdateTimerWidget* m_RenderingTimerWidget; // update timer widget for rendering and updating
  QmitkToolTrackingStatusWidget* m_ToolStatusWidget; // tracking status visualizer widget

  std::string m_RegistrationTrackingFiducialsName;
  std::string m_RegistrationImageFiducialsName;
  
  std::string m_PointSetRecordingDataNodeName;
  bool m_PointSetRecording;

  /**
    \brief This method performs GlobalReinit() for the rendering widgets.
    */
  void GlobalReinit();

};

#endif // QmitkIGTTrackingLabView_h

