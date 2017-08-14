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

// semantic relation plugin
#include "QmitkSemanticRelationsView.h"

#include "QmitkCustomVariants.h"

// semantic relations (UI) module
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationsTestHelper.h>
#include "mitkSemanticRelationException.h"
#include <mitkSemanticTypes.h>
#include <mitkUIDGeneratorBoost.h>
#include <QmitkControlPointDialog.h>

// blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// qt
#include <QInputDialog>
#include <QMessageBox>

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkExtractSliceFilter.h>
#include <vtkMitkLevelWindowFilter.h>
#include <mitkPlanarFigure.h>

// vtk
#include <vtkLookupTable.h>

const std::string QmitkSemanticRelationsView::VIEW_ID = "org.mitk.views.semanticrelations";

void QmitkSemanticRelationsView::SetFocus()
{
  // nothing here
}

void QmitkSemanticRelationsView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  m_SemanticRelations = std::make_unique<mitk::SemanticRelations>(GetDataStorage());

  // create a new model
  m_SemanticRelationsDataModel = std::make_unique<QmitkSemanticRelationsDataModel>(this);
  m_SemanticRelationsDataModel->SetSemanticRelations(m_SemanticRelations.get());
  m_SemanticRelationsDataModel->SetDataStorage(GetDataStorage());

  m_Controls.patientDataTableView->setModel(m_SemanticRelationsDataModel.get());
  m_Controls.patientDataTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Controls.patientDataTableView->horizontalHeader()->setHighlightSections(false);
  m_Controls.patientDataTableView->verticalHeader()->setHighlightSections(false);
  m_Controls.patientDataTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
  m_Controls.patientDataTableView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.patientDataTableView->setContextMenuPolicy(Qt::CustomContextMenu);

  m_ContextMenu = new QMenu(m_Controls.patientDataTableView);

  m_Controls.patientInfoWidget->hide();

  // setup connections
  connect(m_Controls.caseIDComboBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnCaseIDSelectionChanged(const QString&)));
  connect(m_SemanticRelationsDataModel.get(), SIGNAL(ModelReset()), SLOT(OnTableViewDataChanged()));
  connect(m_Controls.patientDataTableView, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(OnTableViewContextMenuRequested(const QPoint&)));
  connect(m_Controls.patientDataTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnTableViewSelectionChanged(const QItemSelection&, const QItemSelection&)));
  connect(m_Controls.patientDataTableView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnTableViewItemClicked(const QModelIndex&)));
}

void QmitkSemanticRelationsView::NodeAdded(const mitk::DataNode* node)
{
  bool helperObject = false;
  node->GetBoolProperty("helper object", helperObject);
  if (helperObject)
  { 
    return;
  }

  // not a helper object; add the image and generate a pixmap of the DICOM image
  SemanticRelationsTestHelper::AddImageInstance(node, *m_SemanticRelations);
  const SemanticTypes::CaseID caseID = DICOMHelper::GetCaseIDFromDataNode(node);
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (-1 == foundIndex)
  {
    // add the caseID to the combo box, if it is not already contained
    m_Controls.caseIDComboBox->addItem(QString::fromStdString(caseID));
  }

  QPixmap pixmapFromImage = GetPixmapFromImageNode(node);
  m_SemanticRelationsDataModel->SetPixmapOfNode(node, pixmapFromImage);
}

void QmitkSemanticRelationsView::NodeRemoved(const mitk::DataNode* node)
{
  bool helperObject = false;
  node->GetBoolProperty("helper object", helperObject);
  if (helperObject)
  {
    return;
  }

  // not a helper object; remove the image and the pixmap
  SemanticTypes::ControlPoint controlPoint = m_SemanticRelations->GetControlPointOfData(node);
  SemanticTypes::CaseID caseID = DICOMHelper::GetCaseIDFromDataNode(node);
  if (m_SemanticRelations->InstanceExists(caseID, controlPoint))
  {
    m_SemanticRelations->UnlinkDataFromControlPoint(node, controlPoint);
  }
  
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (-1 != foundIndex)
  {
    // remove the caseID
    m_Controls.caseIDComboBox->removeItem(foundIndex);
  }
}

void QmitkSemanticRelationsView::OnCaseIDSelectionChanged(const QString& caseID)
{
  m_SemanticRelationsDataModel->SetCurrentCaseID(caseID.toStdString());
}

void QmitkSemanticRelationsView::OnTableViewDataChanged()
{
  m_Controls.patientDataTableView->resizeRowsToContents();
  m_Controls.patientDataTableView->resizeColumnsToContents();
}

void QmitkSemanticRelationsView::OnTableViewContextMenuRequested(const QPoint& pos)
{
  QModelIndex selectedIndex = m_Controls.patientDataTableView->indexAt(pos);
  if (!selectedIndex.isValid())
  {
    return;
  }

  QVariant qvariantDataNode = m_SemanticRelationsDataModel->data(selectedIndex, Qt::UserRole);
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

void QmitkSemanticRelationsView::OnContextMenuSetInformationType()
{
  bool ok = false;
  QString text = QInputDialog::getText(m_Controls.patientDataTableView, tr("Set information type of selected node"), tr("Information type:"), QLineEdit::Normal, "", &ok);
  if (ok && !text.isEmpty())
  {
    m_SemanticRelations->RemoveInformationTypeFromImage(m_SelectedDataNode, m_SemanticRelations->GetInformationTypeOfImage(m_SelectedDataNode));
    m_SemanticRelations->AddInformationTypeToImage(m_SelectedDataNode, text.toStdString());
    m_SemanticRelationsDataModel->DataChanged();
  }
}

void QmitkSemanticRelationsView::OnContextMenuSetControlPoint()
{
  QmitkControlPointDialog* inputDialog = new QmitkControlPointDialog(m_Controls.patientDataTableView);
  inputDialog->setWindowTitle("Set control point");
  inputDialog->SetCurrentDate(DICOMHelper::GetDateFromDataNode(m_SelectedDataNode));

  int dialogReturnValue = inputDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  // store the current control point to relink it, if anything goes wrong
  SemanticTypes::ControlPoint originalControlPoint = m_SemanticRelations->GetControlPointOfData(m_SelectedDataNode);
  // unlink the data, that is about to receive a new date
  // this is needed in order to not extend a single control point, to which the selected node is currently linked
  m_SemanticRelations->UnlinkDataFromControlPoint(m_SelectedDataNode, originalControlPoint);

  const QDate& userSelectedDate = inputDialog->GetCurrentDate();
  SemanticTypes::Date date;
  date.UID = UIDGeneratorBoost::GenerateUID();
  date.year = userSelectedDate.year();
  date.month = userSelectedDate.month();
  date.day = userSelectedDate.day();

  std::vector<SemanticTypes::ControlPoint> allControlPoints = m_SemanticRelations->GetAllControlPointsOfCase(m_SemanticRelationsDataModel->GetCurrentCaseID());
  if (!allControlPoints.empty())
  {
    // need to check if an already existing control point fits/contains the user control point
    SemanticTypes::ControlPoint fittingControlPoint = ControlPointManager::FindFittingControlPoint(date, allControlPoints);
    if (!fittingControlPoint.UID.empty())
    {
      try
      {
        // found a fitting control point
        m_SemanticRelations->LinkDataToControlPoint(m_SelectedDataNode, fittingControlPoint, false);
        m_SemanticRelationsDataModel->DataChanged();
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The data can not be linked to the fitting control point.";
        try
        {
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
    SemanticTypes::ControlPoint extendedControlPoint = ControlPointManager::ExtendClosestControlPoint(date, allControlPoints);
    if (!extendedControlPoint.UID.empty())
    {
      try
      {
        // found and extended a close control point
        m_SemanticRelations->OverwriteControlPointAndLinkData(m_SelectedDataNode, extendedControlPoint, false);
        m_SemanticRelationsDataModel->DataChanged();
      }
      catch (const mitk::SemanticRelationException&)
      {
        MITK_INFO << "The extended control point can not be overwritten and the data can not be linked to this control point.";
        try
        {
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
  SemanticTypes::ControlPoint controlPointFromUserDate = ControlPointManager::GenerateControlPoint(date);
  try
  {
    m_SemanticRelations->AddControlPointAndLinkData(m_SelectedDataNode, controlPointFromUserDate, false);
    m_SemanticRelationsDataModel->DataChanged();
  }
  catch (const mitk::SemanticRelationException&)
  {
    MITK_INFO << "The control point can not be added and the data can not be linked to this control point.";
    try
    {
      m_SemanticRelations->LinkDataToControlPoint(m_SelectedDataNode, originalControlPoint, false);
    }
    catch (const mitk::SemanticRelationException&)
    {
      MITK_INFO << "The data can not be linked to its original control point. Inconsistency in the semantic relations storage assumed.";
    }
  }
}

void QmitkSemanticRelationsView::OnTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  QModelIndex selectedIndex = m_Controls.patientDataTableView->currentIndex();
  if (!selectedIndex.isValid())
  {
    return;
  }

  QVariant qvariantDataNode = m_SemanticRelationsDataModel->data(selectedIndex, Qt::UserRole);
  if (qvariantDataNode.canConvert<mitk::DataNode*>())
  {
    m_SelectedDataNode = qvariantDataNode.value<mitk::DataNode*>();

    m_Controls.patientInfoWidget->SetPatientInfo(m_SelectedDataNode);
    m_Controls.patientInfoWidget->show();
  }
}

void QmitkSemanticRelationsView::OnTableViewItemClicked(const QModelIndex& selectedIndex)
{
  if (!selectedIndex.isValid())
  {
    return;
  }

  // clicked is called each time the item is selected, regardless of the previous selection
}

QPixmap QmitkSemanticRelationsView::GetPixmapFromImageNode(const mitk::DataNode* dataNode) const
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
  double clippingBounds[] = { 0.0, dims[0], 0.0, dims[1] };
  levelWindowFilter->SetClippingBounds(clippingBounds);
  levelWindowFilter->Update();
  imageData = levelWindowFilter->GetOutput();

  QImage thumbnailImage(reinterpret_cast<const unsigned char*>(imageData->GetScalarPointer()), dims[0], dims[1], QImage::Format_ARGB32);

  thumbnailImage = thumbnailImage.rgbSwapped().mirrored(false, true);
  return QPixmap::fromImage(thumbnailImage);
}