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
class QmitkNavigationDataPlayerView : public QObject, public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:  

  static const std::string VIEW_ID;

  QmitkNavigationDataPlayerView();
  virtual ~QmitkNavigationDataPlayerView();

  virtual void CreateQtPartControl(QWidget *parent);

  /**
  \brief This method creates this bundle's SIGNAL and SLOT connections
  */
  virtual void CreateConnections();


  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();


  protected slots:
    void CreatePlaybackVisualization();
    void PerformPlaybackVisualization();
    void Reinit();
    void OnShowTrajectory(int index);
    void OnPlayingStarted();
    void OnEnableSplineTrajectoryMapper(bool enable);

protected:

  enum TrajectoryStyle {
    Points = 1,
    Splines = 2    
  };

  void CreateBundleWidgets(QWidget* parent);
  void RenderScene();
  mitk::DataNode::Pointer CreateRepresentationObject( const std::string& name , const mitk::Color color );
  void AddRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject);
  void RemoveRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject);

  void AddTrajectory(mitk::DataStorage* ds, mitk::DataNode::Pointer trajectoryNode);

  mitk::DataNode::Pointer CreateTrajectory( mitk::PointSet::Pointer points, const std::string& name, const mitk::Color color );

  


  Ui::QmitkNavigationDataPlayerViewControls* m_Controls;
  
  QmitkStdMultiWidget* m_MultiWidget;
  QmitkIGTPlayerWidget* m_PlayerWidget;
   
  mitk::ColorSequenceCycleH* m_ColorCycle;
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer; // this filter visualizes the navigation data



  std::vector<mitk::DataNode::Pointer>* m_RepresentationObjects;

  mitk::DataNode::Pointer m_Trajectory;
  mitk::PointSet::Pointer m_TrajectoryPointSet;
  int m_TrajectoryIndex;




  
  bool m_ReloadData;
  bool m_ShowTrajectory;


  mitk::SplineVtkMapper3D::Pointer m_SplineMapper; // spline trajectory mapper
  mitk::PointSetVtkMapper3D::Pointer m_PointSetMapper; // standard trajectroy mapper

 


private:
  mitk::Color GetColorCircleColor(int index);

  mitk::PointSetVtkMapper3D::Pointer GetTrajectoryMapper(TrajectoryStyle style);

};



#endif // _QMITKNAVIGATIONDATAPLAYERVIEW_H_INCLUDED

