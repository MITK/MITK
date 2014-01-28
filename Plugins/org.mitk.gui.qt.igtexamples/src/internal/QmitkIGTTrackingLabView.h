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


#ifndef QmitkIGTTrackingLabView_h
#define QmitkIGTTrackingLabView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkIGTTrackingLabViewControls.h"

#include <mitkNavigationDataToPointSetFilter.h>
#include <mitkNavigationDataLandmarkTransformFilter.h>
#include <mitkNavigationDataReferenceTransformFilter.h>
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
#include <QSpinBox>
#include <QTimer>

/*!
\brief QmitkIGTTrackingLabView

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkIGTTrackingLabView : public QmitkAbstractView
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

virtual void SetFocus();

protected slots:

/** This timer updates the IGT pipline, when nessecary:
*   1: if permanent registration is activated, then the permanent
*      registration filter has to be updated
*   2: if the camera view is on it also must be updated
*   3: point set recording is based on another filter which needs to be
*      updated when activated
*/
void UpdateTimer();

//############## Configuration Step #####################

/**
\brief This method sets up the navigation pipeline during initialization.
*/
void OnSetupNavigation();

/** This method is called when the instrument is selected.
*  It stores the navigation data of the instrument.
*/
void OnInstrumentSelected();

/** This method is called when the object marker is selected.
*  It stores the navigation data of the object marker.
*/
void OnObjectmarkerSelected();

//############## Initial Registration Step ##############

/**
\brief This method adds a new fiducial to the tracker fiducials PointSet.
*/
void OnAddRegistrationTrackingFiducial();

/**
\brief This method calls the fiducial registration.
*/
void OnRegisterFiducials();

/**
\brief This method initializes the registration for the FiducialRegistrationWidget.
*/
void InitializeRegistration();

//############## Permanent Registration Step ############

/**
\brief This method activates the permanent registration based on one tool's position.
*/
void OnPermanentRegistration(bool on);

//############## Pointset Recording Step ################

/**
\brief This method starts the PointSet recording.
*/
void OnPointSetRecording(bool record);

//############## Camera View Step #######################
/**
\brief This method activates the virtual camera.
*/
void OnVirtualCamera(bool on);

protected:

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
* Checks if everything is initialized for registration. Gives error messages and returns false if not.
*/
bool CheckRegistrationInitialization();

/**
* Checks if the difference between two given transformations is high which means the method returns
* true if the difference exeeds the given position and angular threshold.
*/
bool IsTransformDifferenceHigh(mitk::NavigationData::Pointer transformA, mitk::NavigationData::Pointer transformB, double euclideanDistanceThreshold = .8, double angularDifferenceThreshold = .8);
/**
\brief This method sets up the filter pipeline.
*/
void SetupIGTPipeline();
/**
\brief This method destroys the filter pipeline.
*/
void DestroyIGTPipeline();

//####################### Members for the IGT pipeline ######################################
// The IGT pipeline is basically initialized in the method OnSetupNavigation(). Further initialization
// is done in the methods OnPermanentRegistration(), OnPointSetRecording() and OnVirtualCamera().
// The pipline is updated in the method UpdateTimer(). When the complete pipeline is active, it is
// structured as follows:
//           -> m_PermanentRegistrationFilter
//          /
// m_Source -> m_Visualizer
//          \
//           -> m_VirtualView
mitk::TrackingDeviceSource::Pointer m_Source;                                         ///< source that connects to the tracking device
mitk::NavigationDataObjectVisualizationFilter::Pointer m_PermanentRegistrationFilter; ///< this filter transforms from tracking coordinates into mitk world coordinates if needed it is interconnected before the FiducialEegistrationFilter
mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer;                  ///< visualization filter
mitk::CameraVisualization::Pointer m_VirtualView;                                     ///< filter to update the vtk camera according to the reference navigation data


QTimer* m_Timer;

mitk::PointSet::Pointer m_PSRecordingPointSet;

bool m_PointSetRecording;
bool m_PermanentRegistration;
bool m_CameraView;

mitk::DataNode::Pointer m_ImageFiducialsDataNode;
mitk::DataNode::Pointer m_TrackerFiducialsDataNode;

mitk::PointSet::Pointer m_PermanentRegistrationSourcePoints;

mitk::NavigationData::Pointer m_InstrumentNavigationData;
mitk::NavigationData::Pointer m_ObjectmarkerNavigationData;
mitk::NavigationData::Pointer m_PointSetRecordingNavigationData;

mitk::NavigationData::Pointer m_T_MarkerRel;
mitk::NavigationData::Pointer m_T_ObjectReg;
mitk::AffineTransform3D::Pointer m_T_ImageReg;
mitk::AffineTransform3D::Pointer m_T_ImageGeo;
mitk::NavigationData::Pointer m_ObjectmarkerNavigationDataLastUpdate;




};

#endif // QmitkIGTTrackingLabView_h

