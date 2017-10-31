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
#include "itkTractClusteringFilter.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <boost/progress.hpp>

namespace itk{

TractClusteringFilter::TractClusteringFilter()
  : m_NumPoints(12)
  , m_MinClusterSize(1)
  , m_MaxClusters(0)
  , m_Metric(Metric::MDF)
{

}

TractClusteringFilter::~TractClusteringFilter()
{

}

void TractClusteringFilter::SetMetric(const Metric &Metric)
{
  m_Metric = Metric;
}

std::vector<mitk::FiberBundle::Pointer> TractClusteringFilter::GetOutTractograms() const
{
  return m_OutTractograms;
}

void TractClusteringFilter::SetDistances(const std::vector<float> &Distances)
{
  m_Distances = Distances;
}

float TractClusteringFilter::CalcMDF(vnl_matrix<float>& s, vnl_matrix<float>& t, bool& flipped)
{
  float d_direct = 0;
  float d_flipped = 0;

  for (unsigned int i=0; i<m_NumPoints; ++i)
  {
    d_direct += (s.get_column(i)-t.get_column(i)).magnitude();
    d_flipped += (s.get_column(i)-t.get_column(m_NumPoints-i-1)).magnitude();
  }

  if (d_direct>d_flipped)
  {
    flipped = true;
    return d_flipped/m_NumPoints;
  }
  flipped = false;
  return d_direct/m_NumPoints;
}

float TractClusteringFilter::CalcMDF_VAR(vnl_matrix<float>& s, vnl_matrix<float>& t, bool& flipped)
{
  float d_direct = 0;
  float d_flipped = 0;

  vnl_vector<float> dists_d; dists_d.set_size(m_NumPoints);
  vnl_vector<float> dists_f; dists_f.set_size(m_NumPoints);

  for (unsigned int i=0; i<m_NumPoints; ++i)
  {
    dists_d[i] = (s.get_column(i)-t.get_column(i)).magnitude();
    d_direct += dists_d[i];

    dists_f[i] = (s.get_column(i)-t.get_column(m_NumPoints-i-1)).magnitude();
    d_flipped += dists_f[i];
  }

  if (d_direct>d_flipped)
  {
    float d = d_flipped/m_NumPoints;
    dists_f -= d;
    d += dists_f.magnitude();

    flipped = true;
    return d;
  }

  float d = d_direct/m_NumPoints;
  dists_d -= d;
  d += dists_d.magnitude();

  flipped = false;
  return d;
}

std::vector<vnl_matrix<float> > TractClusteringFilter::ResampleFibers()
{
  mitk::FiberBundle::Pointer temp_fib = m_Tractogram->GetDeepCopy();
  temp_fib->ResampleToNumPoints(m_NumPoints);

  std::vector< vnl_matrix<float> > out_fib;

  for (int i=0; i<temp_fib->GetFiberPolyData()->GetNumberOfCells(); i++)
  {
    vtkCell* cell = temp_fib->GetFiberPolyData()->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vnl_matrix<float> streamline;
    streamline.set_size(3, m_NumPoints);
    streamline.fill(0.0);

    for (int j=0; j<numPoints; j++)
    {
      double cand[3];
      points->GetPoint(j, cand);
      vnl_vector_fixed< float, 3 > candV;
      candV[0]=cand[0]; candV[1]=cand[1]; candV[2]=cand[2];

      streamline.set_column(j, candV);
    }

    out_fib.push_back(streamline);
  }

  return out_fib;
}

std::vector< TractClusteringFilter::Cluster > TractClusteringFilter::ClusterStep(std::vector< long > f_indices, std::vector<float> distances)
{
  float dist_thres = distances.back();
  distances.pop_back();
  std::vector< Cluster > C;

  int N = f_indices.size();

  Cluster c1;
  c1.I.push_back(f_indices.at(0));
  c1.h = T[f_indices.at(0)];
  c1.n = 1;
  C.push_back(c1);
  if (f_indices.size()==1)
    return C;

  MITK_INFO << "Clustering fibers with distance " << dist_thres;
  for (int i=1; i<N; ++i)
  {
    vnl_matrix<float> t = T.at(f_indices.at(i));

    int min_cluster_index = -1;
    float min_cluster_distance = 99999;
    bool flip = false;

    for (unsigned int k=0; k<C.size(); ++k)
    {
      vnl_matrix<float> v = C.at(k).h / C.at(k).n;
      bool f = false;
      float d = 0;
      if (m_Metric==Metric::MDF)
        d = CalcMDF(t, v, f);
      else if (m_Metric==Metric::MDF_VAR)
        d = CalcMDF_VAR(t, v, f);

      if (d<min_cluster_distance)
      {
        min_cluster_distance = d;
        min_cluster_index = k;
        flip = f;
      }
    }

    if (min_cluster_index>=0 && min_cluster_distance<dist_thres)
    {
      C[min_cluster_index].I.push_back(f_indices.at(i));
      if (!flip)
        C[min_cluster_index].h += t;
      else
        C[min_cluster_index].h += t.fliplr();
      C[min_cluster_index].n += 1;
    }
    else
    {
      Cluster c;
      c.I.push_back(f_indices.at(i));
      c.h = t;
      c.n = 1;
      C.push_back(c);
    }
  }

  if (!distances.empty())
  {
    std::vector< Cluster > outC;
#pragma omp parallel for
    for (int c=0; c<(int)C.size(); c++)
    {

      std::vector< Cluster > tempC = ClusterStep(C.at(c).I, distances);
      AppendCluster(outC, tempC);
    }
    return outC;
  }
  else
    return C;
}

void TractClusteringFilter::AppendCluster(std::vector< Cluster >& a, std::vector< Cluster >&b)
{
  for (auto c : b)
    a.push_back(c);
}

void TractClusteringFilter::GenerateData()
{
  m_OutTractograms.clear();

  T = ResampleFibers();

  std::vector< long > f_indices;
  for (unsigned int i=0; i<T.size(); ++i)
    f_indices.push_back(i);
//  std::random_shuffle(f_indices.begin(), f_indices.end());

  std::vector< Cluster > C = ClusterStep(f_indices, m_Distances);

  std::sort(C.begin(),C.end());

  MITK_INFO << "Clustering finished";
  int max = C.size()-1;
  if (m_MaxClusters>0 && C.size()-1>m_MaxClusters)
    max = m_MaxClusters;
  for (int i=C.size()-1; i>=0; --i)
  {
    Cluster c = C.at(i);
    if (c.n<(int)m_MinClusterSize)
      break;
    vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();
    vtkSmartPointer<vtkPolyData> pTmp = m_Tractogram->GeneratePolyDataByIds(c.I, weights);
    mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(pTmp);
    if (max>0)
      fib->SetFiberWeights((float)i/max);
    m_OutTractograms.push_back(fib);

    if (m_MaxClusters>0 && C.size()-i>=m_MaxClusters)
      break;
  }

  int w = 0;
  for (auto fib : m_OutTractograms)
  {
    if (m_OutTractograms.size()>1)
      fib->SetFiberWeights((float)w/(m_OutTractograms.size()-1));
    else
      fib->SetFiberWeights(1);
    fib->ColorFibersByFiberWeights(false, false);
    ++w;
  }

}

}




