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

#ifndef _QmitkToolTrackingStatusWidget_H_INCLUDED
#define _QmitkToolTrackingStatusWidget_H_INCLUDED

#include "ui_QmitkToolTrackingStatusWidgetControls.h"
#include "MitkIGTUIExports.h"

#include <mitkNavigationDataSource.h>
#include <QVector>
#include <QLabel>




/*!
\brief QmitkToolTrackingStatusWidget

Widget for setting up and controlling an update timer in an IGT-Pipeline.

*/
class MitkIGTUI_EXPORT QmitkToolTrackingStatusWidget : public QWidget
{  
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
public: 


  typedef std::vector< mitk::NavigationData::Pointer > NavigationDataPointerArray;



  /*!  
  \brief default constructor  
  */ 
  QmitkToolTrackingStatusWidget( QWidget* parent );

  /*!  
  \brief default destructor  
  */ 
  virtual ~QmitkToolTrackingStatusWidget();

  /*!  
  \brief Sets up the labels in this widget's QGridLayout for showing the track status of the tracking tools
  */ 
  void SetupStatusLabels();
  
  /*!
  \brief Sets the ND for this widget
  */
  void SetNavigationDatas(std::vector<mitk::NavigationData::Pointer>* navDatas);

  /*!  
  \brief Changes background color of status labels (green or red) to show if actual navigation data of each tool is valid.
  */ 
  void Update();
  
  /*!  
  \brief Removes all status labels.
  */ 
  void RemoveStatusLabels();

protected:
  void CreateConnections();
  void CreateQtPartControl( QWidget *parent );
  Ui::QmitkToolTrackingStatusWidgetControls* m_Controls;  ///< gui widgets


private:  

  /*!  
  \brief Vector for all tool tracking status labels.
  */
  QVector< QLabel* >* m_StatusLabels;

  std::vector<mitk::NavigationData::Pointer>*  m_NavigationDatas; 

};
#endif // _QmitkToolTrackingStatusWidget_H_INCLUDED

