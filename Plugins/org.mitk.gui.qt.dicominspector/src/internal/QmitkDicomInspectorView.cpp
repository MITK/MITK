/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

// mitk
#include <QmitkRenderWindow.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkTimeNavigationController.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

// Qt
#include <QMessageBox>
#include <QMessageBox>
#include <QFileDialog>
#include <qwt_plot_marker.h>

#include "QmitkDicomInspectorView.h"

const std::string QmitkDicomInspectorView::VIEW_ID = "org.mitk.views.dicominspector";

namespace
{
  QmitkAbstractNodeSelectionWidget::NodeList GetInitialSelection(berry::ISelection::ConstPointer selection)
  {
    if (selection.IsNotNull() && !selection->IsEmpty())
    {
      auto dataNodeSelection = dynamic_cast<const mitk::DataNodeSelection*>(selection.GetPointer());

      if (nullptr != dataNodeSelection)
      {
        auto firstSelectedDataNode = dataNodeSelection->GetSelectedDataNodes().front();

        if (firstSelectedDataNode.IsNotNull())
        {
          QmitkAbstractNodeSelectionWidget::NodeList initialSelection;
          initialSelection.push_back(firstSelectedDataNode);

          return initialSelection;
        }
      }
    }

    return QmitkAbstractNodeSelectionWidget::NodeList();
  }
}

QmitkDicomInspectorView::QmitkDicomInspectorView()
  : m_RenderWindowPart(nullptr)
  , m_SelectedNode(nullptr)
  , m_ValidSelectedPosition(false)
  , m_SelectedTimePoint(0.)
  , m_CurrentSelectedZSlice(0)
{
  m_SelectedPosition.Fill(0.0);
}

QmitkDicomInspectorView::~QmitkDicomInspectorView()
{
}

void QmitkDicomInspectorView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart != renderWindowPart)
  {
    m_RenderWindowPart = renderWindowPart;

    this->m_SliceNavigationListener.RenderWindowPartActivated(renderWindowPart);
  }
}

void QmitkDicomInspectorView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  m_RenderWindowPart = nullptr;
  this->m_SliceNavigationListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkDicomInspectorView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  auto nodePredicate = mitk::NodePredicateAnd::New();
  nodePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  nodePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

  m_Controls.singleSlot->SetDataStorage(GetDataStorage());
  m_Controls.singleSlot->SetSelectionIsOptional(true);
  m_Controls.singleSlot->SetNodePredicate(nodePredicate);
  m_Controls.singleSlot->SetEmptyInfo(QString("Please select a data node"));
  m_Controls.singleSlot->SetPopUpTitel(QString("Select data node"));

  m_SelectionServiceConnector = std::make_unique<QmitkSelectionServiceConnector>();
  SetAsSelectionListener(true);

  m_Controls.timePointValueLabel->setText(QString(""));
  m_Controls.sliceNumberValueLabel->setText(QString(""));

  connect(m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkDicomInspectorView::OnCurrentSelectionChanged);

  mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart();
  RenderWindowPartActivated(renderWindowPart);
  this->m_SliceNavigationListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_SliceNavigationListener, &QmitkSliceNavigationListener::SliceChanged, this, &QmitkDicomInspectorView::OnSliceChanged);

  auto selection = this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection();
  auto currentSelection = GetInitialSelection(selection);

  if (!currentSelection.isEmpty())
    m_Controls.singleSlot->SetCurrentSelection(currentSelection);
}

void QmitkDicomInspectorView::OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  if (nodes.empty() || nodes.front().IsNull())
  {
    m_SelectedNode = nullptr;
    m_SelectedData = nullptr;
    UpdateData();
    return;
  }

  if (nodes.front() != this->m_SelectedNode)
  {
    // node is selected, create DICOM tag table
    m_SelectedNode = nodes.front();
    m_SelectedData = this->m_SelectedNode->GetData();

    m_SelectedNodeTime.Modified();
    UpdateData();
    OnSliceChanged();
  }
}

void QmitkDicomInspectorView::ValidateAndSetCurrentPosition()
{
  const auto currentSelectedPosition = m_RenderWindowPart->GetSelectedPosition();
  const auto* timeNavigationController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
  const mitk::TimeStepType currentSelectedTimePoint = timeNavigationController->GetSelectedTimePoint();

  if (m_SelectedPosition != currentSelectedPosition
    || m_SelectedTimePoint != currentSelectedTimePoint
    || m_SelectedNodeTime > m_CurrentPositionTime)
  {
    // the current position has been changed, the selected node has been changed since
    // the last position validation or the current time position has been changed -> check position
    m_SelectedPosition = currentSelectedPosition;
    m_SelectedTimePoint = currentSelectedTimePoint;
    m_CurrentPositionTime.Modified();
    m_ValidSelectedPosition = false;

    if (m_SelectedData.IsNull())
    {
      return;
    }

    mitk::BaseGeometry::Pointer geometry = m_SelectedData->GetTimeGeometry()->GetGeometryForTimePoint(m_SelectedTimePoint);

    // check for invalid time step
    if (geometry.IsNull())
    {
      geometry = m_SelectedData->GetTimeGeometry()->GetGeometryForTimeStep(0);
    }

    if (geometry.IsNull())
    {
      return;
    }

    m_ValidSelectedPosition = geometry->IsInside(m_SelectedPosition);
    itk::Index<3> index;
    geometry->WorldToIndex(m_SelectedPosition, index);

    m_CurrentSelectedZSlice = index[2];
  }
}

void QmitkDicomInspectorView::OnSliceChanged()
{
  ValidateAndSetCurrentPosition();

  m_Controls.tableTags->setEnabled(m_ValidSelectedPosition);

  if (m_SelectedNode.IsNotNull())
  {
    RenderTable();
  }
}

void QmitkDicomInspectorView::RenderTable()
{
  assert(nullptr != m_RenderWindowPart);

  const auto timeStep = (m_SelectedData.IsNull()) ? 0 : m_SelectedData->GetTimeGeometry()->TimePointToTimeStep(m_SelectedTimePoint);

  unsigned int rowIndex = 0;
  for (const auto& element : m_Tags)
  {
    QTableWidgetItem* newItem = new QTableWidgetItem(QString::fromStdString(
      element.second.prop->GetValue(timeStep, m_CurrentSelectedZSlice, true, true)));
    m_Controls.tableTags->setItem(rowIndex, 3, newItem);
    ++rowIndex;
  }

  UpdateLabels();
}

void QmitkDicomInspectorView::UpdateData()
{
  QStringList headers;

  m_Controls.tableTags->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

  m_Tags.clear();

  if (m_SelectedData.IsNotNull())
  {
    for (const auto& element : *(m_SelectedData->GetPropertyList()->GetMap()))
    {
      if (element.first.find("DICOM") == 0)
      {
        std::istringstream stream(element.first);
        std::string token;
        std::getline(stream, token, '.'); //drop the DICOM suffix
        std::getline(stream, token, '.'); //group id
        unsigned long dcmgroup = std::stoul(token, nullptr, 16);
        std::getline(stream, token, '.'); //element id
        unsigned long dcmelement = std::stoul(token, nullptr, 16);

        TagInfo info(mitk::DICOMTag(dcmgroup, dcmelement), dynamic_cast<const mitk::DICOMProperty*>(element.second.GetPointer()));

        m_Tags.insert(std::make_pair(element.first, info));
      }
    }
  }

  m_Controls.tableTags->setRowCount(m_Tags.size());

  unsigned int rowIndex = 0;
  for (const auto& element : m_Tags)
  {
    QTableWidgetItem* newItem = new QTableWidgetItem(QString::number(element.second.tag.GetGroup(), 16));
    m_Controls.tableTags->setItem(rowIndex, 0, newItem);
    newItem = new QTableWidgetItem(QString::number(element.second.tag.GetElement(), 16));
    m_Controls.tableTags->setItem(rowIndex, 1, newItem);
    newItem = new QTableWidgetItem(QString::fromStdString(element.second.tag.GetName()));
    m_Controls.tableTags->setItem(rowIndex, 2, newItem);
    newItem = new QTableWidgetItem(QString::fromStdString(element.second.prop->GetValue()));
    m_Controls.tableTags->setItem(rowIndex, 3, newItem);
    ++rowIndex;
  }

  UpdateLabels();
}

void QmitkDicomInspectorView::UpdateLabels()
{
  if (m_SelectedData.IsNull())
  {
    m_Controls.timePointValueLabel->setText(QString(""));
    m_Controls.sliceNumberValueLabel->setText(QString(""));
  }
  else
  {
    const auto timeStep = m_SelectedData->GetTimeGeometry()->TimePointToTimeStep(m_SelectedTimePoint);

    if (m_ValidSelectedPosition)
    {
      m_Controls.timePointValueLabel->setText(QString::number(timeStep) + QStringLiteral("(")+ QString::number(m_SelectedTimePoint/1000.) + QStringLiteral(" [s])"));
      m_Controls.sliceNumberValueLabel->setText(QString::number(m_CurrentSelectedZSlice));
    }
    else
    {
      m_Controls.timePointValueLabel->setText(QString("outside data geometry"));
      m_Controls.sliceNumberValueLabel->setText(QString("outside data geometry"));
    }
  }
}

void QmitkDicomInspectorView::SetAsSelectionListener(bool checked)
{
  if (checked)
  {
    m_SelectionServiceConnector->AddPostSelectionListener(GetSite()->GetWorkbenchWindow()->GetSelectionService());
    connect(m_SelectionServiceConnector.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetCurrentSelection);
  }
  else
  {
    m_SelectionServiceConnector->RemovePostSelectionListener();
    disconnect(m_SelectionServiceConnector.get(), &QmitkSelectionServiceConnector::ServiceSelectionChanged,
      m_Controls.singleSlot, &QmitkSingleNodeSelectionWidget::SetCurrentSelection);
  }
}
