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
  , m_InCentroids(nullptr)
  , m_MinClusterSize(1)
  , m_MaxClusters(0)
  , m_Metric(Metric::MDF)
  , m_ScalarMap(nullptr)
  , m_Scale(0)
  , m_MergeDuplicateThreshold(-1)
{

}

TractClusteringFilter::~TractClusteringFilter()
{

}

std::vector<TractClusteringFilter::Cluster> TractClusteringFilter::GetOutClusters() const
{
  return m_OutClusters;
}

std::vector<mitk::FiberBundle::Pointer> TractClusteringFilter::GetOutCentroids() const
{
  return m_OutCentroids;
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
    return d/2;
  }

  float d = d_direct/m_NumPoints;
  dists_d -= d;
  d += dists_d.magnitude();

  flipped = false;
  return d/2;
}

float TractClusteringFilter::CalcMAX_MDF(vnl_matrix<float>& s, vnl_matrix<float>& t, bool& flipped)
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
    float d = dists_f.max_value();
    flipped = true;
    return d;
  }

  float d = dists_d.max_value();
  flipped = false;
  return d;
}

std::vector<vnl_matrix<float> > TractClusteringFilter::ResampleFibers(mitk::FiberBundle::Pointer tractogram)
{
  mitk::FiberBundle::Pointer temp_fib = tractogram->GetDeepCopy();
  temp_fib->ResampleToNumPoints(m_NumPoints);

  std::vector< vnl_matrix<float> > out_fib;

  for (int i=0; i<temp_fib->GetFiberPolyData()->GetNumberOfCells(); i++)
  {
    vtkCell* cell = temp_fib->GetFiberPolyData()->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vnl_matrix<float> streamline;
    if (m_ScalarMap.IsNull())
      streamline.set_size(3, m_NumPoints);
    else
      streamline.set_size(4, m_NumPoints);
    streamline.fill(0.0);

    for (int j=0; j<numPoints; j++)
    {
      double cand[3];
      points->GetPoint(j, cand);

      if (m_ScalarMap.IsNull())
      {
        vnl_vector_fixed< float, 3 > candV;
        candV[0]=cand[0]; candV[1]=cand[1]; candV[2]=cand[2];
        streamline.set_column(j, candV);
      }
      else
      {
        vnl_vector_fixed< float, 4 > candV;
        candV[0]=cand[0]; candV[1]=cand[1]; candV[2]=cand[2]; candV[3]=0;
        itk::Point<float,3> wp; wp[0]=cand[0]; wp[1]=cand[1]; wp[2]=cand[2];

        itk::Index<3> idx;
        m_ScalarMap->TransformPhysicalPointToIndex(wp, idx);
        if (m_ScalarMap->GetLargestPossibleRegion().IsInside(idx))
          candV[3]=m_ScalarMap->GetPixel(idx)*m_Scale;

        streamline.set_column(j, candV);
      }
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
      else if (m_Metric==Metric::MAX_MDF)
        d = CalcMAX_MDF(t, v, f);

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

void TractClusteringFilter::MergeDuplicateClusters(std::vector< TractClusteringFilter::Cluster >& clusters)
{
  if (m_MergeDuplicateThreshold<0)
    m_MergeDuplicateThreshold = m_Distances.at(0);
  bool found = true;

  MITK_INFO << "Merging duplicate clusters with distance threshold " << m_MergeDuplicateThreshold;
  while (found && m_MergeDuplicateThreshold>mitk::eps)
  {
    found = false;
    for (unsigned int k1=0; k1<clusters.size(); ++k1)
    {
      Cluster c1 = clusters.at(k1);
      vnl_matrix<float> t = c1.h / c1.n;

      int min_cluster_index = -1;
      float min_cluster_distance = 99999;
      bool flip = false;

      for (unsigned int k2=0; k2<clusters.size(); ++k2)
      {
        if (k1==k2)
          continue;

        Cluster c2 = clusters.at(k2);
        vnl_matrix<float> v = c2.h / c2.n;
        bool f = false;
        float d = 0;

        if (m_Metric==Metric::MDF)
          d = CalcMDF(t, v, f);
        else if (m_Metric==Metric::MDF_VAR)
          d = CalcMDF_VAR(t, v, f);
        else if (m_Metric==Metric::MAX_MDF)
          d = CalcMAX_MDF(t, v, f);

        if (d<min_cluster_distance)
        {
          min_cluster_distance = d;
          min_cluster_index = k2;
          flip = f;
        }
      }

      if (min_cluster_index>=0 && min_cluster_distance<m_MergeDuplicateThreshold)
      {
        Cluster c2 = clusters.at(min_cluster_index);
        for (int i=0; i<c2.n; ++i)
        {
          clusters[k1].I.push_back(c2.I.at(i));
          clusters[k1].n += 1;
        }
        if (!flip)
          clusters[k1].h += c2.h;
        else
          clusters[k1].h += c2.h.fliplr();
        clusters.erase (clusters.begin()+min_cluster_index);
        found = true;
        break;
      }
    }
  }
  MITK_INFO << "Number of clusters after merging duplicates: " << clusters.size();
}

std::vector<TractClusteringFilter::Cluster> TractClusteringFilter::AddToKnownClusters(std::vector< long > f_indices, std::vector<vnl_matrix<float> >& centroids)
{
  float dist_thres = m_Distances.at(0);
  int N = f_indices.size();

  std::vector< Cluster > C;
  vnl_matrix<float> zero_h; zero_h.set_size(T.at(0).rows(), T.at(0).cols()); zero_h.fill(0.0);
  Cluster no_fit;
  no_fit.h = zero_h;
  for (unsigned int i=0; i<centroids.size(); ++i)
  {
    Cluster c;
    c.h.set_size(T.at(0).rows(), T.at(0).cols()); c.h.fill(0.0);
    c.f_id = i;
    C.push_back(c);
  }

  for (int i=0; i<N; ++i)
  {
    vnl_matrix<float> t = T.at(f_indices.at(i));

    int min_cluster_index = -1;
    float min_cluster_distance = 99999;
    bool flip = false;

    int c_idx = 0;
    for (vnl_matrix<float> centroid : centroids)
    {
      bool f = false;
      float d = 0;

      if (m_Metric==Metric::MDF)
        d = CalcMDF(t, centroid, f);
      else if (m_Metric==Metric::MDF_VAR)
        d = CalcMDF_VAR(t, centroid, f);
      else if (m_Metric==Metric::MAX_MDF)
        d = CalcMAX_MDF(t, centroid, f);

      if (d<min_cluster_distance)
      {
        min_cluster_distance = d;
        min_cluster_index = c_idx;
        flip = f;
      }
      ++c_idx;
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
      no_fit.I.push_back(f_indices.at(i));
      no_fit.n++;
    }
  }
  C.push_back(no_fit);
  return C;
}

void TractClusteringFilter::GenerateData()
{
  m_OutTractograms.clear();
  m_OutCentroids.clear();
  m_OutClusters.clear();

  if (m_Scale==0)
    m_Scale = m_Distances.at(0);
  MITK_INFO << "Scalar map scaling: " << m_Scale;

  T = ResampleFibers(m_Tractogram);

  std::vector< long > f_indices;
  for (unsigned int i=0; i<T.size(); ++i)
    f_indices.push_back(i);
  //  std::random_shuffle(f_indices.begin(), f_indices.end());

  Cluster no_match;
  std::vector< Cluster > clusters;
  if (m_InCentroids.IsNull())
  {
    clusters = ClusterStep(f_indices, m_Distances);
    MITK_INFO << "Number of clusters: " << clusters.size();
    MergeDuplicateClusters(clusters);
    std::sort(clusters.begin(),clusters.end());
  }
  else
  {
    std::vector<vnl_matrix<float> > centroids = ResampleFibers(m_InCentroids);
    clusters = AddToKnownClusters(f_indices, centroids);
    no_match = clusters.back();
    clusters.pop_back();
    MITK_INFO << "Number of clusters: " << clusters.size();
    MergeDuplicateClusters(clusters);
  }

  MITK_INFO << "Clustering finished";
  int max = clusters.size()-1;
  if (m_MaxClusters>0 && clusters.size()-1>m_MaxClusters)
    max = m_MaxClusters;
  for (int i=clusters.size()-1; i>=0; --i)
  {
    Cluster c = clusters.at(i);
    if ( c.n>=(int)m_MinClusterSize && !(m_MaxClusters>0 && clusters.size()-i>=m_MaxClusters) )
    {
      m_OutClusters.push_back(c);

      vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();
      vtkSmartPointer<vtkPolyData> pTmp = m_Tractogram->GeneratePolyDataByIds(c.I, weights);
      mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(pTmp);
      if (max>0)
        fib->SetFiberWeights((float)i/max);
      m_OutTractograms.push_back(fib);

      // create centroid
      vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
      vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();
      vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
      vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
      vnl_matrix<float> centroid_points = c.h / c.n;
      for (unsigned int j=0; j<centroid_points.cols(); j++)
      {
        double p[3];
        p[0] = centroid_points.get(0,j);
        p[1] = centroid_points.get(1,j);
        p[2] = centroid_points.get(2,j);

        vtkIdType id = vtkNewPoints->InsertNextPoint(p);
        container->GetPointIds()->InsertNextId(id);
      }
      vtkNewCells->InsertNextCell(container);
      polyData->SetPoints(vtkNewPoints);
      polyData->SetLines(vtkNewCells);
      mitk::FiberBundle::Pointer centroid = mitk::FiberBundle::New(polyData);
      centroid->SetFiberColors(255, 255, 255);
      m_OutCentroids.push_back(centroid);
    }
  }
  MITK_INFO << "Final number of clusters: " << m_OutTractograms.size();

  int w = 0;
  for (auto fib : m_OutTractograms)
  {
    if (m_OutTractograms.size()>1)
    {
      fib->SetFiberWeights((float)w/(m_OutTractograms.size()-1));
      m_OutCentroids.at(w)->SetFiberWeights((float)w/(m_OutTractograms.size()-1));
    }
    else
    {
      fib->SetFiberWeights(1);
      m_OutCentroids.at(w)->SetFiberWeights(1);
    }
    fib->ColorFibersByFiberWeights(false, false);
    ++w;
  }

  if (no_match.n>0)
  {
    vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();
    vtkSmartPointer<vtkPolyData> pTmp = m_Tractogram->GeneratePolyDataByIds(no_match.I, weights);
    mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(pTmp);
    fib->SetFiberColors(0, 0, 0);
    m_OutTractograms.push_back(fib);
  }
}

}




