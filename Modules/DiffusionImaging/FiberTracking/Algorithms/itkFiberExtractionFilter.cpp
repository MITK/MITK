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

#ifndef __itkFiberExtractionFilter_cpp
#define __itkFiberExtractionFilter_cpp

#include "itkFiberExtractionFilter.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <boost/progress.hpp>
#include <mitkDiffusionFunctionCollection.h>
#include <boost/lexical_cast.hpp>

namespace itk{


template< class PixelType >
FiberExtractionFilter< PixelType >::FiberExtractionFilter()
  : m_DontResampleFibers(false)
  , m_Mode(MODE::OVERLAP)
  , m_InputType(INPUT::SCALAR_MAP)
  , m_BothEnds(true)
  , m_OverlapFraction(0.8)
  , m_NoNegatives(false)
  , m_NoPositives(false)
  , m_Interpolate(false)
  , m_Threshold(0.5)
  , m_Labels()
  , m_SkipSelfConnections(false)
  , m_OnlySelfConnections(false)
  , m_SplitByRoi(false)
  , m_SplitLabels(false)
  , m_MinFibersPerTract(0)
  , m_PairedStartEndLabels(false)
{
  m_Interpolator = itk::LinearInterpolateImageFunction< itk::Image< PixelType, 3 >, float >::New();
}

template< class PixelType >
FiberExtractionFilter< PixelType >::~FiberExtractionFilter()
{

}

template< class PixelType >
void FiberExtractionFilter< PixelType >::SetRoiImages(const std::vector<ItkInputImgType*> &rois)
{
  for (auto roi : rois)
  {
    if (roi==nullptr)
    {
      MITK_INFO << "ROI image is NULL!";
      return;
    }
  }
  m_RoiImages = rois;
}

template< class PixelType >
void FiberExtractionFilter< PixelType >::SetRoiImageNames(const std::vector< std::string > roi_names)
{
  m_RoiImageNames = roi_names;
}

template< class PixelType >
mitk::FiberBundle::Pointer FiberExtractionFilter< PixelType >::CreateFib(std::vector< unsigned int >& ids)
{
  vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();
  vtkSmartPointer<vtkPolyData> pTmp = m_InputFiberBundle->GeneratePolyDataByIds(ids, weights);
  mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(pTmp);
  fib->SetFiberWeights(weights);
  return fib;
}

template< class PixelType >
bool FiberExtractionFilter< PixelType >::IsPositive(const itk::Point<float, 3>& itkP)
{
  if( m_InputType == INPUT::SCALAR_MAP )
    return mitk::imv::IsInsideMask<PixelType>(itkP, m_Interpolate, m_Interpolator, m_Threshold);
  else if( m_InputType == INPUT::LABEL_MAP )
  {
    auto val = mitk::imv::GetImageValue<PixelType>(itkP, false, m_Interpolator);
    for (auto l : m_Labels)
      if (l==val)
        return true;
  }
  else
    mitkThrow() << "No valid input type selected!";
  return false;
}

template< class PixelType >
std::vector< std::string > FiberExtractionFilter< PixelType >::GetPositiveLabels() const
{
  return m_PositiveLabels;
}

template< class PixelType >
void FiberExtractionFilter< PixelType >::ExtractOverlap(mitk::FiberBundle::Pointer fib)
{
  MITK_INFO << "Extracting fibers (min. overlap " << m_OverlapFraction << ")";
  vtkSmartPointer<vtkPolyData> polydata = fib->GetFiberPolyData();

  std::vector< std::vector< unsigned int > > positive_ids;  // one ID vector per ROI
  positive_ids.resize(m_RoiImages.size());

  std::vector< unsigned int > negative_ids; // fibers not overlapping with ANY mask

  boost::progress_display disp(m_InputFiberBundle->GetNumFibers());
  for (unsigned int i=0; i<m_InputFiberBundle->GetNumFibers(); i++)
  {
    ++disp;
    vtkCell* cell = polydata->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    float best_ol = 0;
    int best_ol_idx = -1;
    for (unsigned int m=0; m<m_RoiImages.size(); ++m)
    {
      auto roi = m_RoiImages.at(m);
      m_Interpolator->SetInputImage(roi);
      PixelType inside = 0;
      PixelType outside = 0;
      for (int j=0; j<numPoints-1; j++)
      {
        itk::Point<float, 3> startVertex = mitk::imv::GetItkPoint(points->GetPoint(j));
        itk::Index<3> startIndex;
        itk::ContinuousIndex<float, 3> startIndexCont;
        roi->TransformPhysicalPointToIndex(startVertex, startIndex);
        roi->TransformPhysicalPointToContinuousIndex(startVertex, startIndexCont);

        itk::Point<float, 3> endVertex = mitk::imv::GetItkPoint(points->GetPoint(j + 1));
        itk::Index<3> endIndex;
        itk::ContinuousIndex<float, 3> endIndexCont;
        roi->TransformPhysicalPointToIndex(endVertex, endIndex);
        roi->TransformPhysicalPointToContinuousIndex(endVertex, endIndexCont);

        std::vector< std::pair< itk::Index<3>, double > > segments = mitk::imv::IntersectImage(roi->GetSpacing(), startIndex, endIndex, startIndexCont, endIndexCont);
        for (std::pair< itk::Index<3>, double > segment : segments)
        {
          if (!roi->GetLargestPossibleRegion().IsInside(segment.first))
            continue;
          if (roi->GetPixel(segment.first)>=m_Threshold)
            inside += segment.second;
          else
            outside += segment.second;
        }
      }

      float overlap = (float)inside/(inside+outside);
      if (overlap > best_ol && overlap >= m_OverlapFraction)
      {
        best_ol_idx = m;
        best_ol = overlap;
      }
    }
    if (best_ol_idx<0)
      negative_ids.push_back(i);
    else
      positive_ids.at(best_ol_idx).push_back(i);
  }

  if (!m_NoNegatives)
    m_Negatives.push_back(CreateFib(negative_ids));
  if (!m_NoPositives)
  {
    for (unsigned int i=0; i<positive_ids.size(); ++i)
    {
      std::string name;
      if (i<m_RoiImageNames.size())
        name = m_RoiImageNames.at(i);
      else
        name = "Roi" + boost::lexical_cast<std::string>(i);

      if (positive_ids.at(i).size()>=m_MinFibersPerTract)
      {
        m_Positives.push_back(CreateFib(positive_ids.at(i)));
        m_PositiveLabels.push_back(name);
      }
    }
    if (!m_SplitByRoi)
    {
      mitk::FiberBundle::Pointer output = mitk::FiberBundle::New(nullptr);
      output = output->AddBundles(m_Positives);
      m_Positives.clear();
      m_Positives.push_back(output);
      m_PositiveLabels.clear();
      m_PositiveLabels.push_back("");
    }
  }
}

template< class PixelType >
void FiberExtractionFilter< PixelType >::ExtractEndpoints(mitk::FiberBundle::Pointer fib)
{
  MITK_INFO << "Extracting fibers (endpoints in mask)";
  vtkSmartPointer<vtkPolyData> polydata = fib->GetFiberPolyData();

  std::vector< std::vector< unsigned int > > positive_ids;  // one ID vector per ROI
  positive_ids.resize(m_RoiImages.size());

  std::vector< unsigned int > negative_ids; // fibers not overlapping with ANY mask

  boost::progress_display disp(m_InputFiberBundle->GetNumFibers());
  for (unsigned int i=0; i<m_InputFiberBundle->GetNumFibers(); i++)
  {
    ++disp;
    vtkCell* cell = polydata->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    bool positive = false;
    if (numPoints>1)
      for (unsigned int m=0; m<m_RoiImages.size(); ++m)
      {
        auto roi = m_RoiImages.at(m);
        m_Interpolator->SetInputImage(roi);

        int inside = 0;

        // check first fiber point
        {
          double* p = points->GetPoint(0);
          itk::Point<float, 3> itkP = mitk::imv::GetItkPoint(p);

          if ( IsPositive(itkP) )
            inside++;
        }

        // check second fiber point
        {
          double* p = points->GetPoint(numPoints-1);
          itk::Point<float, 3> itkP = mitk::imv::GetItkPoint(p);

          if ( IsPositive(itkP) )
            inside++;
        }

        if (inside==2 || (inside==1 && !m_BothEnds))
        {
          positive = true;
          positive_ids[m].push_back(i);
        }
      }
    if (!positive)
      negative_ids.push_back(i);
  }

  if (!m_NoNegatives)
    m_Negatives.push_back(CreateFib(negative_ids));
  if (!m_NoPositives)
  {
    for (unsigned int i=0; i<positive_ids.size(); ++i)
    {
      std::string name;
      if (i<m_RoiImageNames.size())
        name = m_RoiImageNames.at(i);
      else
        name = "Roi" + boost::lexical_cast<std::string>(i);

      if (positive_ids.at(i).size()>=m_MinFibersPerTract)
      {
        m_Positives.push_back(CreateFib(positive_ids.at(i)));
        m_PositiveLabels.push_back(name);
      }
    }
    if (!m_SplitByRoi)
    {
      mitk::FiberBundle::Pointer output = mitk::FiberBundle::New(nullptr);
      output = output->AddBundles(m_Positives);
      m_Positives.clear();
      m_Positives.push_back(output);
      m_PositiveLabels.clear();
      m_PositiveLabels.push_back("");
    }
  }
}

template< class PixelType >
void FiberExtractionFilter< PixelType >::ExtractLabels(mitk::FiberBundle::Pointer fib)
{
  MITK_INFO << "Extracting fibers by labels";
  vtkSmartPointer<vtkPolyData> polydata = fib->GetFiberPolyData();

  std::map< std::string, std::vector< unsigned int > > positive_ids;

  std::vector< unsigned int > negative_ids; // fibers not overlapping with ANY label

  boost::progress_display disp(m_InputFiberBundle->GetNumFibers());
  for (unsigned int i=0; i<m_InputFiberBundle->GetNumFibers(); i++)
  {
    ++disp;
    vtkCell* cell = polydata->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    bool positive = false;
    if (numPoints>1)
      for (unsigned int m=0; m<m_RoiImages.size(); ++m)
      {
        auto roi = m_RoiImages.at(m);
        m_Interpolator->SetInputImage(roi);

        int inside = 0;

        double* p1 = points->GetPoint(0);
        itk::Point<float, 3> itkP1 = mitk::imv::GetItkPoint(p1);
        short label1 = mitk::imv::GetImageValue<PixelType>(itkP1, false, m_Interpolator);

        double* p2 = points->GetPoint(numPoints-1);
        itk::Point<float, 3> itkP2 = mitk::imv::GetItkPoint(p2);
        short label2 = mitk::imv::GetImageValue<PixelType>(itkP2, false, m_Interpolator);

        if (!m_Labels.empty())  // extract fibers from all pairwise label combinations
        {
          for (auto l : m_Labels)
          {
            if (l==label1)
              inside++;
            if (l==label2)
              inside++;
            if (inside==2)
              break;
          }
        }
        else if (!m_StartLabels.empty() || !m_EndLabels.empty()) // extract fibers between start and end labels
        {
          if (!m_StartLabels.empty() && !m_EndLabels.empty())
            m_BothEnds = true;  // if we have start and end labels it does not make sense to not use both endpoints
          if (m_PairedStartEndLabels)
          {
            if (m_StartLabels.size()!=m_EndLabels.size())
              mitkThrow() << "Start and end label lists must have same size if paired labels are used";
            for (unsigned int ii=0; ii<m_StartLabels.size(); ++ii)
              if ( (m_StartLabels.at(ii)==label1 && m_EndLabels.at(ii)==label2) || (m_StartLabels.at(ii)==label2 && m_EndLabels.at(ii)==label1) )
              {
                inside = 2;
                break;
              }
          }
          else
          {
            for (unsigned int ii=0; ii<m_StartLabels.size(); ++ii)
            {
              if ( m_StartLabels.at(ii)==label1 || m_StartLabels.at(ii)==label2 )
              {
                ++inside;
                break;
              }
            }
            for (unsigned int jj=0; jj<m_EndLabels.size(); ++jj)
            {
              if ( m_EndLabels.at(jj)==label1 || m_EndLabels.at(jj)==label2 )
              {
                ++inside;
                break;
              }
            }
          }
        }
        else  // use all labels
        {
          if (label1!=0)
            ++inside;
          if (label2!=0)
            ++inside;
        }

        std::string key = "";

        if (m_SplitLabels)
        {
          if (label1<label2)
            key = boost::lexical_cast<std::string>(label1) + "-" + boost::lexical_cast<std::string>(label2);
          else
            key = boost::lexical_cast<std::string>(label2) + "-" + boost::lexical_cast<std::string>(label1);
        }

        if (m_SplitByRoi)
        {
          if (m<m_RoiImageNames.size())
            key = m_RoiImageNames.at(m) + "_" + key;
          else
            key = "Roi" + boost::lexical_cast<std::string>(m) + "_" + key;
        }

        if (m_BothEnds)
        {
          if ( (inside==2 && (!m_SkipSelfConnections || label1!=label2)) || (inside==2 && m_OnlySelfConnections && label1==label2) )
          {
            positive = true;
            if ( positive_ids.count(key)==0 )
              positive_ids.insert( std::pair< std::string, std::vector< unsigned int > >( key, {i}) );
            else
              positive_ids[key].push_back(i);
          }
        }
        else
        {
          if ( (inside>=1 && (!m_SkipSelfConnections || label1!=label2)) || (inside==2 && m_OnlySelfConnections && label1==label2) )
          {
            positive = true;
            if ( positive_ids.count(key)==0 )
              positive_ids.insert( std::pair< std::string, std::vector< unsigned int > >( key, {i}) );
            else
              positive_ids[key].push_back(i);
          }
        }
      }
    if (!positive)
      negative_ids.push_back(i);
  }

  if (!m_NoNegatives)
    m_Negatives.push_back(CreateFib(negative_ids));
  if (!m_NoPositives)
  {
    for (auto label : positive_ids)
    {
      if (label.second.size()>=m_MinFibersPerTract)
      {
        m_Positives.push_back(CreateFib(label.second));
        m_PositiveLabels.push_back(label.first);
      }
    }
  }
}

template< class PixelType >
void FiberExtractionFilter< PixelType >::SetLabels(const std::vector<unsigned short> &Labels)
{
  m_Labels = Labels;
}

template< class PixelType >
void FiberExtractionFilter< PixelType >::SetStartLabels(const std::vector<unsigned short> &Labels)
{
  m_StartLabels = Labels;
}

template< class PixelType >
void FiberExtractionFilter< PixelType >::SetEndLabels(const std::vector<unsigned short> &Labels)
{
  m_EndLabels = Labels;
}

template< class PixelType >
std::vector<mitk::FiberBundle::Pointer> FiberExtractionFilter< PixelType >::GetNegatives() const
{
  return m_Negatives;
}

template< class PixelType >
std::vector<mitk::FiberBundle::Pointer> FiberExtractionFilter< PixelType >::GetPositives() const
{
  return m_Positives;
}

template< class PixelType >
void FiberExtractionFilter< PixelType >::GenerateData()
{
  mitk::FiberBundle::Pointer fib = m_InputFiberBundle;
  if (fib->GetNumFibers()<=0)
  {
    MITK_INFO << "No fibers in tractogram!";
    return;
  }

  if (m_Mode == MODE::OVERLAP)
    ExtractOverlap(fib);
  else if (m_Mode == MODE::ENDPOINTS)
  {
    if (m_InputType==INPUT::LABEL_MAP)
      ExtractLabels(fib);
    else
      ExtractEndpoints(fib);
  }
}

}

#endif // __itkFiberExtractionFilter_cpp



