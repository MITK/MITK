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


#ifndef IGTTrackingLabView_h
#define IGTTrackingLabView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_IGTTrackingLabViewControls.h"

#include <mitkNavigationDataToPointSetFilter.h>
#include <mitkNavigationDataLandmarkTransformFilter.h>
#include <mitkNavigationDataObjectVisualizationFilter.h>
#include <mitkNavigationDataToPointSetFilter.h>
#include <mitkTrackingDeviceSource.h>

#include <QToolBox>
#include <QCheckBox>

class QmitkNDIConfigurationWidget;
class QmitkFiducialRegistrationWidget;
class QmitkUpdateTimerWidget;
class QmitkToolTrackingStatusWidget;



/*!
  \brief IGTTrackingLabView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
*/
class IGTTrackingLabView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    static const std::string VIEW_ID;

    IGTTrackingLabView();
    virtual ~IGTTrackingLabView();

    virtual void CreateQtPartControl(QWidget *parent);

  protected slots:
  
    void OnAddRegistrationTrackingFiducial();
    void OnRegisterFiducials();
    void OnTrackerDisconnected();
    void OnSetupNavigation();
    void OnChangeToolName(int index, QString name);
    void OnToolLoaded(int index, mitk::DataNode::Pointer toolNode);
    void OnStartNavigation();
    void OnStopNavigation();
    


    /// \brief Called when the user clicks the GUI button
//    void DoImageProcessing();

  protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
//    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    Ui::IGTTrackingLabViewControls m_Controls;
    void CreateBundleWidgets( QWidget* parent );
    void CreateConnections();

    void SetupIGTPipeline();
    void InitializeFilters();
    void DestroyIGTPipeline();
    void StartContinuousUpdate();
    void StopContinuousUpdate();
    void RenderScene();

    
    mitk::DataNode::Pointer CreateInstrumentVisualization(mitk::DataStorage* ds, const char* toolName); // create 3D models for all connected tools
    mitk::DataNode::Pointer CreateConeRepresentation(const char* label = ""); // create a 3D cone as representation for a tool
  


    //void DestroyInstrumentVisualization(mitk::DataStorage* ds, mitk::TrackingDevice::Pointer tracker);




    mitk::TrackingDeviceSource::Pointer m_Source; ///< source that connects to the tracking device

    mitk::NavigationDataLandmarkTransformFilter::Pointer m_FiducialRegistrationFilter; ///< this filter transforms from tracking coordinates into mitk world coordinates
    mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer; ///< visualization filter
    mitk::NavigationDataToPointSetFilter::Pointer m_RegistrationTrackingFiducialsFilter;  ///< fill the Registration Tracking Fiducials with instrument position in tracking space 
  



private:

  QToolBox* m_ToolBox;
  QCheckBox* m_TrackingVolumeCheckbox;

  QmitkNDIConfigurationWidget* m_NDIConfigWidget;  // tracking device configuration widget
  QmitkFiducialRegistrationWidget* m_RegistrationWidget; // landmark registration widget
  QmitkUpdateTimerWidget* m_RenderingTimerWidget; // update timer widget for rendering and updating
  QmitkToolTrackingStatusWidget* m_ToolStatusWidget; // tracking status visualizer widget

  std::string m_RegistrationTrackingFiducialsName;
  std::string m_RegistrationImageFiducialsName;
};

#endif // IGTTrackingLabView_h

