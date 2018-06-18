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

#ifndef QMITKPATIENTTABLEINSPECTOR_H
#define QMITKPATIENTTABLEINSPECTOR_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include <QmitkPatientTableModel.h>

#include "ui_QmitkPatientTableInspector.h"

// qt widgets module
#include "QmitkAbstractDataStorageInspector.h"
#include "QmitkEnums.h"

// qt
#include <QMenu>

/*
* @brief The QmitkPatientTableInspector is a QmitkAbstractDataStorageInspector that shows the currently available data of
*        the patient table model in a control point-information type matrix.
*
*   The QmitkpatientTableInspector uses the QmitkPatientTableModel, a QmitkAbstractDataStorageModel that
*   presents the semantic relations data as a table, showing a QPixmap as thumbnail for the image data.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkPatientTableInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:

  QmitkPatientTableInspector(QWidget* parent = nullptr);

  virtual QAbstractItemView* GetView() override;
  virtual const QAbstractItemView* GetView() const override;

  virtual void SetSelectionMode(SelectionMode mode) override;
  virtual SelectionMode GetSelectionMode() const override;
  /**
  * @brief Extends the abstract base class to allow setting the current case ID which is needed to access the
  *        semantic relations storage. The function sets the case ID in the storage model.
  *
  * @param caseID    A case ID as string
  */
  void SetCaseID(const mitk::SemanticTypes::CaseID& caseID);

protected:

  virtual void QmitkPatientTableInspector::Initialize() override;

private Q_SLOTS:

  void OnPatientTableModelUpdated();
  void OnPatientTableViewContextMenuRequested(const QPoint&);
  void OnContextMenuSetInformationType();
  void OnContextMenuSetControlPoint();

private:

  void SetUpConnections();

  Ui::QmitkPatientTableInspector m_Controls;
  QmitkPatientTableModel* m_StorageModel;

  QMenu* m_ContextMenu;
  mitk::DataNode* m_SelectedDataNode;

};

#endif // QMITKPATIENTTABLEINSPECTOR_H
