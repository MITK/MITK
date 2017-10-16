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

#ifndef QMITKPATIENTTABLEWIDGET_H
#define QMITKPATIENTTABLEWIDGET_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include <ui_QmitkPatientTableWidgetControls.h>
#include "QmitkSelectionWidget.h"
#include "QmitkPatientTableModel.h"

// semantic relations module
#include <mitkISemanticRelationsObserver.h>

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

// qt
#include <QMenu>

/*
* @brief The QmitkPatientTableWidget is a widget that shows the currently available data of the semantic relations model in a control point-information type matrix.
*
*   The QmitkPatientTableWidget has a model that presents the semantic relations data as a table, showing a QPixmap as thumbnail for the image data.
*
*   The QmitkPatientTableWidget implements the 'ISemanticRelationsObserver', so that it is automatically updated, if the
*   semantic relations model changes. Updating means freshly getting all control point data and information type data.
*   This is done by updating the data in the 'QmitkPatientTableModel' model.
*
*   The QmitkPatientTableWidget is derived from 'QmitkSelectionWidget', so that the "SelectionChanged"-signal can be emitted.
*   This is done in order to inform other widgets about a changed selection in this widget (e.g. the 'QmitkSelectNodeDialog').
*   The QmitkPatientTableWidget is currently injected in the 'QmitkSelectNodeDialog' in the 'QmitkSemanticRelationsView' class.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkPatientTableWidget : public QmitkSelectionWidget, public mitk::ISemanticRelationsObserver
{
  Q_OBJECT

public:

  QmitkPatientTableWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);
  virtual ~QmitkPatientTableWidget();

  /*
  * @brief Propagates the 'Update'-call to the patient table model to update the patient data (control points and information types).
  *
  *       Overridden from 'ISemanticRelationsObserver'.
  *       In order for Update-function to be called, this widget has to be added as a observer of SemanticRelations
  *       (e.g. m_SemanticRelations->AddObserver(m_LesionInfoWidget);)
  *
  * @par caseID    The current case ID to identify the currently active patient / case.
  */
  virtual void Update(const mitk::SemanticTypes::CaseID& caseID) override;

  /*
  * @brief Sets a QPixmap of a DICOM image.
  *
  * @par dataNode    The data node that holds the image data.
  */
  void SetPixmapOfNode(const mitk::DataNode* dataNode);
  /*
  * @brief Removes the corresponding pixmap of a DICOM image
  *
  * @par dataNode    The data node that holds the image data.
  */
  void DeletePixmapOfNode(const mitk::DataNode* dataNode);

private Q_SLOTS:

  void OnPatientTableModelUpdated();
  void OnPatientTableViewContextMenuRequested(const QPoint&);
  void OnContextMenuSetInformationType();
  void OnContextMenuSetControlPoint();
  void OnPatientTableViewSelectionChanged(const QItemSelection&, const QItemSelection&);

private:

  void Init();
  void SetUpConnections();

  /*
  * @brief Generates a QPixmap of a DICOM image.
  *
  *       The center sagittal image slice is extracted and used as the thumbnail image.
  *
  * @par dataNode    The data node that holds the image data.
  */
  QPixmap GetPixmapFromImageNode(const mitk::DataNode* dataNode) const;

  Ui::QmitkPatientTableWidgetControls m_Controls;
  std::unique_ptr<QmitkPatientTableModel> m_PatientTableModel;
  QMenu* m_ContextMenu;
  mitk::DataNode* m_SelectedDataNode;

};

#endif // QMITKPATIENTTABLEWIDGET_H
