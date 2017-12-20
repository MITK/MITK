/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKDATASTORAGEDEFAULTVIEW_H
#define QMITKDATASTORAGEDEFAULTVIEW_H

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkDataStorageAbstractView.h>
#include <QmitkDataStorageDefaultListModel.h>

// qt
#include <QListView>

/**
* @brief The QmitkDataStorageDefaultView serves as a standard view on the data storage. It contains a simple
*        'QListView' and a custom 'QmitkDataStorageDefaultListModel'.
*        The model is basically a list model, that extends the 'QmitkIDataStorageViewModel' interface.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageDefaultView : public QmitkDataStorageAbstractView
{
  Q_OBJECT

public:

  QmitkDataStorageDefaultView(QWidget* parent = nullptr);
  QmitkDataStorageDefaultView(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);

  ~QmitkDataStorageDefaultView() override;

private:

  /*
  * @brief Properly initializes the UI.
  *
  *   Calls 'SetModel' and 'SetView' of the parent class to correctly connect its model and view.
  */
  void SetupUI();

  /*
  * @brief Callback for the data storage node added event. 
  *
  *   WIP implementation: Reset the model by resetting the data storage of the model.
  *
  * @par  node	The data node that was added.
  */
  void NodeAdded(const mitk::DataNode* node) override;
  /*
  * @brief Callback for the data storage node changed event.
  *
  *   WIP implementation: Reset the model by resetting the data storage of the model.
  *
  * @par		node	The data node that was changed.
  */
  void NodeChanged(const mitk::DataNode* node) override;
  /*
  * @brief Callback for the data storage node changed event.
  *
  *   WIP implementation: Reset the model by resetting the data storage of the model.
  *
  * @par		node	The data node that was removed.
  */
  void NodeRemoved(const mitk::DataNode* node) override;

  QmitkDataStorageDefaultListModel* m_DataStorageDefaultListModel;
  QListView* m_DataStorageListView;

};

#endif // QMITKDATASTORAGEDEFAULTVIEW_H
