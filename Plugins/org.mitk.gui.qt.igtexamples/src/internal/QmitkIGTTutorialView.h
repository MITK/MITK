/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkIGTTutorialView_h
#define QmitkIGTTutorialView_h

#include <QmitkAbstractView.h>

#include <string>

#include "ui_QmitkIGTTutorialViewControls.h"

#include "mitkTrackingDeviceSource.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"

/**
* \brief QmitkIGTTutorial shows a small typically navigation MITK view
*
* Any kind of navigation application will start with the connection to a tracking system
* and as we do image guided procedures we want to show something on the screen. In this
* tutorial we connect to the NDI Polaris tracking system (or alternatively use a virtual tracking device)
* and we will show the movement of a tool as cone in the render window part.
*
* \sa also take a look at the CMakeLists.txt of this view to see how to
*     link to the mitkIGT library.
*/
class QmitkIGTTutorialView : public QmitkAbstractView
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkIGTTutorialView();
  ~QmitkIGTTutorialView() override;

  void CreateQtPartControl(QWidget *parent) override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  void SetFocus() override;

protected slots:

  /**
  * \brief Execute MITK-IGT Tutorial
  */
  void OnStartIGT();

  /**
  * \brief stop IGT scene and clean up
  */
  void OnStopIGT();

  /**
  * \brief timer based update of IGT scene
  */
  void OnTimer();

protected:

  Ui::QmitkIGTTutorialViewControls* m_Controls;

  mitk::TrackingDeviceSource::Pointer m_Source; ///< source filer that connects to the tracking device
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer; ///< visualization filter uses output from m_Source
  QTimer* m_Timer; ///< timer for continuous tracking update
};




#endif
