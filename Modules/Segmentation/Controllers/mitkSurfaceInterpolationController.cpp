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

#include "mitkSurfaceInterpolationController.h"
#include "mitkMemoryUtilities.h"

mitk::SurfaceInterpolationController::SurfaceInterpolationController()
  :m_SelectedSegmentation(0)
{
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();

  m_ReduceFilter->SetUseProgressBar(false);
  m_NormalsFilter->SetUseProgressBar(false);
  m_InterpolateSurfaceFilter->SetUseProgressBar(false);

  m_Contours = Surface::New();

  m_PolyData = vtkSmartPointer<vtkPolyData>::New();
  m_PolyData->SetPoints(vtkPoints::New());

  m_InterpolationResult = 0;
  m_CurrentNumberOfReducedContours = 0;
}

mitk::SurfaceInterpolationController::~SurfaceInterpolationController()
{

  for (ContourListMap::iterator it = m_MapOfContourLists.begin(); it != m_MapOfContourLists.end(); it++)
  {
      for (unsigned int j = 0; j < m_MapOfContourLists[(*it).first].size(); ++j)
      {
          delete(m_MapOfContourLists[(*it).first].at(j).position);
      }
      m_MapOfContourLists.erase(it);
  }
}

mitk::SurfaceInterpolationController* mitk::SurfaceInterpolationController::GetInstance()
{
  static mitk::SurfaceInterpolationController* m_Instance;

  if ( m_Instance == 0)
  {
    m_Instance = new SurfaceInterpolationController();
  }

  return m_Instance;
}

void mitk::SurfaceInterpolationController::AddNewContour (mitk::Surface::Pointer newContour ,RestorePlanePositionOperation* op)
{
  AffineTransform3D::Pointer transform = AffineTransform3D::New();
  transform = op->GetTransform();

  mitk::Vector3D direction = op->GetDirectionVector();
  int pos (-1);

  for (unsigned int i = 0; i < m_MapOfContourLists[m_SelectedSegmentation].size(); i++)
  {
      itk::Matrix<float> diffM = transform->GetMatrix()-m_MapOfContourLists[m_SelectedSegmentation].at(i).position->GetTransform()->GetMatrix();
      bool isSameMatrix(true);
      for (unsigned int j = 0; j < 3; j++)
      {
        if (fabs(diffM[j][0]) > 0.0001 && fabs(diffM[j][1]) > 0.0001 && fabs(diffM[j][2]) > 0.0001)
        {
          isSameMatrix = false;
          break;
        }
      }
      itk::Vector<float> diffV = m_MapOfContourLists[m_SelectedSegmentation].at(i).position->GetTransform()->GetOffset()-transform->GetOffset();
      if ( isSameMatrix && m_MapOfContourLists[m_SelectedSegmentation].at(i).position->GetPos() == op->GetPos() && (fabs(diffV[0]) < 0.0001 && fabs(diffV[1]) < 0.0001 && fabs(diffV[2]) < 0.0001) )
      {
        pos = i;
        break;
      }

  }

  if (pos == -1)
  {
    //MITK_INFO<<"New Contour";
    mitk::RestorePlanePositionOperation* newOp = new mitk::RestorePlanePositionOperation (OpRESTOREPLANEPOSITION, op->GetWidth(),
      op->GetHeight(), op->GetSpacing(), op->GetPos(), direction, transform);
    ContourPositionPair newData;
    newData.contour = newContour;
    newData.position = newOp;

    m_ReduceFilter->SetInput(m_MapOfContourLists[m_SelectedSegmentation].size(), newContour);
    m_MapOfContourLists[m_SelectedSegmentation].push_back(newData);
  }
  else
  {
    //MITK_INFO<<"Modified Contour";
    m_MapOfContourLists[m_SelectedSegmentation].at(pos).contour = newContour;
    m_ReduceFilter->SetInput(pos, newContour);
  }

  m_ReduceFilter->Update();
  m_CurrentNumberOfReducedContours = m_ReduceFilter->GetNumberOfOutputs();

  for (unsigned int i = 0; i < m_CurrentNumberOfReducedContours; i++)
  {
    m_NormalsFilter->SetInput(i, m_ReduceFilter->GetOutput(i));
    m_InterpolateSurfaceFilter->SetInput(i, m_NormalsFilter->GetOutput(i));
  }

  this->Modified();
}

void mitk::SurfaceInterpolationController::Interpolate()
{
  if (m_CurrentNumberOfReducedContours< 2)
    return;

  //Setting up progress bar
   /*
    * Removed due to bug 12441. ProgressBar messes around with Qt event queue which is fatal for segmentation
    */
  //mitk::ProgressBar::GetInstance()->AddStepsToDo(8);

  m_InterpolateSurfaceFilter->Update();

  Image::Pointer distanceImage = m_InterpolateSurfaceFilter->GetOutput();

  vtkSmartPointer<vtkMarchingCubes> mcFilter = vtkMarchingCubes::New();
  mcFilter->SetInput(distanceImage->GetVtkImageData());
  mcFilter->SetValue(0,0);
  mcFilter->Update();

  m_InterpolationResult = 0;
  m_InterpolationResult = mitk::Surface::New();
  m_InterpolationResult->SetVtkPolyData(mcFilter->GetOutput());
  m_InterpolationResult->GetGeometry()->SetOrigin(distanceImage->GetGeometry()->GetOrigin());

  vtkSmartPointer<vtkAppendPolyData> polyDataAppender = vtkSmartPointer<vtkAppendPolyData>::New();
  for (unsigned int i = 0; i < m_ReduceFilter->GetNumberOfOutputs(); i++)
  {
    polyDataAppender->AddInput(m_ReduceFilter->GetOutput(i)->GetVtkPolyData());
  }
  polyDataAppender->Update();
  m_Contours->SetVtkPolyData(polyDataAppender->GetOutput());

  //Last progress step
  /*
   * Removed due to bug 12441. ProgressBar messes around with Qt event queue which is fatal for segmentation
   */
  //mitk::ProgressBar::GetInstance()->Progress(8);

  m_InterpolationResult->DisconnectPipeline();
}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::GetInterpolationResult()
{
    return m_InterpolationResult;
}

mitk::Surface* mitk::SurfaceInterpolationController::GetContoursAsSurface()
{
  return m_Contours;
}

void mitk::SurfaceInterpolationController::SetDataStorage(DataStorage &ds)
{
  m_DataStorage = &ds;
}

void mitk::SurfaceInterpolationController::SetMinSpacing(double minSpacing)
{
  m_ReduceFilter->SetMinSpacing(minSpacing);
}

void mitk::SurfaceInterpolationController::SetMaxSpacing(double maxSpacing)
{
  m_ReduceFilter->SetMaxSpacing(maxSpacing);
  m_NormalsFilter->SetMaxSpacing(maxSpacing);
}

void mitk::SurfaceInterpolationController::SetDistanceImageVolume(unsigned int distImgVolume)
{
  m_InterpolateSurfaceFilter->SetDistanceImageVolume(distImgVolume);
}

void mitk::SurfaceInterpolationController::SetSegmentationImage(Image* workingImage)
{
  m_NormalsFilter->SetSegmentationBinaryImage(workingImage);
}

mitk::Image* mitk::SurfaceInterpolationController::GetImage()
{
  return m_InterpolateSurfaceFilter->GetOutput();
}

double mitk::SurfaceInterpolationController::EstimatePortionOfNeededMemory()
{
  double numberOfPointsAfterReduction = m_ReduceFilter->GetNumberOfPointsAfterReduction()*3;
  double sizeOfPoints = pow(numberOfPointsAfterReduction,2)*sizeof(double);
  double totalMem = mitk::MemoryUtilities::GetTotalSizeOfPhysicalRam();
  double percentage = sizeOfPoints/totalMem;
  return percentage;
}

void mitk::SurfaceInterpolationController::SetCurrentSegmentationInterpolationList(mitk::Image* segmentation)
{
  if (segmentation == m_SelectedSegmentation)
    return;

  if (segmentation == 0)
    return;

  ContourListMap::iterator it = m_MapOfContourLists.find(segmentation);

  m_SelectedSegmentation = segmentation;

  m_ReduceFilter->Reset();
  m_NormalsFilter->Reset();
  m_InterpolateSurfaceFilter->Reset();

  if (it == m_MapOfContourLists.end())
  {
    ContourPositionPairList newList;
    m_MapOfContourLists.insert(std::pair<mitk::Image*, ContourPositionPairList>(segmentation, newList));
    m_InterpolationResult = 0;
    m_CurrentNumberOfReducedContours = 0;
  }
  else
  {
    for (unsigned int i = 0; i < m_MapOfContourLists[m_SelectedSegmentation].size(); i++)
    {
      m_ReduceFilter->SetInput(i, m_MapOfContourLists[m_SelectedSegmentation].at(i).contour);
    }

    m_ReduceFilter->Update();

    m_CurrentNumberOfReducedContours = m_ReduceFilter->GetNumberOfOutputs();

    for (unsigned int i = 0; i < m_CurrentNumberOfReducedContours; i++)
    {
      m_NormalsFilter->SetInput(i, m_ReduceFilter->GetOutput(i));
      m_InterpolateSurfaceFilter->SetInput(i, m_NormalsFilter->GetOutput(i));
    }
  }
  Modified();
}

void mitk::SurfaceInterpolationController::RemoveSegmentationFromContourList(mitk::Image *segmentation)
{
  if (segmentation != 0)
  {
    m_MapOfContourLists.erase(segmentation);
  }
}
