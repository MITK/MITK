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

// TODO Use Equal of BaseGeometry instead after master merge!
bool PlanesEqual (mitk::PlaneGeometry::ConstPointer leftHandSide, mitk::PlaneGeometry::ConstPointer rightHandSide, mitk::ScalarType eps)
{
  bool result = true;

  //Compare spacings
  if( !mitk::Equal( leftHandSide->GetSpacing(), rightHandSide->GetSpacing(), eps ) )
  {
    result = false;
  }

  //Compare Origins
  if( !mitk::Equal( leftHandSide->GetOrigin(), rightHandSide->GetOrigin(), eps ) )
  {
    result = false;
  }

  //Compare Axis and Extents
  for( unsigned int i=0; i<3; ++i)
  {
    if( !mitk::Equal( leftHandSide->GetAxisVector(i), rightHandSide->GetAxisVector(i), eps))
    {
      result =  false;
    }

    if( !mitk::Equal( leftHandSide->GetExtent(i), rightHandSide->GetExtent(i), eps) )
    {
      result = false;
    }
  }

  //Compare ImageGeometry Flag
  if( rightHandSide->GetImageGeometry() != leftHandSide->GetImageGeometry() )
  {
    result = false;
  }

  //Compare IndexToWorldTransform Matrix
  if( !mitk::MatrixEqualElementWise( leftHandSide->GetIndexToWorldTransform()->GetMatrix(), rightHandSide->GetIndexToWorldTransform()->GetMatrix(), eps) )
  {
    result = false;
  }
  return result;
}

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

void mitk::SurfaceInterpolationController::AddNewContour (mitk::Surface::Pointer newContour, PlaneGeometry::ConstPointer plane)
{
  ContourPositionPair pair;
  pair.contour = newContour;
  pair.plane = plane;
  this->AddToInterpolationPipeline(pair);

  this->Modified();
}

void mitk::SurfaceInterpolationController::AddNewContours(ContourPositionPairList newContours)
{
  for (unsigned int i = 0; i < newContours.size(); ++i)
  {
    this->AddToInterpolationPipeline(newContours.at(i));
  }
  this->Modified();
}

void mitk::SurfaceInterpolationController::AddToInterpolationPipeline(ContourPositionPair pair)
{
  int pos (-1);
  ContourPositionPairList currentContourList = m_ListOfInterpolationSessions[m_SelectedSegmentation];
  const mitk::PlaneGeometry* plane = pair.plane;
  mitk::Surface* newContour = pair.contour;
  for (unsigned int i = 0; i < currentContourList.size(); i++)
  {
    mitk::PlaneGeometry::ConstPointer planeFromList = currentContourList.at(i).plane;
    if ( PlanesEqual(plane, planeFromList, mitk::eps) )
    {
      pos = i;
      break;
    }
  }

  //Don't save a new empty contour
  if (pos == -1 && newContour->GetVtkPolyData()->GetNumberOfPoints() > 0)
  {
    m_ReduceFilter->SetInput(m_ListOfInterpolationSessions[m_SelectedSegmentation].size(), newContour);
    m_ListOfInterpolationSessions[m_SelectedSegmentation].push_back(pair);
  }
  else if (pos != -1 && newContour->GetVtkPolyData()->GetNumberOfPoints() > 0)
  {
    m_ListOfInterpolationSessions[m_SelectedSegmentation].at(pos) = pair;
    m_ReduceFilter->SetInput(pos, newContour);
  }
  else if (newContour->GetVtkPolyData()->GetNumberOfPoints() == 0)
  {
    this->RemoveContour(plane);
  }

  m_ReduceFilter->Update();
  m_CurrentNumberOfReducedContours = m_ReduceFilter->GetNumberOfOutputs();

  for (unsigned int i = 0; i < m_CurrentNumberOfReducedContours; i++)
  {
    m_NormalsFilter->SetInput(i, m_ReduceFilter->GetOutput(i));
    m_InterpolateSurfaceFilter->SetInput(i, m_NormalsFilter->GetOutput(i));
  }
}

bool mitk::SurfaceInterpolationController::RemoveContour(const mitk::PlaneGeometry *plane)
{
  ContourPositionPairList::iterator it = m_ListOfInterpolationSessions[m_SelectedSegmentation].begin();
  while (it !=  m_ListOfInterpolationSessions[m_SelectedSegmentation].end())
  {
    ContourPositionPair pair = (*it);
    if (PlanesEqual(plane, pair.plane, mitk::eps))
    {
      m_ListOfInterpolationSessions[m_SelectedSegmentation].erase(it);
      this->ReinitializeInterpolation();
      return true;
    }
    ++it;
  }
  return false;
}

const mitk::Surface* mitk::SurfaceInterpolationController::GetContour(mitk::PlaneGeometry::ConstPointer plane)
{
  ContourPositionPairList contourList = m_ListOfInterpolationSessions[m_SelectedSegmentation];
  for (unsigned int i = 0; i < contourList.size(); ++i)
  {
    ContourPositionPair pair = contourList.at(i);
    if (PlanesEqual(plane, pair.plane, mitk::eps))
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
    ContourPositionPairList newList;
    m_ListOfInterpolationSessions.insert(std::pair<mitk::Image*, ContourPositionPairList>(m_SelectedSegmentation, newList));
    m_InterpolationResult = 0;
    m_CurrentNumberOfReducedContours = 0;

    itk::MemberCommand<SurfaceInterpolationController>::Pointer command = itk::MemberCommand<SurfaceInterpolationController>::New();
    command->SetCallbackFunction(this, &SurfaceInterpolationController::OnSegmentationDeleted);
    m_SegmentationObserverTags.insert( std::pair<mitk::Image*, unsigned long>( m_SelectedSegmentation, m_SelectedSegmentation->AddObserver( itk::DeleteEvent(), command ) ) );
  }

  this->ReinitializeInterpolation();
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
  unsigned int num_Polys = contours->GetVtkPolyData()->GetNumberOfPolys();
  MITK_INFO<<"Reinit 3D interpolation using ["<<num_Polys<<"] number of contours!";

  MITK_INFO<<"----- Prepare surfaces -----";

  // 1. detect coplanar contours
  // 2. merge coplanar contours into a single surface
  // 4. add contour to pipeline
  // 5. create position nodes

  // Sort contours
  vtkSmartPointer<vtkCellArray> existingPolys;
  vtkSmartPointer<vtkPoints> existingPoints;
  existingPolys = contours->GetVtkPolyData()->GetPolys();
  existingPoints = contours->GetVtkPolyData()->GetPoints();
  existingPolys->InitTraversal();

  vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();

  typedef std::pair<mitk::Vector3D, mitk::Point3D> PointNormalPair;
  std::vector<PointNormalPair> list;
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
    double p1[3];

    existingPoints->GetPoint(ids->GetId(0), p1);
    p_n.second = p1;

    list.push_back(p_n);

    continue;
  }

  double vec[3];
//  std::vector<std::pair<unsigned int, unsigned int> > coplanar_indices;

  std::vector<PointNormalPair>::iterator outer = list.begin();
//  for (unsigned int i = 0; i < list.size(); ++i)
  std::vector< std::vector< vtkSmartPointer<vtkPoints> > > relatedPoints;
  while (outer != list.end())
  {
    std::vector<PointNormalPair>::iterator inner = outer;
    ++inner;
    std::vector< vtkSmartPointer<vtkPoints> > rel;
    std::vector< vtkSmartPointer<vtkPoints> >::iterator pointsIter = pointsList.begin();
    rel.push_back((*pointsIter));
    pointsIter = pointsList.erase(pointsIter);
//    for (unsigned int j = i+1; j < list.size(); ++j)
    while (inner != list.end())
    {
      vec[0] = (*outer).second[0] - (*inner).second[0];
      vec[1] = (*outer).second[1] - (*inner).second[1];
      vec[2] = (*outer).second[2] - (*inner).second[2];
      double n[3];
      n[0] = (*outer).first[0];
      n[1] = (*outer).first[1];
      n[2] = (*outer).first[2];
      double dot = vtkMath::Dot(n, vec);

      if (mitk::Equal(dot, 0.0, 0.001, true))
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

  // Build the surfaces
  MITK_INFO<<"NUM REL POINTS: "<<relatedPoints.size();
  std::vector<mitk::Surface::Pointer> finalSurfaces;
  for (unsigned int i = 0; i < relatedPoints.size(); ++i)
  {
    MITK_INFO<<"Size ["<<i<<"]: "<<relatedPoints.at(i).size();
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

  for (unsigned int i = 0; i < finalSurfaces.size(); ++i)
  {
    mitk::Surface* surface = finalSurfaces.at(i);
    ContourPositionPair contourPosPair;
    contourPosPair.contour = surface;
    this->AddToInterpolationPipeline(contourPosPair);
  }
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
