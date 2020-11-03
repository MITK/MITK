/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNavigationDataSourceSelectionWidget_H
#define QmitkNavigationDataSourceSelectionWidget_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include <mitkNavigationToolStorage.h>
#include <mitkNavigationDataSource.h>
#include <usServiceReference.h>
//ui header
#include "ui_QmitkNavigationDataSourceSelectionWidgetControls.h"


 /** Documentation:
  *   \brief This widget allows the user to select a NavigationDataSource. Tools of this Source are also shown and the user can select one of these tools.
  *   \ingroup IGTUI
  */
class MITKIGTUI_EXPORT QmitkNavigationDataSourceSelectionWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkNavigationDataSourceSelectionWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkNavigationDataSourceSelectionWidget() override;

    /** @return Returns the currently selected NavigationDataSource. Returns null if no source is selected at the moment. */
    mitk::NavigationDataSource::Pointer GetSelectedNavigationDataSource();

    /** @return Returns the ID of the currently selected tool. You can get the corresponding NavigationData when calling GetOutput(id)
      *         on the source object. Returns -1 if there is no tool selected.
      */
    int GetSelectedToolID();

    /** @return Returns the NavigationTool of the current selected tool if a NavigationToolStorage is available. Returns nullptr if
      *         there is no storage available or if no tool is selected.
      */
    mitk::NavigationTool::Pointer GetSelectedNavigationTool();

    /** @return Returns the NavigationToolStorage of the currently selected NavigationDataSource. Returns nullptr if there is no
      *         source selected or if the source has no NavigationToolStorage assigned.
      */
    mitk::NavigationToolStorage::Pointer GetNavigationToolStorageOfSource();

  signals:
    /** @brief This signal is emitted when a new navigation data source is selected.
      * @param n Holds the new selected navigation data source. Is null if the old source is deselected and no new source is selected.
      */
    void NavigationDataSourceSelected(mitk::NavigationDataSource::Pointer n);


    /** @brief This signal is emitted when a new navigation data tool is selected.
    * @param n Holds the new selected navigation tool. Is null if the old source is deselected and no new source is selected.
    */
    void NavigationToolSelected(mitk::NavigationTool::Pointer n);



  protected slots:

    void NavigationDataSourceSelected(us::ServiceReferenceU s);

    void NavigationToolSelected(int selection);


  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkNavigationDataSourceSelectionWidgetControls* m_Controls;



    mitk::NavigationToolStorage::Pointer m_CurrentStorage;
    mitk::NavigationDataSource::Pointer m_CurrentSource;


};
#endif
