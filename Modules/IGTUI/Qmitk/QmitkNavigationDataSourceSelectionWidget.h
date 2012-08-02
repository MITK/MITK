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

#ifndef QmitkNavigationDataSourceSelectionWidget_H
#define QmitkNavigationDataSourceSelectionWidget_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include <mitkNavigationToolStorage.h>
#include <mitkNavigationDataSource.h>
#include <mitkServiceReference.h>
//ui header
#include "ui_QmitkNavigationDataSourceSelectionWidgetControls.h"


 /** Documentation:
  *   \brief GUI to access the IGT recorder.
  *   User can specify the file name where the output shall be stored and
  *   how long the recording shall be performed.
  *
  *   \ingroup IGTUI
  */
class MitkIGTUI_EXPORT QmitkNavigationDataSourceSelectionWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkNavigationDataSourceSelectionWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkNavigationDataSourceSelectionWidget();



  protected slots:

    void NavigationDataSourceSelected(mitk::ServiceReference* s);


  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkNavigationDataSourceSelectionWidgetControls* m_Controls;

    

    mitk::NavigationToolStorage::Pointer m_CurrentStorage;
    mitk::NavigationDataSource::Pointer m_CurrentSource;


};
#endif