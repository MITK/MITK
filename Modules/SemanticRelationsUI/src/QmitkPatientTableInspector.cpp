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

// semantic relations UI module
#include "QmitkPatientTableInspector.h"
#include "QmitkPatientTableModel.h"
#include "QmitkControlPointDialog.h"

#include "QmitkCustomVariants.h"

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationException.h>
#include <mitkSemanticTypes.h>
#include <mitkUIDGeneratorBoost.h>

// qt
#include <QInputDialog>

QmitkPatientTableInspector::QmitkPatientTableInspector(QWidget* parent/* =nullptr*/)
  : QmitkAbstractDataStorageInspector(parent)
{
  m_Controls.setupUi(this);

  m_Controls.view->horizontalHeader()->setHighlightSections(false);
  m_Controls.view->verticalHeader()->setHighlightSections(false);
  m_Controls.view->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.view->setSelectionBehavior(QAbstractItemView::SelectItems);
  m_Controls.view->setContextMenuPolicy(Qt::CustomContextMenu);

  m_StorageModel = new QmitkPatientTableModel(this);

  m_Controls.view->setModel(m_StorageModel);

  m_ContextMenu = new QMenu(m_Controls.view);

  SetUpConnections();
}

QAbstractItemView* QmitkPatientTableInspector::GetView()
{
  return m_Controls.view;
}

const QAbstractItemView* QmitkPatientTableInspector::GetView() const
{
  return m_Controls.view;
}

void QmitkPatientTableInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.view->setSelectionMode(mode);
}

QmitkPatientTableInspector::SelectionMode QmitkPatientTableInspector::GetSelectionMode() const
{
  return m_Controls.view->selectionMode();
}

void QmitkPatientTableInspector::SetCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_StorageModel->SetCaseID(caseID);
}

void QmitkPatientTableInspector::Initialize()
{
  m_StorageModel->SetDataStorage(m_DataStorage.Lock());
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_Connector->SetView(m_Controls.view);
}

void QmitkPatientTableInspector::OnPatientTableModelUpdated()
{
  m_Controls.view->resizeRowsToContents();
  m_Controls.view->resizeColumnsToContents();
}

void QmitkPatientTableInspector::OnPatientTableViewContextMenuRequested(const QPoint& pos)
{
  QModelIndex selectedIndex = m_Controls.view->indexAt(pos);
  if (!selectedIndex.isValid())
  {
    return;
  }

  QVariant qvariantDataNode = m_StorageModel->data(selectedIndex, QmitkDataNodeRawPointerRole);
  if (qvariantDataNode.canConvert<mitk::DataNode*>())
  {
    m_SelectedDataNode = qvariantDataNode.value<mitk::DataNode*>();

    m_ContextMenu->clear();

    QAction* setInformationTypeAction = new QAction("Set information type", m_ContextMenu);
    m_ContextMenu->addAction(setInformationTypeAction);
    connect(setInformationTypeAction, &QAction::triggered, this, &QmitkPatientTableInspector::OnContextMenuSetInformationType);

    QAction* setControlPointAction = new QAction("Set control point", m_ContextMenu);
    m_ContextMenu->addAction(setControlPointAction);
    connect(setControlPointAction, &QAction::triggered, this, &QmitkPatientTableInspector::OnContextMenuSetControlPoint);

    m_ContextMenu->popup(QCursor::pos());
  }
}

void QmitkPatientTableInspector::OnContextMenuSetInformationType()
{
  bool ok = false;
  QString text = QInputDialog::getText(m_Controls.view, tr("Set information type of selected node"), tr("Information type:"), QLineEdit::Normal, "", &ok);
  if (ok && !text.isEmpty())
  {
    m_StorageModel->GetSemanticRelations()->RemoveInformationTypeFromImage(m_SelectedDataNode);
    m_StorageModel->GetSemanticRelations()->AddInformationTypeToImage(m_SelectedDataNode, text.toStdString());
    m_StorageModel->UpdateModelData();
  }
}

void QmitkPatientTableInspector::OnContextMenuSetControlPoint()
{
  QmitkControlPointDialog* inputDialog = new QmitkControlPointDialog(m_Controls.view);
  inputDialog->setWindowTitle("Set control point");
  inputDialog->SetCurrentDate(mitk::GetDICOMDateFromDataNode(m_SelectedDataNode));

  int dialogReturnValue = inputDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  // store the current control point to relink it, if anything goes wrong
  mitk::SemanticTypes::ControlPoint originalControlPoint = m_StorageModel->GetSemanticRelations()->GetControlPointOfData(m_SelectedDataNode);
  // unlink the data, that is about to receive a new date
  // this is needed in order to not extend a single control point, to which the selected node is currently linked
  m_StorageModel->GetSemanticRelations()->UnlinkDataFromControlPoint(m_SelectedDataNode);

  const QDate& userSelectedDate = inputDialog->GetCurrentDate();
  mitk::SemanticTypes::Date date;
  date.UID = mitk::UIDGeneratorBoost::GenerateUID();
  date.year = userSelectedDate.year();
  date.month = userSelectedDate.month();
  date.day = userSelectedDate.day();

  std::vector<mitk::SemanticTypes::ControlPoint> allControlPoints = m_StorageModel->GetSemanticRelations()->GetAllControlPointsOfCase(m_StorageModel->GetCurrentCaseID());
  if (!allControlPoints.empty())
  {
    // need to check if an already existing control point fits/contains the user control point
    mitk::SemanticTypes::ControlPoint fittingControlPoint = mitk::FindFittingControlPoint(date, allControlPoints);
    if (!fittingControlPoint.UID.empty())
    {
      try
      {
        // found a fitting control point
        m_StorageModel->GetSemanticRelations()->LinkDataToControlPoint(m_SelectedDataNode, fittingControlPoint, false);
        m_StorageModel->UpdateModelData();
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The data can not be linked to the fitting control point.";
        try
        {
          // link to the original control point
          m_StorageModel->GetSemanticRelations()->LinkDataToControlPoint(m_SelectedDataNode, originalControlPoint, false);
        }
        catch (const mitk::SemanticRelationException&)
        {
          MITK_INFO << "The data can not be linked to its original control point. Inconsistency in the semantic relations storage assumed.";
        }
      }
      return;
    }

    // did not find a fitting control point, although some control points already exist
    // need to check if a close control point can be found and extended
    mitk::SemanticTypes::ControlPoint extendedControlPoint = mitk::ExtendClosestControlPoint(date, allControlPoints);
    if (!extendedControlPoint.UID.empty())
    {
      try
      {
        // found and extended a close control point
        m_StorageModel->GetSemanticRelations()->OverwriteControlPointAndLinkData(m_SelectedDataNode, extendedControlPoint, false);
        m_StorageModel->UpdateModelData();
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The extended control point can not be overwritten and the data can not be linked to this control point.";
        try
        {
          // link to the original control point
          m_StorageModel->GetSemanticRelations()->LinkDataToControlPoint(m_SelectedDataNode, originalControlPoint, false);
        }
        catch (const mitk::SemanticRelationException&)
        {
          MITK_INFO << "The data can not be linked to its original control point. Inconsistency in the semantic relations storage assumed.";
        }
      }
      return;
    }
  }

  // generate a control point from the user-given date
  mitk::SemanticTypes::ControlPoint controlPointFromUserDate = mitk::GenerateControlPoint(date);
  try
  {
    m_StorageModel->GetSemanticRelations()->AddControlPointAndLinkData(m_SelectedDataNode, controlPointFromUserDate, false);
    m_StorageModel->UpdateModelData();
  }
  catch (const mitk::SemanticRelationException&)
  {
    MITK_INFO << "The control point can not be added and the data can not be linked to this control point.";
    try
    {
      // link to the original control point
      m_StorageModel->GetSemanticRelations()->LinkDataToControlPoint(m_SelectedDataNode, originalControlPoint, false);
    }
    catch (const mitk::SemanticRelationException&)
    {
      MITK_INFO << "The data can not be linked to its original control point. Inconsistency in the semantic relations storage assumed.";
    }
  }
}

void QmitkPatientTableInspector::SetUpConnections()
{
  connect(m_StorageModel, SIGNAL(ModelUpdated()), SLOT(OnPatientTableModelUpdated()));
  connect(m_Controls.view, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(OnPatientTableViewContextMenuRequested(const QPoint&)));
}
