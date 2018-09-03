#include "itkTractsToVectorImageFilter.h"

// VTK
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>

// ITK
#include <itkTimeProbe.h>
#include <itkImageRegionIterator.h>

// misc
#define _USE_MATH_DEFINES
#include <cmath>
#include <boost/progress.hpp>
#include <mitkDiffusionFunctionCollection.h>

namespace itk{

static inline bool CompareVectorLengths(const vnl_vector_fixed< double, 3 >& v1, const vnl_vector_fixed< double, 3 >& v2)
{
  return (v1.magnitude()>v2.magnitude());
}

template< class PixelType >
TractsToVectorImageFilter< PixelType >::TractsToVectorImageFilter():
  m_NormalizationMethod(GLOBAL_MAX),
  m_AngularThreshold(0.7),
  m_Epsilon(0.999),
  m_MaxNumDirections(3),
  m_SizeThreshold(0.3),
  m_OnlyUseMaskGeometry(false)
{
  this->SetNumberOfRequiredOutputs(1);
}


template< class PixelType >
TractsToVectorImageFilter< PixelType >::~TractsToVectorImageFilter()
{
}


template< class PixelType >
vnl_vector_fixed<double, 3> TractsToVectorImageFilter< PixelType >::GetVnlVector(double point[])
{
  vnl_vector_fixed<double, 3> vnlVector;
  vnlVector[0] = point[0];
  vnlVector[1] = point[1];
  vnlVector[2] = point[2];
  return vnlVector;
}

template< class PixelType >
void TractsToVectorImageFilter< PixelType >::GenerateData()
{
  mitk::BaseGeometry::Pointer geometry = m_FiberBundle->GetGeometry();

  // calculate new image parameters
  itk::Vector<double> spacing3;
  itk::Point<double> origin3;
  itk::Matrix<double, 3, 3> direction3;
  ImageRegion<3> imageRegion3;
  if (!m_MaskImage.IsNull())
  {
    spacing3 = m_MaskImage->GetSpacing();
    imageRegion3 = m_MaskImage->GetLargestPossibleRegion();
    origin3 = m_MaskImage->GetOrigin();
    direction3 = m_MaskImage->GetDirection();
  }
  else
  {
    spacing3 = geometry->GetSpacing();
    origin3 = geometry->GetOrigin();
    mitk::BaseGeometry::BoundsArrayType bounds = geometry->GetBounds();
    origin3[0] += bounds.GetElement(0);
    origin3[1] += bounds.GetElement(2);
    origin3[2] += bounds.GetElement(4);

    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
        direction3[j][i] = geometry->GetMatrixColumn(i)[j];
    imageRegion3.SetSize(0, geometry->GetExtent(0)+1);
    imageRegion3.SetSize(1, geometry->GetExtent(1)+1);
    imageRegion3.SetSize(2, geometry->GetExtent(2)+1);


    m_MaskImage = ItkUcharImgType::New();
    m_MaskImage->SetSpacing( spacing3 );
    m_MaskImage->SetOrigin( origin3 );
    m_MaskImage->SetDirection( direction3 );
    m_MaskImage->SetRegions( imageRegion3 );
    m_MaskImage->Allocate();
    m_MaskImage->FillBuffer(1);
  }
  OutputImageType::RegionType::SizeType outImageSize = imageRegion3.GetSize();
  m_OutImageSpacing = m_MaskImage->GetSpacing();
  m_ClusteredDirectionsContainer = ContainerType::New();

  // initialize num directions image
  m_NumDirectionsImage = ItkUcharImgType::New();
  m_NumDirectionsImage->SetSpacing( spacing3 );
  m_NumDirectionsImage->SetOrigin( origin3 );
  m_NumDirectionsImage->SetDirection( direction3 );
  m_NumDirectionsImage->SetRegions( imageRegion3 );
  m_NumDirectionsImage->Allocate();
  m_NumDirectionsImage->FillBuffer(0);

  itk::Vector<double, 4> spacing4;
  itk::Point<float, 4> origin4;
  itk::Matrix<double, 4, 4> direction4;
  itk::ImageRegion<4> imageRegion4;

  spacing4[0] = spacing3[0]; spacing4[1] = spacing3[1]; spacing4[2] = spacing3[2]; spacing4[3] = 1;
  origin4[0] = origin3[0]; origin4[1] = origin3[1]; origin4[2] = origin3[2]; origin3[3] = 0;
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
      direction4[r][c] = direction3[r][c];
  direction4[3][3] = 1;
  imageRegion4.SetSize(0, imageRegion3.GetSize()[0]);
  imageRegion4.SetSize(1, imageRegion3.GetSize()[1]);
  imageRegion4.SetSize(2, imageRegion3.GetSize()[2]);
  imageRegion4.SetSize(3, m_MaxNumDirections*3);

  m_DirectionImage = ItkDirectionImageType::New();
  m_DirectionImage->SetSpacing( spacing4 );
  m_DirectionImage->SetOrigin( origin4 );
  m_DirectionImage->SetDirection( direction4 );
  m_DirectionImage->SetRegions( imageRegion4 );
  m_DirectionImage->Allocate();
  m_DirectionImage->FillBuffer(0.0);

  // iterate over all fibers
  vtkSmartPointer<vtkPolyData> fiberPolyData = m_FiberBundle->GetFiberPolyData();
  int numFibers = m_FiberBundle->GetNumFibers();
  m_DirectionsContainer = ContainerType::New();

  VectorContainer< unsigned int, std::vector< double > >::Pointer peakLengths = VectorContainer< unsigned int, std::vector< double > >::New();

  MITK_INFO << "Generating directions from tractogram";
  boost::progress_display disp(numFibers);
  for( int i=0; i<numFibers; i++ )
  {
    ++disp;
    vtkCell* cell = fiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();
    if (numPoints<2)
      continue;

    vnl_vector_fixed<double, 3> dir;
    vnl_vector<double> v;

    float fiberWeight = m_FiberBundle->GetFiberWeight(i);

    for( int j=0; j<numPoints-1; j++)
    {
      itk::Point<float, 3> startVertex = mitk::imv::GetItkPoint(points->GetPoint(j));
      itk::Index<3> startIndex;
      itk::ContinuousIndex<float, 3> startIndexCont;
      m_MaskImage->TransformPhysicalPointToIndex(startVertex, startIndex);
      m_MaskImage->TransformPhysicalPointToContinuousIndex(startVertex, startIndexCont);

      itk::Point<float, 3> endVertex = mitk::imv::GetItkPoint(points->GetPoint(j + 1));
      itk::Index<3> endIndex;
      itk::ContinuousIndex<float, 3> endIndexCont;
      m_MaskImage->TransformPhysicalPointToIndex(endVertex, endIndex);
      m_MaskImage->TransformPhysicalPointToContinuousIndex(endVertex, endIndexCont);

      dir[0] = endVertex[0]-startVertex[0];
      dir[1] = endVertex[1]-startVertex[1];
      dir[2] = endVertex[2]-startVertex[2];
      if (dir.is_zero())
        continue;
      dir.normalize();

      std::vector< std::pair< itk::Index<3>, double > > segments = mitk::imv::IntersectImage(spacing3, startIndex, endIndex, startIndexCont, endIndexCont);
      for (std::pair< itk::Index<3>, double > segment : segments)
      {
        if (!m_MaskImage->GetLargestPossibleRegion().IsInside(segment.first) || (!m_OnlyUseMaskGeometry && m_MaskImage->GetPixel(segment.first)==0))
          continue;

        // add direction to container
        unsigned int idx = segment.first[0] + outImageSize[0]*(segment.first[1] + outImageSize[1]*segment.first[2]);
        DirectionContainerType::Pointer dirCont;
        if (m_DirectionsContainer->IndexExists(idx))
        {
          peakLengths->ElementAt(idx).push_back(fiberWeight*segment.second);

          dirCont = m_DirectionsContainer->GetElement(idx);
          if (dirCont.IsNull())
          {
            dirCont = DirectionContainerType::New();
            dirCont->push_back(dir);
            m_DirectionsContainer->InsertElement(idx, dirCont);
          }
          else
            dirCont->push_back(dir);
        }
        else
        {
          dirCont = DirectionContainerType::New();
          dirCont->push_back(dir);
          m_DirectionsContainer->InsertElement(idx, dirCont);

          std::vector< double > lengths; lengths.push_back(fiberWeight*segment.second);
          peakLengths->InsertElement(idx, lengths);
        }
      }

    }
  }

  itk::ImageRegionIterator<ItkUcharImgType> dirIt(m_NumDirectionsImage, m_NumDirectionsImage->GetLargestPossibleRegion());

  MITK_INFO << "Clustering directions";
  float max_dir_mag = 0;
  boost::progress_display disp2(outImageSize[0]*outImageSize[1]*outImageSize[2]);
  while(!dirIt.IsAtEnd())
  {
    ++disp2;
    OutputImageType::IndexType idx3 = dirIt.GetIndex();
    int idx_lin = idx3[0]+(idx3[1]+idx3[2]*outImageSize[1])*outImageSize[0];

    itk::Index<4> idx4; idx4[0] = idx3[0]; idx4[1] = idx3[1]; idx4[2] = idx3[2];

    if (!m_DirectionsContainer->IndexExists(idx_lin))
    {
      ++dirIt;
      continue;
    }
    DirectionContainerType::Pointer dirCont = m_DirectionsContainer->GetElement(idx_lin);
    if (dirCont.IsNull() || dirCont->empty())
    {
      ++dirIt;
      continue;
    }

    DirectionContainerType::Pointer directions;
    if (m_MaxNumDirections>0)
    {
      directions = FastClustering(dirCont, peakLengths->GetElement(idx_lin));
      std::sort( directions->begin(), directions->end(), CompareVectorLengths );
    }
    else
      directions = dirCont;

    unsigned int numDir = directions->size();
    if (m_MaxNumDirections>0 && numDir>m_MaxNumDirections)
      numDir = m_MaxNumDirections;

    float voxel_max_mag = 0;
    for (unsigned int i=0; i<numDir; i++)
    {
      DirectionType dir = directions->at(i);
      float mag = dir.magnitude();

      if (mag>voxel_max_mag)
        voxel_max_mag = mag;
      if (mag>max_dir_mag)
        max_dir_mag = mag;
    }

    int count = 0;
    for (unsigned int i=0; i<numDir; i++)
    {
      DirectionType dir = directions->at(i);
      count++;

      float mag = dir.magnitude();
      if (m_NormalizationMethod==MAX_VEC_NORM && voxel_max_mag>mitk::eps)
        dir /= voxel_max_mag;
      else if (m_NormalizationMethod==SINGLE_VEC_NORM && mag>mitk::eps)
        dir.normalize();

      for (unsigned int j = 0; j<3; j++)
      {
        idx4[3] = i*3 + j;
        m_DirectionImage->SetPixel(idx4, dir[j]);
      }
    }
    dirIt.Set(count);
    ++dirIt;
  }

  if (m_NormalizationMethod==GLOBAL_MAX && max_dir_mag>0)
  {
    itk::ImageRegionIterator<ItkDirectionImageType> dirImgIt(m_DirectionImage, m_DirectionImage->GetLargestPossibleRegion());
    while(!dirImgIt.IsAtEnd())
    {
      dirImgIt.Set(dirImgIt.Get()/max_dir_mag);
      ++dirImgIt;
    }
  }
}


template< class PixelType >
TractsToVectorImageFilter< PixelType >::DirectionContainerType::Pointer TractsToVectorImageFilter< PixelType >::FastClustering(DirectionContainerType::Pointer inDirs, std::vector< double > lengths)
{
  DirectionContainerType::Pointer outDirs = DirectionContainerType::New();
  if (inDirs->size()<2)
  {
    if (inDirs->size()==1)
      inDirs->SetElement(0, inDirs->at(0)*lengths.at(0));
    return inDirs;
  }

  DirectionType oldMean, currentMean;
  std::vector< int > touched;

  // initialize
  touched.resize(inDirs->size(), 0);
  bool free = true;
  currentMean = inDirs->at(0);  // initialize first seed
  currentMean.normalize();
  double length = lengths.at(0);
  touched[0] = 1;
  std::vector< double > newLengths;
  bool meanChanged = false;

  double max = 0;
  while (free)
  {
    oldMean.fill(0.0);

    // start mean-shift clustering
    double angle = 0;

    while (fabs(dot_product(currentMean, oldMean))<0.99)
    {
      oldMean = currentMean;
      currentMean.fill(0.0);
      for (unsigned int i=0; i<inDirs->size(); i++)
      {
        angle = dot_product(oldMean, inDirs->at(i));
        if (angle>=m_AngularThreshold)
        {
          currentMean += inDirs->at(i);
          if (meanChanged)
            length += lengths.at(i);
          touched[i] = 1;
          meanChanged = true;
        }
        else if (-angle>=m_AngularThreshold)
        {
          currentMean -= inDirs->at(i);
          if (meanChanged)
            length += lengths.at(i);
          touched[i] = 1;
          meanChanged = true;
        }
      }
      if(!meanChanged)
        currentMean = oldMean;
      else
        currentMean.normalize();
    }

    // found stable mean
    outDirs->push_back(currentMean);
    newLengths.push_back(length);
    if (length>max)
      max = length;

    // find next unused seed
    free = false;
    for (unsigned int i=0; i<touched.size(); i++)
      if (touched[i]==0)
      {
        currentMean = inDirs->at(i);
        free = true;
        meanChanged = false;
        length = lengths.at(i);
        touched[i] = 1;
        break;
      }
  }

  if (inDirs->size()==outDirs->size())
  {
    if (max>0)
    {
      for (unsigned int i=0; i<outDirs->size(); i++)
        outDirs->SetElement(i, outDirs->at(i)*newLengths.at(i));
    }
    return outDirs;
  }
  else
    return FastClustering(outDirs, newLengths);
}

}



