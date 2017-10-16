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
#include "QmitkPatientTableWidget.h"
#include "QmitkControlPointDialog.h"

#include "QmitkCustomVariants.h"

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationException.h>
#include <mitkSemanticTypes.h>
#include <mitkUIDGeneratorBoost.h>

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkExtractSliceFilter.h>
#include <vtkMitkLevelWindowFilter.h>
#include <mitkPlanarFigure.h>
#include <mitkPropertyNameHelper.h>

// vtk
#include <vtkLookupTable.h>

// qt
#include <QInputDialog>
#include <QMessageBox>

QmitkPatientTableWidget::QmitkPatientTableWidget(mitk::DataStorage* dataStorage, QWidget* parent /*=nullptr*/)
  : QmitkSelectionWidget(dataStorage, parent)
{
  Init();
}

QmitkPatientTableWidget::~QmitkPatientTableWidget()
{
  // nothing here
}

void QmitkPatientTableWidget::Init()
{
  // create GUI from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  // create a new model
  m_PatientTableModel = std::make_unique<QmitkPatientTableModel>(m_SemanticRelations, this);

  m_Controls.patientTableView->setModel(m_PatientTableModel.get());
  m_Controls.patientTableView->horizontalHeader()->setHighlightSections(false);
  m_Controls.patientTableView->verticalHeader()->setHighlightSections(false);
  m_Controls.patientTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
  m_Controls.patientTableView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.patientTableView->setContextMenuPolicy(Qt::CustomContextMenu);

  m_ContextMenu = new QMenu(m_Controls.patientTableView);

  SetUpConnections();
}

void QmitkPatientTableWidget::SetUpConnections()
{
  connect(m_PatientTableModel.get(), SIGNAL(ModelUpdated()), SLOT(OnPatientTableModelUpdated()));
  connect(m_Controls.patientTableView, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(OnPatientTableViewContextMenuRequested(const QPoint&)));
  connect(m_Controls.patientTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnPatientTableViewSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

void QmitkPatientTableWidget::Update(const mitk::SemanticTypes::CaseID& caseID)
{
  m_PatientTableModel->Update(caseID);
}

void QmitkPatientTableWidget::SetPixmapOfNode(const mitk::DataNode* dataNode)
{
  QPixmap pixmapFromImage = GetPixmapFromImageNode(dataNode);
  m_PatientTableModel->SetPixmapOfNode(dataNode, &pixmapFromImage);
}

void QmitkPatientTableWidget::DeletePixmapOfNode(const mitk::DataNode* dataNode)
{
  m_PatientTableModel->SetPixmapOfNode(dataNode, nullptr);
}

void QmitkPatientTableWidget::OnPatientTableModelUpdated()
{
  m_Controls.patientTableView->resizeRowsToContents();
  m_Controls.patientTableView->resizeColumnsToContents();
}

void QmitkPatientTableWidget::OnPatientTableViewContextMenuRequested(const QPoint& pos)
{
  QModelIndex selectedIndex = m_Controls.patientTableView->indexAt(pos);
  if (!selectedIndex.isValid())
  {
    return;
  }

  QVariant qvariantDataNode = m_PatientTableModel->data(selectedIndex, Qt::UserRole);
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

void QmitkPatientTableWidget::OnContextMenuSetInformationType()
{
  bool ok = false;
  QString text = QInputDialog::getText(m_Controls.patientTableView, tr("Set information type of selected node"), tr("Information type:"), QLineEdit::Normal, "", &ok);
  if (ok && !text.isEmpty())
  {
    m_SemanticRelations->RemoveInformationTypeFromImage(m_SelectedDataNode);
    m_SemanticRelations->AddInformationTypeToImage(m_SelectedDataNode, text.toStdString());
    m_PatientTableModel->SetPatientData();
  }
}

void QmitkPatientTableWidget::OnContextMenuSetControlPoint()
{
  QmitkControlPointDialog* inputDialog = new QmitkControlPointDialog(m_Controls.patientTableView);
  inputDialog->setWindowTitle("Set control point");
  inputDialog->SetCurrentDate(mitk::GetDICOMDateFromDataNode(m_SelectedDataNode));

  int dialogReturnValue = inputDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  // store the current control point to relink it, if anything goes wrong
  mitk::SemanticTypes::ControlPoint originalControlPoint = m_SemanticRelations->GetControlPointOfData(m_SelectedDataNode);
  // unlink the data, that is about to receive a new date
  // this is needed in order to not extend a single control point, to which the selected node is currently linked
  m_SemanticRelations->UnlinkDataFromControlPoint(m_SelectedDataNode);

  const QDate& userSelectedDate = inputDialog->GetCurrentDate();
  mitk::SemanticTypes::Date date;
  date.UID = mitk::UIDGeneratorBoost::GenerateUID();
  date.year = userSelectedDate.year();
  date.month = userSelectedDate.month();
  date.day = userSelectedDate.day();

  std::vector<mitk::SemanticTypes::ControlPoint> allControlPoints = m_SemanticRelations->GetAllControlPointsOfCase(m_PatientTableModel->GetCurrentCaseID());
  if (!allControlPoints.empty())
  {
    // need to check if an already existing control point fits/contains the user control point
    mitk::SemanticTypes::ControlPoint fittingControlPoint = mitk::FindFittingControlPoint(date, allControlPoints);
    if (!fittingControlPoint.UID.empty())
    {
      try
      {
        // found a fitting control point
        m_SemanticRelations->LinkDataToControlPoint(m_SelectedDataNode, fittingControlPoint, false);
        m_PatientTableModel->SetPatientData();
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The data can not be linked to the fitting control point.";
        try
        {
          // link to the original control point
          m_SemanticRelations->LinkDataToControlPoint(m_SelectedDataNode, originalControlPoint, false);
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
        m_SemanticRelations->OverwriteControlPointAndLinkData(m_SelectedDataNode, extendedControlPoint, false);
        m_PatientTableModel->SetPatientData();
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The extended control point can not be overwritten and the data can not be linked to this control point.";
        try
        {
          // link to the original control point
          m_SemanticRelations->LinkDataToControlPoint(m_SelectedDataNode, originalControlPoint, false);
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
    m_SemanticRelations->AddControlPointAndLinkData(m_SelectedDataNode, controlPointFromUserDate, false);
    m_PatientTableModel->SetPatientData();
  }
  catch (const mitk::SemanticRelationException&)
  {
    MITK_INFO << "The control point can not be added and the data can not be linked to this control point.";
    try
    {
      // link to the original control point
      m_SemanticRelations->LinkDataToControlPoint(m_SelectedDataNode, originalControlPoint, false);
    }
    catch (const mitk::SemanticRelationException&)
    {
      MITK_INFO << "The data can not be linked to its original control point. Inconsistency in the semantic relations storage assumed.";
    }
  }
}

void QmitkPatientTableWidget::OnPatientTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  QModelIndex selectedIndex = m_Controls.patientTableView->currentIndex();
  if (!selectedIndex.isValid())
  {
    return;
  }

  QVariant qvariantDataNode = m_PatientTableModel->data(selectedIndex, Qt::UserRole);
  if (qvariantDataNode.canConvert<mitk::DataNode*>())
  {
    m_SelectedDataNode = qvariantDataNode.value<mitk::DataNode*>();

    emit SelectionChanged(m_SelectedDataNode);
  }
}

QPixmap QmitkPatientTableWidget::GetPixmapFromImageNode(const mitk::DataNode* dataNode) const
{
  if (nullptr == dataNode)
  {
    return QPixmap();
  }

  const mitk::Image* image = static_cast<const mitk::Image*>(dataNode->GetData());
  if (nullptr == image || !image->IsInitialized())
  {
    return QPixmap();
  }

  if (image->GetPixelType().GetNumberOfComponents() != 1) // for now only single component are allowed
  {
    return QPixmap();
  }

  mitk::PlaneGeometry::Pointer sagittalPlaneGeometry = mitk::PlaneGeometry::New();
  int sliceNumber = image->GetDimension(1) / 2;
  sagittalPlaneGeometry->InitializeStandardPlane(image->GetGeometry(), mitk::PlaneGeometry::Sagittal, sliceNumber);

  mitk::ExtractSliceFilter::Pointer extractSliceFilter = mitk::ExtractSliceFilter::New();
  extractSliceFilter->SetInput(image);
  extractSliceFilter->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_CUBIC);
  extractSliceFilter->SetResliceTransformByGeometry(image->GetGeometry());
  extractSliceFilter->SetWorldGeometry(sagittalPlaneGeometry);
  extractSliceFilter->SetOutputDimensionality(2);
  extractSliceFilter->SetVtkOutputRequest(true);
  extractSliceFilter->Update();

  /*
  mitk::Vector3D spacing;
  mitk::FillVector3D(spacing,1.0,1.0,1.0);
  slice->GetGeometry()->SetSpacing(spacing);
  // save image slice
  mitk::IOUtil::SaveImage( slice, d->m_Path + fileName + ".png" );
  */

  vtkImageData* imageData = extractSliceFilter->GetVtkOutput();

  mitk::LevelWindow levelWindow;
  dataNode->GetLevelWindow(levelWindow);
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetRange(levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound());
  lookupTable->SetSaturationRange(0.0, 0.0);
  lookupTable->SetValueRange(0.0, 1.0);
  lookupTable->SetHueRange(0.0, 0.0);
  lookupTable->SetRampToLinear();

  vtkSmartPointer<vtkMitkLevelWindowFilter> levelWindowFilter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();
  levelWindowFilter->SetLookupTable(lookupTable);
  levelWindowFilter->SetInputData(imageData);
  levelWindowFilter->SetMinOpacity(0.0);
  levelWindowFilter->SetMaxOpacity(1.0);
  int dims[3];
  imageData->GetDimensions(dims);
  double clippingBounds[] = { 0.0, static_cast<double>(dims[0]), 0.0, static_cast<double>(dims[1]) };
  levelWindowFilter->SetClippingBounds(clippingBounds);
  levelWindowFilter->Update();
  imageData = levelWindowFilter->GetOutput();

  QImage thumbnailImage(reinterpret_cast<const unsigned char*>(imageData->GetScalarPointer()), dims[0], dims[1], QImage::Format_ARGB32);

  thumbnailImage = thumbnailImage.rgbSwapped().mirrored(false, true);
  return QPixmap::fromImage(thumbnailImage);
}
