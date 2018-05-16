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
#ifndef __itkFiniteDiffOdfMaximaExtractionFilter_cpp
#define __itkFiniteDiffOdfMaximaExtractionFilter_cpp

#include "itkFiniteDiffOdfMaximaExtractionFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <vnl/vnl_vector.h>
#include <itkOrientationDistributionFunction.h>
#include <itkContinuousIndex.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

#include <boost/math/special_functions.hpp>
#include <boost/progress.hpp>
#include <mitkDiffusionFunctionCollection.h>

using namespace boost::math;

namespace itk {


static bool CompareVectors(const vnl_vector_fixed< double, 3 >& v1, const vnl_vector_fixed< double, 3 >& v2)
{
  return (v1.magnitude()>v2.magnitude());
}

template< class PixelType, int ShOrder, int NrOdfDirections >
FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::FiniteDiffOdfMaximaExtractionFilter()
  : m_NormalizationMethod(MAX_VEC_NORM)
  , m_MaxNumPeaks(2)
  , m_PeakThreshold(0.4)
  , m_AbsolutePeakThreshold(0)
  , m_ClusteringThreshold(0.9)
  , m_AngularThreshold(0.7)
  , m_NumCoeffs((ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder)
  , m_Toolkit(MRTRIX)
  , m_ApplyDirectionMatrix(false)
{
  this->SetNumberOfRequiredInputs(1);
}

template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::FindCandidatePeaks(OdfType& odf, double thr, std::vector< DirectionType >& container)
{
  double gfa = odf.GetGeneralizedFractionalAnisotropy();
  //Find the peaks using a finite difference method
  bool flag = true;
  vnl_vector_fixed< bool, NrOdfDirections > used; used.fill(false);
  //Find the peaks
  for (int i=0; i<NrOdfDirections; i++)
  {
    if (used[i])
      continue;

    double val = odf.GetElement(i);
    if (val>thr && val*gfa>m_AbsolutePeakThreshold)  // limit to one hemisphere ???
    {
      flag = true;
      std::vector< int > neighbours = odf.GetNeighbors(i);
      for (unsigned int j=0; j<neighbours.size(); j++)
        if (val<=odf.GetElement(neighbours.at(j)))
        {
          flag = false;
          break;
        }
      if (flag)   // point is a peak
      {
        container.push_back(odf.GetDirection(i).normalize());
        used[i] = true;
        for (unsigned int j=0; j<neighbours.size(); j++)
          used[neighbours.at(j)] = true;
      }
    }
  }
}

template< class PixelType, int ShOrder, int NrOdfDirections >
std::vector< vnl_vector_fixed< double, 3 > >  FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>::MeanShiftClustering(std::vector< DirectionType >& inDirs)
{
  std::vector< DirectionType > outDirs;
  if (inDirs.empty())
    return inDirs;

  DirectionType oldMean, currentMean, workingMean;
  std::vector< int > touched;

  // initialize
  touched.resize(inDirs.size(), 0);
  bool free = true;
  currentMean = inDirs[0];  // initialize first seed
  while (free)
  {
    oldMean.fill(0.0);

    // start mean-shift clustering
    float angle = 0.0;
    int counter = 0;
    while ((currentMean-oldMean).magnitude()>0.0001)
    {
      counter = 0;
      oldMean = currentMean;
      workingMean = oldMean;
      workingMean.normalize();
      currentMean.fill(0.0);
      for (unsigned int i=0; i<inDirs.size(); i++)
      {
        angle = dot_product(workingMean, inDirs[i]);
        if (angle>=m_ClusteringThreshold)
        {
          currentMean += inDirs[i];
          touched[i] = 1;
          counter++;
        }
        else if (-angle>=m_ClusteringThreshold)
        {
          currentMean -= inDirs[i];
          touched[i] = 1;
          counter++;
        }
      }
    }

    // found stable mean
    if (counter>0)
    {
      float mag = currentMean.magnitude();
      if (mag>0)
      {
        currentMean /= mag;
        outDirs.push_back(currentMean);
      }
    }

    // find next unused seed
    free = false;
    for (unsigned int i=0; i<touched.size(); i++)
      if (touched[i]==0)
      {
        currentMean = inDirs[i];
        free = true;
      }
  }

  if (inDirs.size()==outDirs.size())
  {
    return outDirs;
  }
  else
    return MeanShiftClustering(outDirs);
}

template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::BeforeThreadedGenerateData()
{
  typename CoefficientImageType::Pointer ShCoeffImage = static_cast< CoefficientImageType* >( this->ProcessObject::GetInput(0) );
  itk::Vector<double,3> spacing = ShCoeffImage->GetSpacing();
  double minSpacing = spacing[0];
  if (spacing[1]<minSpacing)
    minSpacing = spacing[1];
  if (spacing[2]<minSpacing)
    minSpacing = spacing[2];

  mitk::Point3D origin = ShCoeffImage->GetOrigin();
  itk::Matrix<double, 3, 3> direction = ShCoeffImage->GetDirection();
  ImageRegion<3> imageRegion = ShCoeffImage->GetLargestPossibleRegion();

  if (m_MaskImage.IsNotNull())
  {
    origin = m_MaskImage->GetOrigin();
    direction = m_MaskImage->GetDirection();
    imageRegion = m_MaskImage->GetLargestPossibleRegion();
  }

  itk::Vector<double, 3> spacing3 = ShCoeffImage->GetSpacing();
  itk::Point<float, 3> origin3 = ShCoeffImage->GetOrigin();
  itk::Matrix<double, 3, 3> direction3 = ShCoeffImage->GetDirection();
  itk::ImageRegion<3> imageRegion3 = ShCoeffImage->GetLargestPossibleRegion();

  itk::Vector<double, 4> spacing4;
  itk::Point<float, 4> origin4;
  itk::Matrix<double, 4, 4> direction4;
  itk::ImageRegion<4> imageRegion4;

  spacing4[0] = spacing3[0]; spacing4[1] = spacing3[1]; spacing4[2] = spacing3[2]; spacing4[3] = 1;
  origin4[0] = origin3[0]; origin4[1] = origin3[1]; origin4[2] = origin3[2]; origin4[3] = 0;
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
      direction4[r][c] = direction3[r][c];
  direction4[3][3] = 1;
  imageRegion4.SetSize(0, imageRegion3.GetSize()[0]);
  imageRegion4.SetSize(1, imageRegion3.GetSize()[1]);
  imageRegion4.SetSize(2, imageRegion3.GetSize()[2]);
  imageRegion4.SetSize(3, m_MaxNumPeaks*3);

  m_PeakImage = PeakImageType::New();
  m_PeakImage->SetSpacing( spacing4 );
  m_PeakImage->SetOrigin( origin4 );
  m_PeakImage->SetDirection( direction4 );
  m_PeakImage->SetRegions( imageRegion4 );
  m_PeakImage->Allocate();
  m_PeakImage->FillBuffer(0.0);

  if (m_MaskImage.IsNull())
  {
    m_MaskImage = ItkUcharImgType::New();
    m_MaskImage->SetSpacing( spacing );
    m_MaskImage->SetOrigin( origin );
    m_MaskImage->SetDirection( direction );
    m_MaskImage->SetRegions( imageRegion );
    m_MaskImage->Allocate();
    m_MaskImage->FillBuffer(1);
  }
  m_NumDirectionsImage = ItkUcharImgType::New();
  m_NumDirectionsImage->SetSpacing( spacing );
  m_NumDirectionsImage->SetOrigin( origin );
  m_NumDirectionsImage->SetDirection( direction );
  m_NumDirectionsImage->SetRegions( imageRegion );
  m_NumDirectionsImage->Allocate();
  m_NumDirectionsImage->FillBuffer(0);

  // calculate SH basis
  OdfType odf;
  vnl_matrix< double > sphCoords;
  std::vector< DirectionType > dirs;
  for (int i=0; i<NrOdfDirections; i++)
  {
    DirectionType odf_dir;
    odf_dir[0] = odf.GetDirection(i)[0];
    odf_dir[1] = odf.GetDirection(i)[1];
    odf_dir[2] = odf.GetDirection(i)[2];
    dirs.push_back(odf_dir);
  }
  CreateDirMatrix(dirs, sphCoords);                          // convert candidate peaks to spherical angles
  if (m_Toolkit==Toolkit::MRTRIX)
    m_ShBasis = mitk::sh::CalcShBasisForDirections(ShOrder, sphCoords);
  else
    m_ShBasis = mitk::sh::CalcShBasisForDirections(ShOrder, sphCoords, false);

  MITK_INFO << "Starting finite differences maximum extraction";
  MITK_INFO << "ODF sampling points: " << NrOdfDirections;
  MITK_INFO << "SH order: " << ShOrder;
  MITK_INFO << "Maximum peaks: " << m_MaxNumPeaks;
  MITK_INFO << "Relative threshold: " << m_PeakThreshold;
  MITK_INFO << "Absolute threshold: " << m_AbsolutePeakThreshold;
  MITK_INFO << "Clustering threshold: " << m_ClusteringThreshold;
  MITK_INFO << "Angular threshold: " << m_AngularThreshold;
}

template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::AfterThreadedGenerateData()
{

}

template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadID )
{
  typename CoefficientImageType::Pointer ShCoeffImage = static_cast< CoefficientImageType* >( this->ProcessObject::GetInput(0) );

  ImageRegionConstIterator< CoefficientImageType > cit(ShCoeffImage, outputRegionForThread );

  OdfType odf;
  while( !cit.IsAtEnd() )
  {
    typename CoefficientImageType::IndexType idx3 = cit.GetIndex();
    if (m_MaskImage->GetPixel(idx3)==0)
    {
      ++cit;
      continue;
    }

    CoefficientPixelType c = cit.Get();

    // calculate ODF
    double max = 0;
    odf.Fill(0.0);
    for (int i=0; i<NrOdfDirections; i++)
    {
      for (int j=0; j<m_NumCoeffs; j++)
        odf[i] += c[j]*m_ShBasis(i,j);
      if (odf[i]>max)
        max = odf[i];
    }
    if (max<0.0001)
    {
      ++cit;
      continue;
    }

    std::vector< DirectionType > candidates, peaks, temp;
    peaks.clear();
    max *= m_PeakThreshold;                         // relative threshold
    FindCandidatePeaks(odf, max, candidates);       // find all local maxima
    candidates = MeanShiftClustering(candidates);   // cluster maxima

    vnl_matrix<double> sphCoords;
    CreateDirMatrix(candidates, sphCoords);                // convert candidate peaks to spherical angles
    vnl_matrix< float > shBasis;
    if (m_Toolkit==Toolkit::MRTRIX)
      shBasis = mitk::sh::CalcShBasisForDirections(ShOrder, sphCoords);
    else
      shBasis = mitk::sh::CalcShBasisForDirections(ShOrder, sphCoords, false);

    max = 0.0;
    for (unsigned int i=0; i<candidates.size(); i++)         // scale peaks according to ODF value
    {
      double val = 0;
      for (int j=0; j<m_NumCoeffs; j++)
        val += c[j]*shBasis(i,j);
      if (val>max)
        max = val;
      peaks.push_back(candidates[i]*val);
    }
    std::sort( peaks.begin(), peaks.end(), CompareVectors );  // sort peaks

    // kick out directions to close to a larger direction (too far away to cluster but too close to keep)
    unsigned int m = peaks.size();
    if ( m>m_MaxNumPeaks )
      m = m_MaxNumPeaks;
    for (unsigned int i=0; i<m; i++)
    {
      DirectionType v1 = peaks.at(i);
      double val = v1.magnitude();
      if (val<max*m_PeakThreshold || val<m_AbsolutePeakThreshold)
        break;

      bool flag = true;
      for (unsigned int j=0; j<peaks.size(); j++)
        if (i!=j)
        {
          DirectionType v2 = peaks.at(j);
          double val2 = v2.magnitude();
          double angle = fabs(dot_product(v1,v2)/(val*val2));
          if (angle>m_AngularThreshold && val<val2)
          {
            flag = false;
            break;
          }
        }

      if (flag)
        temp.push_back(v1);
    }
    peaks = temp;

    itk::Index<4> idx4; idx4[0] = idx3[0]; idx4[1] = idx3[1]; idx4[2] = idx3[2];

    // fill output image
    unsigned int num = peaks.size();
    if ( num>m_MaxNumPeaks )
      num = m_MaxNumPeaks;
    for (unsigned int i=0; i<num; i++)
    {
      DirectionType dir = peaks.at(i);
      switch (m_NormalizationMethod)
      {
      case NO_NORM:
        break;
      case SINGLE_VEC_NORM:
        dir.normalize();
        break;
      case MAX_VEC_NORM:
        dir /= max;
        break;
      }

      if (m_ApplyDirectionMatrix)
        dir = m_MaskImage->GetDirection()*dir;

      if (m_FlipX)
        dir[0] = -dir[0];
      if (m_FlipY)
        dir[1] = -dir[1];
      if (m_FlipZ)
        dir[2] = -dir[2];

      for (unsigned int j = 0; j<3; j++)
      {
        idx4[3] = i*3 + j;
        m_PeakImage->SetPixel(idx4, dir[j]);
      }
    }
    m_NumDirectionsImage->SetPixel(idx3, num);
    ++cit;
  }
  MITK_INFO << "Thread " << threadID << " finished extraction";
}

// convert cartesian to spherical coordinates
template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::CreateDirMatrix(const std::vector< DirectionType >& dir, vnl_matrix<double>& sphCoords)
{
  sphCoords.set_size(3, dir.size());
  for (unsigned int i=0; i<dir.size(); i++)
  {
    sphCoords(0, i) = dir[i](0);
    sphCoords(1, i) = dir[i](1);
    sphCoords(2, i) = dir[i](2);
  }
}

}

#endif // __itkFiniteDiffOdfMaximaExtractionFilter_cpp
