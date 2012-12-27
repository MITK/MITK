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
  *   \brief This widget allows the user to select a NavigationDataSource. Tools of this Source are also shown and the user can select one of these tools.
  *   \ingroup IGTUI
  */
class MitkIGTUI_EXPORT QmitkNavigationDataSourceSelectionWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkNavigationDataSourceSelectionWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkNavigationDataSourceSelectionWidget();

    /** @return Returns the currently selected NavigationDataSource. Returns null if no source is selected at the moment. */
    mitk::NavigationDataSource::Pointer GetSelectedNavigationDataSource();

    /** @return Returns the ID of the currently selected tool. You can get the corresponding NavigationData when calling GetOutput(id)
      *         on the source object. Returns -1 if there is no tool selected.
      */
    int GetSelectedToolID();

    /** @return Returns the NavigationTool of the current selected tool if a NavigationToolStorage is available. Returns NULL if
      *         there is no storage available or if no tool is selected.
      */
    mitk::NavigationTool::Pointer GetSelectedNavigationTool();

    /** @return Returns the NavigationToolStorage of the currently selected NavigationDataSource. Returns NULL if there is no
      *         source selected or if the source has no NavigationToolStorage assigned.
      */
    mitk::NavigationToolStorage::Pointer GetNavigationToolStorageOfSource();

  signals:



  protected slots:

    void NavigationDataSourceSelected(mitk::ServiceReference s);


  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkNavigationDataSourceSelectionWidgetControls* m_Controls;



    mitk::NavigationToolStorage::Pointer m_CurrentStorage;
    mitk::NavigationDataSource::Pointer m_CurrentSource;


};
#endif