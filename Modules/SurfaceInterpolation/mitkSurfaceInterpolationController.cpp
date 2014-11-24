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

// Check whether the given contours are coplanar
bool ContoursCoplanar(mitk::SurfaceInterpolationController::ContourPositionInformation leftHandSide, mitk::SurfaceInterpolationController::ContourPositionInformation rightHandSide)
{
  // Here we check two things:
  // 1. Whether the normals of both contours are at least parallel
  // 2. Whether both contours lie in the same plane

  // Check for coplanarity:
  // a. Span a vector between two points one from each contour
  // b. Calculate dot product for the vector and one of the normals
  // c. If the dot is zero the two vectors are orthogonal and the contours are coplanar

  double vec[3];
  vec[0] = leftHandSide.contourPoint[0] - rightHandSide.contourPoint[0];
  vec[1] = leftHandSide.contourPoint[1] - rightHandSide.contourPoint[1];
  vec[2] = leftHandSide.contourPoint[2] - rightHandSide.contourPoint[2];
  double n[3];
  n[0] = rightHandSide.contourNormal[0];
  n[1] = rightHandSide.contourNormal[1];
  n[2] = rightHandSide.contourNormal[2];
  double dot = vtkMath::Dot(n, vec);

  double n2[3];
  n2[0] = leftHandSide.contourNormal[0];
  n2[1] = leftHandSide.contourNormal[1];
  n2[2] = leftHandSide.contourNormal[2];

  // The normals of both contours have to be parallel but not of the same orientation
  double lengthLHS = leftHandSide.contourNormal.GetNorm();
  double lengthRHS = rightHandSide.contourNormal.GetNorm();
  double dot2 = vtkMath::Dot(n, n2);
  bool contoursParallel = mitk::Equal(fabs(lengthLHS*lengthRHS), fabs(dot2), 0.001);

  if (mitk::Equal(dot, 0.0, 0.001) && contoursParallel)
    return true;
  else
    return false;
}

mitk::SurfaceInterpolationController::ContourPositionInformation CreateContourPositionInformation(mitk::Surface::Pointer contour)
{
  mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo;
  contourInfo.contour = contour;
  double n[3];
  double p[3];
  contour->GetVtkPolyData()->GetPoints()->GetPoint(0, p);
  vtkPolygon::ComputeNormal(contour->GetVtkPolyData()->GetPoints(), n);
  contourInfo.contourNormal = n;
  contourInfo.contourPoint = p;
  return contourInfo;
}

mitk::SurfaceInterpolationController::SurfaceInterpolationController()
  :m_SelectedSegmentation(0)
{
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();

  m_ReduceFilter->SetUseProgressBar(false);
//  m_ReduceFilter->SetProgressStepSize(1);
  m_NormalsFilter->SetUseProgressBar(true);
  m_NormalsFilter->SetProgressStepSize(1);
  m_InterpolateSurfaceFilter->SetUseProgressBar(true);
  m_InterpolateSurfaceFilter->SetProgressStepSize(7);

  m_Contours = Surface::New();

  m_PolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  m_PolyData->SetPoints(points);

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
  static mitk::SurfaceInterpolationController::Pointer m_Instance;

  if ( m_Instance.IsNull() )
  {
    m_Instance = SurfaceInterpolationController::New();
  }
  return m_Instance;
}

void mitk::SurfaceInterpolationController::AddNewContour (mitk::Surface::Pointer newContour)
{
  if( newContour->GetVtkPolyData()->GetNumberOfPoints() > 0)
  {
    ContourPositionInformation contourInfo = CreateContourPositionInformation(newContour);
    this->AddToInterpolationPipeline(contourInfo);

    this->Modified();
  }
}

void mitk::SurfaceInterpolationController::AddNewContours(std::vector<mitk::Surface::Pointer> newContours)
{
  for (unsigned int i = 0; i < newContours.size(); ++i)
  {
    if( newContours.at(i)->GetVtkPolyData()->GetNumberOfPoints() > 0)
    {
      ContourPositionInformation contourInfo = CreateContourPositionInformation(newContours.at(i));
      this->AddToInterpolationPipeline(contourInfo);
    }
  }
  this->Modified();
}

void mitk::SurfaceInterpolationController::AddToInterpolationPipeline(ContourPositionInformation contourInfo)
{
  int pos (-1);
  ContourPositionInformationList currentContourList = m_ListOfInterpolationSessions[m_SelectedSegmentation];
  mitk::Surface* newContour = contourInfo.contour;
  for (unsigned int i = 0; i < currentContourList.size(); i++)
  {
    ContourPositionInformation contourFromList = currentContourList.at(i);
    if (ContoursCoplanar(contourInfo, contourFromList))
    {
      pos = i;
      break;
    }
  }

  //Don't save a new empty contour
  if (pos == -1 && newContour->GetVtkPolyData()->GetNumberOfPoints() > 0)
  {
    m_ReduceFilter->SetInput(m_ListOfInterpolationSessions[m_SelectedSegmentation].size(), newContour);
    m_ListOfInterpolationSessions[m_SelectedSegmentation].push_back(contourInfo);
  }
  else if (pos != -1 && newContour->GetVtkPolyData()->GetNumberOfPoints() > 0)
  {
    m_ListOfInterpolationSessions[m_SelectedSegmentation].at(pos) = contourInfo;
    m_ReduceFilter->SetInput(pos, newContour);
  }
  else if (newContour->GetVtkPolyData()->GetNumberOfPoints() == 0)
  {
    this->RemoveContour(contourInfo);
  }

  m_ReduceFilter->Update();
  m_CurrentNumberOfReducedContours = m_ReduceFilter->GetNumberOfOutputs();

  for (unsigned int i = 0; i < m_CurrentNumberOfReducedContours; i++)
  {
    m_NormalsFilter->SetInput(i, m_ReduceFilter->GetOutput(i));
    m_InterpolateSurfaceFilter->SetInput(i, m_NormalsFilter->GetOutput(i));
  }
}

bool mitk::SurfaceInterpolationController::RemoveContour(ContourPositionInformation contourInfo)
{
  if(!m_SelectedSegmentation)
    return false;
  ContourPositionInformationList::iterator it = m_ListOfInterpolationSessions[m_SelectedSegmentation].begin();
  while (it !=  m_ListOfInterpolationSessions[m_SelectedSegmentation].end())
  {
    ContourPositionInformation currentContour = (*it);
    if (ContoursCoplanar(currentContour, contourInfo))
    {
      m_ListOfInterpolationSessions[m_SelectedSegmentation].erase(it);
      this->ReinitializeInterpolation();
      return true;
    }
    ++it;
  }
  return false;
}

const mitk::Surface* mitk::SurfaceInterpolationController::GetContour(ContourPositionInformation contourInfo)
{
  ContourPositionInformationList contourList = m_ListOfInterpolationSessions[m_SelectedSegmentation];
  for (unsigned int i = 0; i < contourList.size(); ++i)
  {
    ContourPositionInformation currentContour = contourList.at(i);
    if (ContoursCoplanar(contourInfo, currentContour))
      return currentContour.contour;
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
  mitk::ProgressBar::GetInstance()->AddStepsToDo(10);

  // create a surface from the distance-image
  mitk::ImageToSurfaceFilter::Pointer imageToSurfaceFilter = mitk::ImageToSurfaceFilter::New();
  imageToSurfaceFilter->SetInput( m_InterpolateSurfaceFilter->GetOutput() );
  imageToSurfaceFilter->SetThreshold( 0 );
  imageToSurfaceFilter->SetSmooth(true);
  imageToSurfaceFilter->SetSmoothIteration(20);
  imageToSurfaceFilter->Update();
  m_InterpolationResult = imageToSurfaceFilter->GetOutput();

  vtkSmartPointer<vtkAppendPolyData> polyDataAppender = vtkSmartPointer<vtkAppendPolyData>::New();
  for (unsigned int i = 0; i < m_ListOfInterpolationSessions[m_SelectedSegmentation].size(); i++)
  {
    polyDataAppender->AddInputData(m_ListOfInterpolationSessions[m_SelectedSegmentation].at(i).contour->GetVtkPolyData());
  }
  polyDataAppender->Update();
  m_Contours->SetVtkPolyData(polyDataAppender->GetOutput());

  //Last progress step
  mitk::ProgressBar::GetInstance()->Progress(20);

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

  if (currentSegmentationImage.IsNull())
  {
    m_SelectedSegmentation = 0;
    return;
  }

  m_SelectedSegmentation = currentSegmentationImage.GetPointer();

  ContourListMap::iterator it = m_ListOfInterpolationSessions.find(currentSegmentationImage.GetPointer());
  // If the session does not exist yet create a new ContourPositionPairList otherwise reinitialize the interpolation pipeline
  if (it == m_ListOfInterpolationSessions.end())
  {
    ContourPositionInformationList newList;
    m_ListOfInterpolationSessions.insert(std::pair<mitk::Image*, ContourPositionInformationList>(m_SelectedSegmentation, newList));
    m_InterpolationResult = 0;
    m_CurrentNumberOfReducedContours = 0;

    itk::MemberCommand<SurfaceInterpolationController>::Pointer command = itk::MemberCommand<SurfaceInterpolationController>::New();
    command->SetCallbackFunction(this, &SurfaceInterpolationController::OnSegmentationDeleted);
    m_SegmentationObserverTags.insert( std::pair<mitk::Image*, unsigned long>( m_SelectedSegmentation, m_SelectedSegmentation->AddObserver( itk::DeleteEvent(), command ) ) );
  }

  this->ReinitializeInterpolation();
}

bool mitk::SurfaceInterpolationController::ReplaceInterpolationSession(mitk::Image::Pointer oldSession, mitk::Image::Pointer newSession)
{
  if (oldSession.IsNull() || newSession.IsNull())
    return false;

  if (oldSession.GetPointer() == newSession.GetPointer())
    return false;

  if (!mitk::Equal(*(oldSession->GetGeometry()), *(newSession->GetGeometry()), mitk::eps, false))
    return false;

  ContourListMap::iterator it = m_ListOfInterpolationSessions.find(oldSession.GetPointer());

  if (it == m_ListOfInterpolationSessions.end())
    return false;

  ContourPositionInformationList oldList = (*it).second;
  m_ListOfInterpolationSessions.insert(std::pair<mitk::Image*, ContourPositionInformationList>(newSession.GetPointer(), oldList));
  itk::MemberCommand<SurfaceInterpolationController>::Pointer command = itk::MemberCommand<SurfaceInterpolationController>::New();
  command->SetCallbackFunction(this, &SurfaceInterpolationController::OnSegmentationDeleted);
  m_SegmentationObserverTags.insert( std::pair<mitk::Image*, unsigned long>( newSession, newSession->AddObserver( itk::DeleteEvent(), command ) ) );

  if (m_SelectedSegmentation == oldSession)
    m_SelectedSegmentation = newSession;
  m_NormalsFilter->SetSegmentationBinaryImage(m_SelectedSegmentation);

  this->RemoveInterpolationSession(oldSession);
  return true;
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
      m_NormalsFilter->SetSegmentationBinaryImage(NULL);
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

void mitk::SurfaceInterpolationController::ReinitializeInterpolation(mitk::Surface::Pointer contours)
{
  // 1. detect coplanar contours
  // 2. merge coplanar contours into a single surface
  // 4. add contour to pipeline

  // Split the surface into separate polygons
  vtkSmartPointer<vtkCellArray> existingPolys;
  vtkSmartPointer<vtkPoints> existingPoints;
  existingPolys = contours->GetVtkPolyData()->GetPolys();
  existingPoints = contours->GetVtkPolyData()->GetPoints();
  existingPolys->InitTraversal();

  vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();

  typedef std::pair<mitk::Vector3D, mitk::Point3D> PointNormalPair;
  std::vector<ContourPositionInformation> list;
  std::vector<vtkSmartPointer<vtkPoints> > pointsList;
  int count (0);
  for( existingPolys->InitTraversal(); existingPolys->GetNextCell(ids);)
  {
    // Get the points
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    existingPoints->GetPoints(ids, points);
    ++count;
    pointsList.push_back(points);


    PointNormalPair p_n;
    double n[3];
    vtkPolygon::ComputeNormal(points, n);
    p_n.first = n;
    double p[3];

    existingPoints->GetPoint(ids->GetId(0), p);
    p_n.second = p;

    ContourPositionInformation p_info;
    p_info.contourNormal = n;
    p_info.contourPoint = p;
    list.push_back(p_info);
    continue;
  }

  // Detect and sort coplanar polygons
  std::vector<ContourPositionInformation>::iterator outer = list.begin();
  std::vector< std::vector< vtkSmartPointer<vtkPoints> > > relatedPoints;
  while (outer != list.end())
  {
    std::vector<ContourPositionInformation>::iterator inner = outer;
    ++inner;
    std::vector< vtkSmartPointer<vtkPoints> > rel;
    std::vector< vtkSmartPointer<vtkPoints> >::iterator pointsIter = pointsList.begin();
    rel.push_back((*pointsIter));
    pointsIter = pointsList.erase(pointsIter);

    while (inner != list.end())
    {
      if(ContoursCoplanar((*outer),(*inner)))
      {
        inner = list.erase(inner);
        rel.push_back((*pointsIter));
        pointsIter = pointsList.erase(pointsIter);
      }
      else
      {
        ++inner;
        ++pointsIter;
      }
    }
    relatedPoints.push_back(rel);
    ++outer;
  }

  // Build the separate surfaces again
  std::vector<mitk::Surface::Pointer> finalSurfaces;
  for (unsigned int i = 0; i < relatedPoints.size(); ++i)
  {
    vtkSmartPointer<vtkPolyData> contourSurface = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();
    unsigned int pointId (0);
    for (unsigned int j = 0; j < relatedPoints.at(i).size(); ++j)
    {
      unsigned int numPoints = relatedPoints.at(i).at(j)->GetNumberOfPoints();
      vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
      polygon->GetPointIds()->SetNumberOfIds(numPoints);
      polygon->GetPoints()->SetNumberOfPoints(numPoints);
      vtkSmartPointer<vtkPoints> currentPoints = relatedPoints.at(i).at(j);
      for (unsigned k = 0; k < numPoints; ++k)
      {
        points->InsertPoint(pointId, currentPoints->GetPoint(k));
        polygon->GetPointIds()->SetId(k, pointId);
        ++pointId;
      }
      polygons->InsertNextCell(polygon);
    }
    contourSurface->SetPoints(points);
    contourSurface->SetPolys(polygons);
    contourSurface->BuildLinks();
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(contourSurface);
    finalSurfaces.push_back(surface);
  }

  // Add detected contours to interpolation pipeline
  this->AddNewContours(finalSurfaces);
}

void mitk::SurfaceInterpolationController::OnSegmentationDeleted(const itk::Object *caller, const itk::EventObject &/*event*/)
{
  mitk::Image* tempImage = dynamic_cast<mitk::Image*>(const_cast<itk::Object*>(caller));
  if (tempImage)
  {
    if (m_SelectedSegmentation == tempImage)
    {
      m_NormalsFilter->SetSegmentationBinaryImage(NULL);
      m_SelectedSegmentation = 0;
    }
    m_SegmentationObserverTags.erase(tempImage);
    m_ListOfInterpolationSessions.erase(tempImage);
  }
}

void mitk::SurfaceInterpolationController::ReinitializeInterpolation()
{
  m_NormalsFilter->SetSegmentationBinaryImage(m_SelectedSegmentation);

  // If session has changed reset the pipeline
  m_ReduceFilter->Reset();
  m_NormalsFilter->Reset();
  m_InterpolateSurfaceFilter->Reset();

  itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();
  AccessFixedDimensionByItk_1( m_SelectedSegmentation, GetImageBase, 3, itkImage );
  m_InterpolateSurfaceFilter->SetReferenceImage(itkImage.GetPointer());

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
  Modified();
}
