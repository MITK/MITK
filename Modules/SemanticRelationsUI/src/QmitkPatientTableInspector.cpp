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
#include "QmitkControlPointDialog.h"

// mitk qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationException.h>
#include <mitkUIDGeneratorBoost.h>

// qt
#include <QSignalMapper>

QmitkPatientTableInspector::QmitkPatientTableInspector(QWidget* parent/* =nullptr*/)
{
  m_Controls.setupUi(this);

  m_Controls.tableView->horizontalHeader()->setHighlightSections(false);
  m_Controls.tableView->verticalHeader()->setHighlightSections(false);
  m_Controls.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
  m_Controls.tableView->setContextMenuPolicy(Qt::CustomContextMenu);

  m_StorageModel = new QmitkPatientTableModel(this);

  m_Controls.tableView->setModel(m_StorageModel);

  SetUpConnections();
}

QAbstractItemView* QmitkPatientTableInspector::GetView()
{
  return m_Controls.tableView;
}

const QAbstractItemView* QmitkPatientTableInspector::GetView() const
{
  return m_Controls.tableView;
}

void QmitkPatientTableInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.tableView->setSelectionMode(mode);
}

QmitkPatientTableInspector::SelectionMode QmitkPatientTableInspector::GetSelectionMode() const
{
  return m_Controls.tableView->selectionMode();
}

void QmitkPatientTableInspector::SetCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_StorageModel->SetCaseID(caseID);
}

void QmitkPatientTableInspector::SetLesion(const mitk::SemanticTypes::Lesion& lesion)
{
  m_StorageModel->SetLesion(lesion);
}

QItemSelectionModel* QmitkPatientTableInspector::GetSelectionModel()
{
  return m_Controls.tableView->selectionModel();
}

void QmitkPatientTableInspector::Initialize()
{
  m_StorageModel->SetDataStorage(m_DataStorage.Lock());
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_Connector->SetView(m_Controls.tableView);
}

void QmitkPatientTableInspector::OnModelUpdated()
{
  m_Controls.tableView->resizeRowsToContents();
  m_Controls.tableView->resizeColumnsToContents();
}

void QmitkPatientTableInspector::OnNodeButtonClicked(const QString& nodeType)
{
  m_StorageModel->SetNodeType(nodeType.toStdString());
}

void QmitkPatientTableInspector::OnContextMenuSetControlPoint()
{
  QmitkControlPointDialog* inputDialog = new QmitkControlPointDialog(m_Controls.tableView);
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

  std::vector<mitk::SemanticTypes::ControlPoint> allControlPoints = m_StorageModel->GetSemanticRelations()->GetAllControlPointsOfCase(m_StorageModel->GetCaseID());
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

void QmitkPatientTableInspector::OnItemDoubleClicked(const QModelIndex& itemIndex)
{
  if (itemIndex.isValid())
  {
    QVariant qvariantDataNode = m_StorageModel->data(itemIndex, QmitkDataNodeRole);
    if (qvariantDataNode.canConvert<mitk::DataNode::Pointer>())
    {
      mitk::DataNode::Pointer dataNode = qvariantDataNode.value<mitk::DataNode::Pointer>();
      emit DataNodeDoubleClicked(dataNode);
    }
  }
}

void QmitkPatientTableInspector::SetUpConnections()
{
  connect(m_StorageModel, &QmitkPatientTableModel::ModelUpdated, this, &QmitkPatientTableInspector::OnModelUpdated);
  connect(m_Controls.tableView, &QTableView::customContextMenuRequested, this, &QmitkPatientTableInspector::OnContextMenuRequested);

  QSignalMapper* nodeButtonSignalMapper = new QSignalMapper(this);
  nodeButtonSignalMapper->setMapping(m_Controls.imageNodeButton, QString("Image"));
  nodeButtonSignalMapper->setMapping(m_Controls.segmentationNodeButton, QString("Segmentation"));
  connect(nodeButtonSignalMapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), this, &QmitkPatientTableInspector::OnNodeButtonClicked);
  connect(m_Controls.imageNodeButton, SIGNAL(clicked()), nodeButtonSignalMapper, SLOT(map()));
  connect(m_Controls.segmentationNodeButton, SIGNAL(clicked()), nodeButtonSignalMapper, SLOT(map()));
  m_Controls.imageNodeButton->setChecked(true);

  connect(m_Controls.tableView, &QTableView::doubleClicked, this, &QmitkPatientTableInspector::OnItemDoubleClicked);
}
