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

#ifndef QMITKLESIONINFOWIDGET_H
#define QMITKLESIONINFOWIDGET_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include <ui_QmitkLesionInfoWidgetControls.h>

// semantic relations module
#include <mitkSemanticRelations.h>
#include <mitkISemanticRelationsObserver.h>

// mitk
#include <mitkDataStorage.h>

// qt
#include <QWidget>

/*
* @brief The QmitkLesionInfoWidget is a widget that shows the currently available lesion data of the semantic relations model
*
*   The QmitkLesionInfoWidget provides three QListWidgets, that show the lesion data and the referenced segmentation data, as
*   well as the connected image data, depending on the selected lesion.
*
*   The QmitkLesionInfoWidget implements the 'ISemanticRelationsObserver', so that it is automatically updated, if the
*   semantic relations model changes. Updating means freshly getting all lesion data and filling the lesion-ListWidget with the lesion data.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkLesionInfoWidget : public QWidget, public mitk::ISemanticRelationsObserver
{
  Q_OBJECT

public:

  QmitkLesionInfoWidget::QmitkLesionInfoWidget(QWidget* parent = nullptr);
  ~QmitkLesionInfoWidget();

  void SetupConnections();

  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID);

  /*
  * @brief Updates the 'lesionListWidget' of the GUI with the current lesion-data from the semantic relations model.
  *
  *       Overridden from 'ISemanticRelationsObserver'.
  *       In order for Update-function to be called, this widget has to be added as a observer of the SemanticRelationsManager
  *       (e.g. m_SemanticRelationsManager->AddObserver(m_LesionInfoWidget);)
  *
  * @par caseID    The current case ID to identify the patient.
  */
  virtual void Update(const mitk::SemanticTypes::CaseID& caseID) override;

  /*
  * @brief Removes all items from the lesion list widget, the segmentation list widget and the image list widget
  */
  void ClearLesionInfoWidget();
  /*
  * @brief Removes all items from the the segmentation list widget and the image list widget
  */
  void ClearSegmentationList();
  /*
  * @brief Removes all items from the image list widget
  */
  void ClearImageList();

private Q_SLOTS:

  void OnLesionListSelectionChanged(const QItemSelection&, const QItemSelection&);
  void OnImageListSelectionChanged(const QItemSelection&, const QItemSelection&);
  void OnSegmentationListSelectionChanged(const QItemSelection&, const QItemSelection&);

private:

  Ui::QmitkLesionInfoWidgetControls m_Controls;
  mitk::DataStorage* m_DataStorage;
  std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;
  mitk::SemanticTypes::CaseID m_CaseID;
};

#endif // QMITKLESIONINFOWIDGET_H
