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

  /*!
  \brief Adds the NavigationData to the existing ones.
  */
  void AddNavigationData(mitk::NavigationData::Pointer nd);

  /*!
  \brief Changes background color of status labels (green or red) to show if actual navigation data of each tool is valid.
  * Depending on usage of SetShowPosition(bool) or SetShowQuaternions(bool) the position coordinates and quaternion values of each tool are shown.
  * The number of decimal places is set with the parameters posPrecision and quatPrecision.
  */
  void Refresh(int posPrecision = 2, int quatPrecision = 2);

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

