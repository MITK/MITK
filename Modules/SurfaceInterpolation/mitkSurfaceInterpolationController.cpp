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
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"

#include "mitkImageToSurfaceFilter.h"


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
  //Removing all observers
  std::map<mitk::Image*, unsigned long>::iterator dataIter = m_SegmentationObserverTags.begin();
  for (; dataIter != m_SegmentationObserverTags.end(); ++dataIter )
  {
    (*dataIter).first->RemoveObserver( (*dataIter).second );
  }
  m_SegmentationObserverTags.clear();
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

void mitk::SurfaceInterpolationController::AddNewContour (mitk::Surface::Pointer newContour, PlaneGeometry::Pointer plane)
{
  int pos (-1);

  for (unsigned int i = 0; i < m_ListOfInterpolationSessions[m_SelectedSegmentation].size(); i++)
  {
    mitk::PlaneGeometry::Pointer planeFromList = m_ListOfInterpolationSessions[m_SelectedSegmentation].at(i).plane;
    if ( mitk::Equal(*plane, *planeFromList, mitk::eps, false) )
    {
      pos = i;
      break;
    }
  }

  //Don't save a new empty contour
  if (pos == -1 && newContour->GetVtkPolyData()->GetNumberOfPoints() > 0)
  {
    ContourPositionPair newData;
    newData.contour = newContour;
    newData.plane = plane;

    m_ReduceFilter->SetInput(m_ListOfInterpolationSessions[m_SelectedSegmentation].size(), newContour);
    m_ListOfInterpolationSessions[m_SelectedSegmentation].push_back(newData);
  }
  else if (pos != -1 && newContour->GetVtkPolyData()->GetNumberOfPoints() > 0)
  {
    m_ListOfInterpolationSessions[m_SelectedSegmentation].at(pos).contour = newContour;
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

const mitk::Surface* mitk::SurfaceInterpolationController::GetContour(mitk::PlaneGeometry::Pointer plane)
{
  ContourPositionPairList contourList = m_ListOfInterpolationSessions[m_SelectedSegmentation];
  for (unsigned int i = 0; i < contourList.size(); ++i)
  {
    ContourPositionPair pair = contourList.at(i);
    if (mitk::Equal(*plane, *pair.plane, mitk::eps, false))
      return pair.contour;
  }
  return 0;
}

unsigned int mitk::SurfaceInterpolationController::GetNumberOfContours()
{
  return m_ListOfInterpolationSessions[m_SelectedSegmentation].size();
}

void mitk::SurfaceInterpolationController::Interpolate()
{
  if (m_CurrentNumberOfReducedContours< 2)
  {
    //If no interpolation is possible reset the interpolation result
    m_InterpolationResult = 0;
    return;
  }

  //Setting up progress bar
   /*
    * Removed due to bug 12441. ProgressBar messes around with Qt event queue which is fatal for segmentation
    */
  //mitk::ProgressBar::GetInstance()->AddStepsToDo(8);

  // update the filter and get teh resulting distance-image
  m_InterpolateSurfaceFilter->Update();
  Image::Pointer distanceImage = m_InterpolateSurfaceFilter->GetOutput();

  // create a surface from the distance-image
  mitk::ImageToSurfaceFilter::Pointer imageToSurfaceFilter = mitk::ImageToSurfaceFilter::New();
  imageToSurfaceFilter->SetInput( distanceImage );
  imageToSurfaceFilter->SetThreshold( 0 );
  imageToSurfaceFilter->SetSmooth(true);
  imageToSurfaceFilter->SetSmoothIteration(20);
  imageToSurfaceFilter->Update();
  m_InterpolationResult = imageToSurfaceFilter->GetOutput();


  vtkSmartPointer<vtkAppendPolyData> polyDataAppender = vtkSmartPointer<vtkAppendPolyData>::New();
  for (unsigned int i = 0; i < m_ReduceFilter->GetNumberOfOutputs(); i++)
  {
    polyDataAppender->AddInputData(m_ReduceFilter->GetOutput(i)->GetVtkPolyData());
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

void mitk::SurfaceInterpolationController::SetDataStorage(DataStorage::Pointer ds)
{
  m_DataStorage = ds;
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

void mitk::SurfaceInterpolationController::SetSegmentationImage(Image* /*workingImage*/)
{
//  m_NormalsFilter->SetSegmentationBinaryImage(workingImage);
}

mitk::Image::Pointer mitk::SurfaceInterpolationController::GetCurrentSegmentation()
{
  return m_SelectedSegmentation;
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

unsigned int mitk::SurfaceInterpolationController::GetNumberOfInterpolationSessions()
{
  return m_ListOfInterpolationSessions.size();
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::SurfaceInterpolationController::GetImageBase(itk::Image<TPixel, VImageDimension>* input, itk::ImageBase<3>::Pointer& result)
{
  result->Graft(input);
}

void mitk::SurfaceInterpolationController::SetCurrentSegmentationInterpolationList(mitk::Image::Pointer segmentation)
{
  this->SetCurrentInterpolationSession(segmentation);
}

void mitk::SurfaceInterpolationController::SetCurrentInterpolationSession(mitk::Image::Pointer currentSegmentationImage)
{
  if (currentSegmentationImage.GetPointer() == m_SelectedSegmentation)
    return;

  m_ReduceFilter->Reset();
  m_NormalsFilter->Reset();
  m_InterpolateSurfaceFilter->Reset();

  if (currentSegmentationImage.IsNull())
  {
    m_SelectedSegmentation = 0;
    return;
  }
  ContourListMap::iterator it = m_ListOfInterpolationSessions.find(currentSegmentationImage.GetPointer());

  m_SelectedSegmentation = currentSegmentationImage.GetPointer();

  itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();
  AccessFixedDimensionByItk_1( m_SelectedSegmentation, GetImageBase, 3, itkImage );
  m_InterpolateSurfaceFilter->SetReferenceImage( itkImage.GetPointer() );

  if (it == m_ListOfInterpolationSessions.end())
  {
    ContourPositionPairList newList;
    m_ListOfInterpolationSessions.insert(std::pair<mitk::Image*, ContourPositionPairList>(m_SelectedSegmentation, newList));
    m_InterpolationResult = 0;
    m_CurrentNumberOfReducedContours = 0;

    itk::MemberCommand<SurfaceInterpolationController>::Pointer command = itk::MemberCommand<SurfaceInterpolationController>::New();
    command->SetCallbackFunction(this, &SurfaceInterpolationController::OnSegmentationDeleted);
    m_SegmentationObserverTags.insert( std::pair<mitk::Image*, unsigned long>( m_SelectedSegmentation, m_SelectedSegmentation->AddObserver( itk::DeleteEvent(), command ) ) );

  }
  else
  {
    for (unsigned int i = 0; i < m_ListOfInterpolationSessions[m_SelectedSegmentation].size(); i++)
    {
      m_ReduceFilter->SetInput(i, m_ListOfInterpolationSessions[m_SelectedSegmentation].at(i).contour);
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
  this->RemoveInterpolationSession(segmentation);
}

void mitk::SurfaceInterpolationController::RemoveInterpolationSession(mitk::Image::Pointer segmentationImage)
{
  if (segmentationImage)
  {
    if (m_SelectedSegmentation == segmentationImage)
    {
      SetSegmentationImage(NULL);
      m_SelectedSegmentation = 0;
    }
    m_ListOfInterpolationSessions.erase(segmentationImage);
    // Remove observer
    std::map<mitk::Image*, unsigned long>::iterator pos = m_SegmentationObserverTags.find(segmentationImage);
    if (pos != m_SegmentationObserverTags.end())
    {
      segmentationImage->RemoveObserver((*pos).second);
      m_SegmentationObserverTags.erase(pos);
    }
  }
}

void mitk::SurfaceInterpolationController::RemoveAllInterpolationSessions()
{
  //Removing all observers
  std::map<mitk::Image*, unsigned long>::iterator dataIter = m_SegmentationObserverTags.begin();
  while (dataIter != m_SegmentationObserverTags.end())
  {
    mitk::Image* image = (*dataIter).first;
    image->RemoveObserver((*dataIter).second);
    ++dataIter;
  }

  m_SegmentationObserverTags.clear();
  m_SelectedSegmentation = 0;
  m_ListOfInterpolationSessions.clear();
}

void mitk::SurfaceInterpolationController::OnSegmentationDeleted(const itk::Object *caller, const itk::EventObject &/*event*/)
{
  mitk::Image* tempImage = dynamic_cast<mitk::Image*>(const_cast<itk::Object*>(caller));
  if (tempImage)
  {
    if (m_SelectedSegmentation == tempImage)
    {
      SetSegmentationImage(NULL);
      m_SelectedSegmentation = 0;
    }
    m_SegmentationObserverTags.erase(tempImage);
    m_ListOfInterpolationSessions.erase(tempImage);
  }
}
