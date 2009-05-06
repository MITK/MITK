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

#if !defined(QmitkIGTTutorial_H__INCLUDED)
#define QmitkIGTTutorial_H__INCLUDED

#include "QmitkFunctionality.h"
#include "mitkTestingConfig.h"

#include "mitkTrackingDeviceSource.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"

class QmitkStdMultiWidget;
class QmitkIGTTutorialControls;

/**
* \brief QmitkIGTTutorial shows a small typically navigation MITK functionality.
*
* Any kind of navigation application will start with the connection to a tracking system 
* and as we do image guided procedures we want to show something on the screen. In this
* tutorial we connect to the NDI Polaris tracking system and we will show the movement 
* of a tool as cone in MITK.
*
*
* \ingroup Functionalities
*/
class QmitkIGTTutorial : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  /**
  \brief default constructor  
  */  
  QmitkIGTTutorial(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /**
  \brief default destructor  
  */  
  virtual ~QmitkIGTTutorial();

  /**
  \brief method for creating the widget containing the application controls, like sliders, buttons etc.  
  */  
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /**
  \brief method for creating the applications main widget  
  */  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  /**
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

  /**
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();

protected slots:  
  void TreeChanged();
  
  /**
   * \brief Execute MITK-IGT Tutorial
   */
   void OnDoIGT();

   /**
   * \brief timer based update of IGT scene
   */
   void OnTimer();

   /**
   * \brief stop IGT scene and clean up
   */
   void OnStop();
protected:  
  /**
  * default main widget containing 4 windows showing 3   
  * orthogonal slices of the volume and a 3d render window  
  */  
  QmitkStdMultiWidget * m_MultiWidget;

  /**
  * controls for the functionality. Sliders, Buttons, TreenodeSelectors,...  
  */  
  QmitkIGTTutorialControls * m_Controls;

  mitk::TrackingDeviceSource::Pointer m_Source; ///< first filter in the pipeline
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_Visualizer;
  QTimer* m_Timer; ///< timer for continuous tracking update
};
#endif // !defined(QmitkIGTTutorial_H__INCLUDED)
