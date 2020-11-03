/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKNAVIGATIONTOOLMANAGEMENTWIDGET_H
#define QMITKNAVIGATIONTOOLMANAGEMENTWIDGET_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include "mitkNavigationTool.h"
#include <mitkNavigationToolStorage.h>

//ui header
#include "ui_QmitkNavigationToolManagementWidgetControls.h"

/** Documentation:
 *   \brief An object of this class offers an UI to manage NavigationTools and
 *       NavigationToolStorages. This means a user may create, save and load
 *       single NavigationTools and/or NavigationToolStorages with this widget.
 *
 *      Be sure to call the Initialize-methode before you start the widget
 *      otherwise some errors might occure.
 *
 *   \ingroup IGTUI
 */
class MITKIGTUI_EXPORT QmitkNavigationToolManagementWidget : public QWidget
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  /** Initializes the widget. Has to be called before any action, otherwise errors might occur. */
  void Initialize(mitk::DataStorage* dataStorage);

  /** Loads a storage to the widget. The old storage storage is dropped, so be careful, if the
   *  storage is not saved somewhere else it might be lost. You might want to ask the user if he
   *  wants to save the storage to the harddisk before calling this method.
   *  @param storageToLoad This storage will be loaded and might be modified by the user.
   */
  void LoadStorage(mitk::NavigationToolStorage::Pointer storageToLoad);

  QmitkNavigationToolManagementWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkNavigationToolManagementWidget() override;

signals:

  /** This signal is emmited if a new storage was added by the widget itself, e.g. because
   *  a storage was loaded from the harddisk.
   *  @param newStorage Holds the new storage which was added.
   *  @param storageName Name of the new storage (e.g. filename)
   */
  void NewStorageAdded(mitk::NavigationToolStorage::Pointer newStorage, std::string storageName);

  protected slots:

  //main widget page:
  void OnAddTool();
  void OnDeleteTool();
  void OnEditTool();
  void OnLoadTool();
  void OnSaveTool();
  void OnMoveToolUp();
  void OnMoveToolDown();
  void OnLoadStorage();
  void OnSaveStorage();
  void OnCreateStorage();
  void OnToolSelected();

  //widget page "add tool":
  void OnAddToolCancel();
  void OnAddToolSave();

protected:

  /// \brief Creation of the connections
  virtual void CreateConnections();

  virtual void CreateQtPartControl(QWidget *parent);

  Ui::QmitkNavigationToolManagementWidgetControls* m_Controls;

  /** @brief holds the DataStorage */
  mitk::DataStorage* m_DataStorage;

  /** @brief holds the NavigationToolStorage we are working with. */
  mitk::NavigationToolStorage::Pointer m_NavigationToolStorage;

  /** @brief shows if we are in edit mode, if not we create new navigation tool objects. */
  bool m_edit;

  //############## private help methods #######################
  void MessageBox(const std::string &s);
  void UpdateToolTable();
  void DisableStorageControls();
  void EnableStorageControls();
};
#endif
