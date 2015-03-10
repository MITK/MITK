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

#ifndef QmitkNavigationToolStorageSelectionWidget_H
#define QmitkNavigationToolStorageSelectionWidget_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include <mitkNavigationToolStorage.h>
#include <mitkNavigationDataSource.h>
#include <usServiceReference.h>
//ui header
#include "ui_QmitkNavigationToolStorageSelectionWidgetControls.h"


 /** Documentation:
  *   \brief This widget allows the user to select a navigation tool storage.
  *
  *          The widget lists all navigation tool storages which are available
  *          as microservice via the module context.
  *
  *          A signal is emmited whenever the tool selection changes.
  *
  *   \ingroup IGTUI
  */
class MITKIGTUI_EXPORT QmitkNavigationToolStorageSelectionWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkNavigationToolStorageSelectionWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkNavigationToolStorageSelectionWidget();

    /** @return Returns the currently selected NavigationToolStorage. Returns null if no storage is selected at the moment. */
    mitk::NavigationToolStorage::Pointer GetSelectedNavigationToolStorage();

  signals:
    /** @brief This signal is emitted when a new navigation tool storage is selected.
      * @param storage Holds the new selected navigation tool storage. Is null if the old storage is deselected and no new storage is selected.
      */
    void NavigationToolStorageSelected(mitk::NavigationToolStorage::Pointer storage);



  protected slots:

    void NavigationToolStorageSelected(us::ServiceReferenceU s);


  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkNavigationToolStorageSelectionWidgetControls* m_Controls;

    mitk::NavigationToolStorage::Pointer m_CurrentStorage;



};
#endif
