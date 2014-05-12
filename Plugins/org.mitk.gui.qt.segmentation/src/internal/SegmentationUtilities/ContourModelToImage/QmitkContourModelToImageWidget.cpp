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

#include "QmitkContourModelToImageWidget.h"
#include "mitkImage.h"
#include "../../Common/QmitkDataSelectionWidget.h"

#include <mitkContourModelUtils.h>
#include <mitkContourModelSet.h>
#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkExtractSliceFilter.h>
#include <mitkProgressBar.h>
#include <mitkSegTool2D.h>
#include <mitkSliceNavigationController.h>

#include <qmessagebox.h>

static const char* const HelpText = "Select a binary image and a contour(set)";

QmitkContourModelToImageWidget::QmitkContourModelToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SegmentationPredicate);
  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::ContourModelPredicate);
  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  this->EnableButtons(false);

  connect (m_Controls.btnProcess, SIGNAL(pressed()), this, SLOT(OnProcessPressed()));

  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)),
          this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));

  if( m_Controls.dataSelectionWidget->GetSelection(0).IsNotNull() &&
      m_Controls.dataSelectionWidget->GetSelection(1).IsNotNull() )
  {
    this->OnSelectionChanged( 0, m_Controls.dataSelectionWidget->GetSelection(0));
  }
}

QmitkContourModelToImageWidget::~QmitkContourModelToImageWidget()
{
}

void QmitkContourModelToImageWidget::OnSelectionChanged(unsigned int index, const mitk::DataNode* selection)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node0 = dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer node1 = dataSelectionWidget->GetSelection(1);

  if (node0.IsNull() || node1.IsNull() )
  {
    this->EnableButtons(false);
  }
  else
  {
    this->SelectionControl(index, selection);
  }
}

void QmitkContourModelToImageWidget::SelectionControl(unsigned int index, const mitk::DataNode* /*selection*/)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(index);

  //TODO check if contours are inside the image!

  dataSelectionWidget->SetHelpText("");
  this->EnableButtons();
}

void QmitkContourModelToImageWidget::EnableButtons(bool enable)
{
  m_Controls.btnProcess->setEnabled(enable);
}

void QmitkContourModelToImageWidget::OnProcessPressed()
{
  //Disable Buttons during calculation and initialize Progressbar
  this->EnableButtons(false);
  mitk::ProgressBar::GetInstance()->AddStepsToDo(4);
  mitk::ProgressBar::GetInstance()->Progress();

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;

  //create result image, get mask node and reference image
  mitk::Image::Pointer resultImage(0);
  mitk::DataNode::Pointer segmentationNode = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer segmentationImage = static_cast<mitk::Image*>(segmentationNode->GetData());

  //TODO Use one ContourModelSet!!!
  //TODO Extract logic to somewhere
  std::vector<mitk::ContourModel::Pointer> contourList;
  mitk::DataNode::Pointer contourNode = dataSelectionWidget->GetSelection(1);
  mitk::ContourModel::Pointer contour = static_cast<mitk::ContourModel*>(contourNode->GetData());
  if (contour.IsNotNull())
  {
    contourList.push_back(contour);
  }
  else
  {
    // TODO move to utils!
    mitk::ContourModelSet::Pointer contourSet = static_cast<mitk::ContourModelSet*>(contourNode->GetData());
    mitk::ContourModelSet::ContourModelSetIterator it = contourSet->Begin();
    while (it != contourSet->End())
    {
      contourList.push_back(*it);
      ++it;
    }
  }

  // Do actual filling
  mitk::Geometry3D::Pointer segmentationGeometry = segmentationImage->GetGeometry();
  mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
  mitk::Point3D point3D;
  mitk::Vector3D normal;

  mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New();
  mitk::Image::Pointer slice;
  mitk::ContourModel::Pointer projectedContour;

  unsigned int timeStep = this->GetTimeNavigationController()->GetTime()->GetPos();
  int sliceIndex;
  bool isFrontside = true;
  bool isRotated = false;

  foreach (mitk::ContourModel::Pointer contour, contourList)
  {
    //Check Controls
    // BUG save project with contours not working!!
//    contour->GetGeometry()->Print(std::cout);

    // 1. construct geometry -> extract slice
    // TODO Detect contour orientation
    point3D = contour->GetVertexAt(0)->Coordinates;
    segmentationGeometry->WorldToIndex(point3D, point3D);
    plane->InitializeStandardPlane(segmentationGeometry, mitk::PlaneGeometry::Axial, point3D[2], isFrontside, isRotated);
    point3D = plane->GetOrigin();
    normal = plane->GetNormal();
    normal.Normalize();
    point3D += normal * 0.5;//pixelspacing is 1, so half the spacing is 0.5
    plane->SetOrigin(point3D);

    // 2. fill into it
    extractor->SetInput( segmentationImage );
//    extractor->SetTimeStep( timeStep );
    extractor->SetWorldGeometry( plane );
    extractor->SetVtkOutputRequest(false);
    extractor->SetResliceTransformByGeometry( segmentationImage->GetTimeGeometry()->GetGeometryForTimeStep( timeStep ) );

    extractor->Modified();
    extractor->Update();

    slice = extractor->GetOutput();
    projectedContour = mitk::ContourModelUtils::ProjectContourTo2DSlice(slice, contour, true, false);
    mitk::ContourModelUtils::FillContourInSlice(projectedContour, slice);
    mitk::DataNode::Pointer filled = mitk::DataNode::New();
    filled->SetName("FILLED");
    filled->SetData(slice);
    dataSelectionWidget->GetDataStorage()->Add(filled);

  }

  if(segmentationNode.IsNull() || contourNode.IsNull() )
  {
    MITK_ERROR << "Selection does not contain valid data";
    QMessageBox::information( this, "Contour To Image",
                              "Selection does not contain valid data, please select a binary image and a contour(set)", QMessageBox::Ok );
    m_Controls.btnProcess->setEnabled(false);
    return;
  }

  //Do Image-Masking
  mitk::ProgressBar::GetInstance()->Progress();

  // Do contour filling

  mitk::ProgressBar::GetInstance()->Progress();

  if( resultImage.IsNull() )
  {
    MITK_ERROR << "Contour To Image failed";
    QMessageBox::information( this, "Contour To Image", "Failed to write contours into image. For more information please see logging window.", QMessageBox::Ok );
    this->EnableButtons();
    mitk::ProgressBar::GetInstance()->Progress(4);
    return;
  }

  //Add result to data storage
  this->AddToDataStorage(
        dataSelectionWidget->GetDataStorage(),
        resultImage,
        dataSelectionWidget->GetSelection(0)->GetName() + "_" + dataSelectionWidget->GetSelection(1)->GetName(),
        dataSelectionWidget->GetSelection(0));

  this->EnableButtons();

  mitk::ProgressBar::GetInstance()->Progress();
}


void QmitkContourModelToImageWidget::AddToDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::Image::Pointer segmentation, const std::string& name, mitk::DataNode::Pointer parent )
{
  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();

  dataNode->SetName(name);
  dataNode->SetData(segmentation);

  dataStorage->Add(dataNode, parent);
}
