/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKLESIONINFOWIDGET_H
#define QMITKLESIONINFOWIDGET_H

// semantic relations plugin
#include <ui_QmitkLesionInfoWidgetControls.h>

// semantic relations UI module
#include <QmitkLesionTreeModel.h>

// semantic relations module
#include <mitkSemanticRelationsDataStorageAccess.h>
#include <mitkSemanticRelationsIntegration.h>

// mitk
#include <mitkDataStorage.h>

// berry
#include <berryIWorkbenchPartSite.h>

// qt
#include <QWidget>

/*
* @brief The QmitkLesionInfoWidget is a widget that shows and modifies the currently available lesion data of the semantic relations model.
*
*   The widget provides a dialogs to add nodes from the data storage to the semantic relations model.
*   It provides functionality to create new lesions and link them with segmentation nodes.
*
*   The QmitkLesionInfoWidget provides three QListWidgets, that show the lesion data and the referenced segmentation data, as
*   well as the connected image data, depending on the selected lesion.
*/
class QmitkLesionInfoWidget : public QWidget
{
  Q_OBJECT

public:

  static const QBrush DEFAULT_BACKGROUND_COLOR;
  static const QBrush SELECTED_BACKGROUND_COLOR;
  static const QBrush CONNECTED_BACKGROUND_COLOR;

  QmitkLesionInfoWidget(mitk::DataStorage* dataStorage, berry::IWorkbenchPartSite::Pointer workbenchPartSite, QWidget* parent = nullptr);

  void SetCaseID(const mitk::SemanticTypes::CaseID& caseID);

  void SetDataNodeSelection(const QList<mitk::DataNode::Pointer>& dataNodeSelection);

Q_SIGNALS:

  void LesionSelectionChanged(const mitk::SemanticTypes::Lesion&);

private Q_SLOTS:

  void OnModelUpdated();
  /*
  * @brief Generates a new, empty lesion to add to the semantic relations model for the current case ID.
  */
  void OnAddLesionButtonClicked();

  // slots for the mouse click events of tree view's selection model
  void OnSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

  void OnLesionListContextMenuRequested(const QPoint&);

  // slots for the context menu actions of the lesion list widget
  void OnLinkToSegmentation(mitk::SemanticTypes::Lesion);
  void OnSetLesionName(mitk::SemanticTypes::Lesion);
  void OnSetLesionClass(mitk::SemanticTypes::Lesion);
  void OnCreateNewSegmentation(mitk::SemanticTypes::Lesion);
  void OnRemoveLesion(mitk::SemanticTypes::Lesion);

private:

  void Initialize();
  void SetUpConnections();

  void LinkSegmentationToLesion(const mitk::DataNode* selectedDataNode, mitk::SemanticTypes::Lesion selectedLesion);

  Ui::QmitkLesionInfoWidgetControls m_Controls;
  QmitkLesionTreeModel* m_StorageModel;

  mitk::SemanticTypes::CaseID m_CaseID;

  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  berry::IWorkbenchPartSite::WeakPtr m_WorkbenchPartSite;
  std::unique_ptr<mitk::SemanticRelationsDataStorageAccess> m_SemanticRelationsDataStorageAccess;
  std::unique_ptr<mitk::SemanticRelationsIntegration> m_SemanticRelationsIntegration;

};

#endif // QMITKLESIONINFOWIDGET_H
