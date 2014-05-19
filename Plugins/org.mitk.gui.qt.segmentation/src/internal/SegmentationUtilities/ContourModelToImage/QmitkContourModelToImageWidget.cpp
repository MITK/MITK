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
#include <mitkVtkImageOverwrite.h>

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
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;

  mitk::DataNode::Pointer segmentationNode = dataSelectionWidget->GetSelection(0);

  if (segmentationNode.IsNull())
  {
    MITK_ERROR<<"Error writing contours into binary image! No image selected!";
    return;
  }

  mitk::Image::Pointer segmentationImage = static_cast<mitk::Image*>(segmentationNode->GetData());

  if (segmentationImage.IsNull())
  {
    MITK_ERROR<<"Error writing contours into binary image! Invalid image data selected!";
    return;
  }

  unsigned int timeStep = this->GetTimeNavigationController()->GetTime()->GetPos();

  m_SegmentationImage = segmentationImage;
  m_SegmentationImageGeometry = segmentationImage->GetTimeGeometry()->GetGeometryForTimeStep(timeStep);

  mitk::ContourModelSet::Pointer contourSet;
  mitk::DataNode::Pointer contourNode = dataSelectionWidget->GetSelection(1);
  mitk::ContourModel::Pointer contour = dynamic_cast<mitk::ContourModel*>(contourNode->GetData());
  if (contour.IsNotNull())
  {
    contourSet = mitk::ContourModelSet::New();
    contourSet->AddContourModel(contour);
  }
  else
  {
    contourSet = static_cast<mitk::ContourModelSet*>(contourNode->GetData());
    if (contourSet.IsNull())
    {
      MITK_ERROR<<"Error writing contours into binary image! Invalid contour data selected!";
      return;
    }
  }

  //Disable Buttons during calculation and initialize Progressbar
  this->EnableButtons(false);
  unsigned int num_contours = contourSet->GetContourModelList()->size();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(num_contours);
  mitk::ProgressBar::GetInstance()->Progress();

  // Do actual filling
  mitk::ContourModel::Pointer projectedContour;
  mitk::ContourModelSet::ContourModelSetIterator it = contourSet->Begin();
  mitk::ContourModelSet::ContourModelSetIterator end = contourSet->End();
  while (it != end)
  {
    mitk::ContourModel* contour = it->GetPointer();

    //Check Controls

    // 1. Get corresponding image slice
    mitk::Image::Pointer slice = this->GetSliceForContour(contour, timeStep);

    // 2. Fill contour into slice
    projectedContour = mitk::ContourModelUtils::ProjectContourTo2DSlice(slice, contour, true, false);
    mitk::ContourModelUtils::FillContourInSlice(projectedContour, slice);
//    mitk::DataNode::Pointer filled = mitk::DataNode::New();
//    filled->SetName("FILLED");
//    filled->SetData(slice);
//    dataSelectionWidget->GetDataStorage()->Add(filled);

    // 3. Write slice back into image volume
    if (slice.IsNotNull())
    {
      this->WriteBackSlice(slice, timeStep);
    }
    else
    {
      MITK_ERROR<<"Contour to image failed!";
    }
    mitk::ProgressBar::GetInstance()->Progress();
    ++it;
  }

  if(segmentationNode.IsNull() || contourNode.IsNull() )
  {
    MITK_ERROR << "Selection does not contain valid data";
    QMessageBox::information( this, "Contour To Image",
                              "Selection does not contain valid data, please select a binary image and a contour(set)", QMessageBox::Ok );
    m_Controls.btnProcess->setEnabled(false);
    return;
  }
  this->EnableButtons();
}

mitk::Image::Pointer QmitkContourModelToImageWidget::GetSliceForContour(mitk::ContourModel::Pointer contour, unsigned int timestep)
{
  mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
  mitk::Point3D point3D, tempPoint;
  mitk::Vector3D normal;

  mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New();
  mitk::Image::Pointer slice;

  int sliceIndex;
  bool isFrontside = true;
  bool isRotated = false;

  point3D = contour->GetVertexAt(0)->Coordinates;
  tempPoint = contour->GetVertexAt(contour->GetNumberOfVertices()*0.5)->Coordinates;
  mitk::Vector3D vec = point3D - tempPoint;
  vec.Normalize();
//  MITK_INFO<<setprecision(20)<<"VEC: ["<<vec[0]<<","<<vec[1]<<","<<vec[2]<<"] MATRIX: "<<m_SegmentationImageGeometry->GetIndexToWorldTransform()->GetMatrix();
  m_SegmentationImageGeometry->WorldToIndex(point3D, point3D);
  mitk::PlaneGeometry::PlaneOrientation orientation;
  if (mitk::Equal(vec[0], 0))
  {
    orientation = mitk::PlaneGeometry::Sagittal;
    sliceIndex = point3D[0];
  }
  else if (mitk::Equal(vec[1], 0))
  {
    orientation = mitk::PlaneGeometry::Frontal;
    sliceIndex = point3D[1];
  }
  else if (mitk::Equal(vec[2], 0))
  {
    orientation = mitk::PlaneGeometry::Axial;
    sliceIndex = point3D[2];
  }
  else
  {
    // Maybe rotate geometry to extract slice?
    MITK_ERROR<<"Cannot detect correct slice number! Only axial, sagittal and frontal oriented contours are supported!";
    return 0;
  }

  plane->InitializeStandardPlane(m_SegmentationImageGeometry, orientation, sliceIndex, isFrontside, isRotated);
  point3D = plane->GetOrigin();
  normal = plane->GetNormal();
  normal.Normalize();
  point3D += normal * 0.5;//pixelspacing is 1, so half the spacing is 0.5
  plane->SetOrigin(point3D);

  m_CurrentPlane = plane;

  // 2. fill into it
  extractor->SetInput( m_SegmentationImage );
//    extractor->SetTimeStep( timeStep );
  extractor->SetWorldGeometry( plane );
  extractor->SetVtkOutputRequest(false);
  extractor->SetResliceTransformByGeometry( m_SegmentationImageGeometry );

  extractor->Modified();
  extractor->Update();

  slice = extractor->GetOutput();
  return slice;
}

void QmitkContourModelToImageWidget::WriteBackSlice(mitk::Image::Pointer slice, unsigned int timestep)
{
  //Make sure that for reslicing and overwriting the same alogrithm is used. We can specify the mode of the vtk reslicer
  vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

  //Set the slice as 'input'
  reslice->SetInputSlice(slice->GetVtkImageData());

  //set overwrite mode to true to write back to the image volume
  reslice->SetOverwriteMode(true);
  reslice->Modified();

  mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslice);
  extractor->SetInput( m_SegmentationImage );
  extractor->SetTimeStep( timestep );
  extractor->SetWorldGeometry( m_CurrentPlane );
  extractor->SetVtkOutputRequest(true);
  extractor->SetResliceTransformByGeometry( m_SegmentationImage->GetGeometry( timestep ) );

  extractor->Modified();
  extractor->Update();

  m_SegmentationImage->Modified();
  m_SegmentationImage->GetVtkImageData()->Modified();
}
