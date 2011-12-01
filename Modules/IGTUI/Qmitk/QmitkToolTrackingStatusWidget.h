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
#include <mitkNavigationToolStorage.h>
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

  enum Style
    {
    GridLowerStyle,
    VerticalUpperStyle
    };

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
  void ShowStatusLabels();
  
  /*!
  \brief Sets the ND for this widget
  */
  void SetNavigationDatas(std::vector<mitk::NavigationData::Pointer>* navDatas);


  void AddNavigationData(mitk::NavigationData::Pointer nd);

  /*!  
  \brief Changes background color of status labels (green or red) to show if actual navigation data of each tool is valid.
  */ 
  void Refresh();
  
  /*!  
  \brief Removes all status labels.
  */ 
  void RemoveStatusLabels();

  /** @brief Enables / disables if the tool positions are shown. Default is off.*/
  void SetShowPositions(bool enable);

  /** @brief Enables / disables if the tool quaternions are shown. Default is off.*/
  void SetShowQuaternions(bool enable);

  /** @brief Sets the text alignment of the tool labels. Default is center. Example: Use Qt::AlignLeft for left alignment. */
  void SetTextAlignment(Qt::AlignmentFlag alignment);

  /** @brief Sets the alignment style of this widget:
    *         GridLowerStyle: Tool labels are at the lower side of the widget in grid alignment
    *         VerticalUpperStyle: Tool labels are at the upper side in a vertical alignment (default)
    */
  void SetStyle(QmitkToolTrackingStatusWidget::Style newStyle);

  /** @brief Shows tool labels for the tools in the tool storage. This method can be called BEFORE connecting the navigation data to
   *         make a preview of the tools.
   */
  void PreShowTools(mitk::NavigationToolStorage::Pointer toolStorage);



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

  bool m_ShowPositions;

  bool m_ShowQuaternions;

  Qt::AlignmentFlag m_Alignment;

  QmitkToolTrackingStatusWidget::Style m_Style;

  void RemoveGuiLabels();

  /** @brief Adds an empty label which tells the user that currently no tool is availiable. */
  void AddEmptyLabel();


};
#endif // _QmitkToolTrackingStatusWidget_H_INCLUDED

