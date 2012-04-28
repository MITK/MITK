/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKIGTTUTORIALVIEW_H_INCLUDED
#define _QMITKIGTTUTORIALVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkIGTTutorialViewControls.h"
//#include "../IgttutorialDll.h"

#include "mitkTrackingDeviceSource.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"

/**
* \brief QmitkIGTTutorial shows a small typically navigation MITK functionality.
*
* Any kind of navigation application will start with the connection to a tracking system
* and as we do image guided procedures we want to show something on the screen. In this
* tutorial we connect to the NDI Polaris tracking system (or alternatively use a virtual tracking device)
* and we will show the movement of a tool as cone in the StdMultiWidget editor.
*
* \sa also take a look at the CMakeLists.txt of this functionality to see how to
*     link to the mitkIGT library.
* \sa QmitkFunctionality
* \ingroup Functionalities
*/
class QmitkIGTTutorialView : public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkIGTTutorialView();
  QmitkIGTTutorialView(const QmitkIGTTutorialView& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  virtual ~QmitkIGTTutorialView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

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

  QmitkStdMultiWidget* m_MultiWidget; ///< our display widget

  mitk::TrackingDeviceSource::Pointer m_Source; ///< source filer that connects to the tracking device
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer; ///< visualization filter uses output from m_Source
  QTimer* m_Timer; ///< timer for continuous tracking update
};




#endif // _QMITKIGTTUTORIALVIEW_H_INCLUDED
