/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkNavigationDataPlayerView_h
#define QmitkNavigationDataPlayerView_h

#include <berryISelectionListener.h>

//Qmitk
#include <QmitkFunctionality.h>
#include <QmitkIGTPlayerWidget.h>

#include <mitkLineVtkMapper3D.h>
#include <mitkSplineVtkMapper3D.h>

// ui
#include "ui_QmitkNavigationDataPlayerViewControls.h"

//mitk
#include <mitkColorSequenceCycleH.h>
#include <mitkNavigationDataObjectVisualizationFilter.h>





/*!
\brief QmitkNavigationDataPlayerView 

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkNavigationDataPlayerView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:  

  static const std::string VIEW_ID;

  QmitkNavigationDataPlayerView();
  QmitkNavigationDataPlayerView(const QmitkNavigationDataPlayerView& other)
  {
    Q_UNUSED(other)
      throw std::runtime_error("Copy constructor not implemented");
  }
  virtual ~QmitkNavigationDataPlayerView();

  virtual void CreateQtPartControl(QWidget *parent);

  /**
  \brief This method creates this bundle's SIGNAL and SLOT connections
  */
  virtual void CreateConnections();

  
  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();


  protected slots:
  
    /*!  
    \brief Creates DataNodes for all available playback objects
    */ 
    void OnCreatePlaybackVisualization();
    /*!  
    \brief Assigns position changings from the player widget to the visualization objects
    */ 
    void OnPerformPlaybackVisualization();
    /*!  
    \brief Reinits this player. Cleans all timers and trajectory data 
    */ 
    void OnReinit();
    /*!  
    \brief Shows trajectory of tool with index
    */
    void OnShowTrajectory(int index);
    /*!  
    \brief Cleans trajectory data before playing is started
    */
    void OnPlayingStarted();
    /*!  
    \brief Enables or disables trajectory visualization with splines
    */
    void OnEnableSplineTrajectoryMapper(bool enable);
    

protected:

  enum TrajectoryStyle { 
    Points = 1,
    Splines = 2    
  };

  void CreateBundleWidgets(QWidget* parent);
  
   /**
    \brief Refreshes the visualization of the playback object DataNodes.
    */
  void RenderScene();
  
  /**
    \brief Creates representation DataNode with given name and color
  */
  mitk::DataNode::Pointer CreateRepresentationObject( const std::string& name , const mitk::Color color );
  /**
    \brief Adds representation DataNode to the DataStorage
  */
  void AddRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject);
  /**
    \brief Removes representation DataNode from the DataStorage
  */
  void RemoveRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject);

  /**
    \brief Adds trajectory DataNode to the DataStorage
  */
  void AddTrajectory(mitk::DataStorage* ds, mitk::DataNode::Pointer trajectoryNode);

  /**
    \brief Creates a trajectory DataNode from given PointSet with given name and color
  */
  mitk::DataNode::Pointer CreateTrajectory( mitk::PointSet::Pointer points, const std::string& name, const mitk::Color color );

  


  Ui::QmitkNavigationDataPlayerViewControls* m_Controls;
  
  QmitkStdMultiWidget* m_MultiWidget;
  QmitkIGTPlayerWidget* m_PlayerWidget; ///< this bundle's playback widget
   
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer; ///< this filter visualizes the navigation data

  std::vector<mitk::DataNode::Pointer> m_RepresentationObjects; ///< vector for current visualization objects

  mitk::DataNode::Pointer m_Trajectory; ///< main trajectory visualization DataNode
  mitk::PointSet::Pointer m_TrajectoryPointSet; ///< PointSet with all points for trajectory
  int m_TrajectoryIndex;  ///< trajectory tool index
  
  bool m_ReloadData;  ///< flag needed for refresh of visualization if needed
  bool m_ShowTrajectory;  ///< flag needed for trajectory visualization

  mitk::SplineVtkMapper3D::Pointer m_SplineMapper; ///< spline trajectory mapper
  mitk::PointSetVtkMapper3D::Pointer m_PointSetMapper; ///< standard trajectroy mapper

 


private:
  /**
    \brief Returns color from colorcycle with given index
  */
  mitk::Color GetColorCircleColor(int index);
  /**
    \brief Returns the trajectory mapper for the given style if stýle is not Points or Splines NULL will be returned.
  */
  mitk::PointSetVtkMapper3D::Pointer GetTrajectoryMapper(TrajectoryStyle style);

};



#endif // _QMITKNAVIGATIONDATAPLAYERVIEW_H_INCLUDED

