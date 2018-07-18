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

#include "QmitkCustomVariants.h"

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationException.h>
#include <mitkUIDGeneratorBoost.h>

// qt
#include <QInputDialog>
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

  m_ContextMenu = new QMenu(m_Controls.tableView);

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

void QmitkPatientTableInspector::OnTableViewContextMenuRequested(const QPoint& pos)
{
  QModelIndex selectedIndex = m_Controls.tableView->indexAt(pos);
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
    connect(setInformationTypeAction, SIGNAL(triggered()), this, SLOT(OnContextMenuSetInformationType()));

    QAction* setControlPointAction = new QAction("Set control point", m_ContextMenu);
    m_ContextMenu->addAction(setControlPointAction);
    connect(setControlPointAction, SIGNAL(triggered()), this, SLOT(OnContextMenuSetControlPoint()));

    m_ContextMenu->popup(QCursor::pos());
  }
}

void QmitkPatientTableInspector::OnContextMenuSetInformationType()
{
  bool ok = false;
  QString text = QInputDialog::getText(m_Controls.tableView, tr("Set information type of selected node"), tr("Information type:"), QLineEdit::Normal, "", &ok);
  if (ok && !text.isEmpty())
  {
    m_StorageModel->GetSemanticRelations()->RemoveInformationTypeFromImage(m_SelectedDataNode);
    m_StorageModel->GetSemanticRelations()->AddInformationTypeToImage(m_SelectedDataNode, text.toStdString());
    m_StorageModel->UpdateModelData();
  }
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

void QmitkPatientTableInspector::SetUpConnections()
{
  connect(m_StorageModel, SIGNAL(ModelUpdated()), SLOT(OnModelUpdated()));
  connect(m_Controls.tableView, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(OnTableViewContextMenuRequested(const QPoint&)));

  QSignalMapper* nodeButtonSignalMapper = new QSignalMapper(this);
  nodeButtonSignalMapper->setMapping(m_Controls.imageNodeButton, QString("Image"));
  nodeButtonSignalMapper->setMapping(m_Controls.segmentationNodeButton, QString("Segmentation"));
  connect(nodeButtonSignalMapper, SIGNAL(mapped(const QString&)), this, SLOT(OnNodeButtonClicked(const QString&)));
  connect(m_Controls.imageNodeButton, SIGNAL(clicked()), nodeButtonSignalMapper, SLOT(map()));
  connect(m_Controls.segmentationNodeButton, SIGNAL(clicked()), nodeButtonSignalMapper, SLOT(map()));
  m_Controls.imageNodeButton->setChecked(true);
}
