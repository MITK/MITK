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
#include <vnl/vnl_sparse_matrix.h>

namespace itk{

TractClusteringFilter::TractClusteringFilter()
  : m_NumPoints(12)
  , m_InCentroids(nullptr)
  , m_MinClusterSize(1)
  , m_MaxClusters(0)
  , m_MergeDuplicateThreshold(-1)
  , m_DoResampling(true)
  , m_FilterMask(nullptr)
  , m_OverlapThreshold(0.0)
{

}

TractClusteringFilter::~TractClusteringFilter()
{
  for (auto m : m_Metrics)
    delete m;
}

std::vector<std::vector<unsigned int> > TractClusteringFilter::GetOutFiberIndices() const
{
  return m_OutFiberIndices;
}

void TractClusteringFilter::SetMetrics(const std::vector<mitk::ClusteringMetric *> &Metrics)
{
  m_Metrics = Metrics;
}

std::vector<TractClusteringFilter::Cluster> TractClusteringFilter::GetOutClusters() const
{
  return m_OutClusters;
}

std::vector<mitk::FiberBundle::Pointer> TractClusteringFilter::GetOutCentroids() const
{
  return m_OutCentroids;
}

std::vector<mitk::FiberBundle::Pointer> TractClusteringFilter::GetOutTractograms() const
{
  return m_OutTractograms;
}

void TractClusteringFilter::SetDistances(const std::vector<float> &Distances)
{
  m_Distances = Distances;
}

float TractClusteringFilter::CalcOverlap(vnl_matrix<float>& t)
{
  float overlap = 0;
  if (m_FilterMask.IsNotNull())
  {

    for (unsigned int i=0; i<m_NumPoints; ++i)
    {
      vnl_vector<float> p = t.get_column(i);
      itk::Point<float,3> point;
      point[0] = p[0];
      point[1] = p[1];
      point[2] = p[2];
      itk::Index<3> idx;
      m_FilterMask->TransformPhysicalPointToIndex(point, idx);
      if (m_FilterMask->GetLargestPossibleRegion().IsInside(idx) && m_FilterMask->GetPixel(idx)>0)
        overlap += 1;
    }
    overlap /= m_NumPoints;
  }
  else
    return 1.0;

  return overlap;
}

std::vector<vnl_matrix<float> > TractClusteringFilter::ResampleFibers(mitk::FiberBundle::Pointer tractogram)
{
  mitk::FiberBundle::Pointer temp_fib = tractogram->GetDeepCopy();
  if (m_DoResampling)
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

std::vector< TractClusteringFilter::Cluster > TractClusteringFilter::ClusterStep(std::vector< unsigned int > f_indices, std::vector<float> distances)
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
      for (auto m : m_Metrics)
        d += m->CalculateDistance(t, v, f);
      d /= m_Metrics.size();

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
#pragma omp critical
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

std::vector< TractClusteringFilter::Cluster > TractClusteringFilter::MergeDuplicateClusters2(std::vector< TractClusteringFilter::Cluster >& clusters)
{
  if (m_MergeDuplicateThreshold<0)
    m_MergeDuplicateThreshold = m_Distances.at(0)/2;
  else if (m_MergeDuplicateThreshold==0)
    return clusters;

  MITK_INFO << "Merging duplicate clusters with distance threshold " << m_MergeDuplicateThreshold;

  std::vector< TractClusteringFilter::Cluster > new_clusters;
  for (Cluster c1 : clusters)
  {
    vnl_matrix<float> t = c1.h / c1.n;

    int min_idx = -1;
    float min_d = 99999;
    bool flip = false;

#pragma omp parallel for
    for (int k2=0; k2<(int)new_clusters.size(); ++k2)
    {
      Cluster c2 = new_clusters.at(k2);
      vnl_matrix<float> v = c2.h / c2.n;

      bool f = false;
      float d = 0;
      for (auto m : m_Metrics)
        d += m->CalculateDistance(t, v, f);
      d /= m_Metrics.size();

#pragma omp critical
      if (d<min_d && d<m_MergeDuplicateThreshold)
      {
        min_d = d;
        min_idx = k2;
        flip = f;
      }
    }

    if (min_idx<0)
      new_clusters.push_back(c1);
    else
    {
      for (int i=0; i<c1.n; ++i)
      {
        new_clusters[min_idx].I.push_back(c1.I.at(i));
        new_clusters[min_idx].n += 1;
      }
      if (!flip)
        new_clusters[min_idx].h += c1.h;
      else
        new_clusters[min_idx].h += c1.h.fliplr();
    }
  }

  MITK_INFO << "\nNumber of clusters after merging duplicates: " << new_clusters.size();
  return new_clusters;
}

void TractClusteringFilter::MergeDuplicateClusters(std::vector< TractClusteringFilter::Cluster >& clusters)
{
  if (m_MergeDuplicateThreshold<0)
    m_MergeDuplicateThreshold = m_Distances.at(0)/2;
  bool found = true;


  MITK_INFO << "Merging duplicate clusters with distance threshold " << m_MergeDuplicateThreshold;
  int start = 0;
  while (found && m_MergeDuplicateThreshold>mitk::eps)
  {
    std::cout << "                                                                                                     \r";
    std::cout << "Number of clusters: " << clusters.size() << '\r';
    cout.flush();

    found = false;
    for (int k1=start; k1<(int)clusters.size(); ++k1)
    {
      Cluster c1 = clusters.at(k1);
      vnl_matrix<float> t = c1.h / c1.n;

      std::vector< int > merge_indices;
      std::vector< bool > flip_indices;

#pragma omp parallel for
      for (int k2=start+1; k2<(int)clusters.size(); ++k2)
      {
        if (k1!=k2)
        {
          Cluster c2 = clusters.at(k2);
          vnl_matrix<float> v = c2.h / c2.n;
          bool f = false;

          float d = 0;
          for (auto m : m_Metrics)
            d += m->CalculateDistance(t, v, f);
          d /= m_Metrics.size();

#pragma omp critical
          if (d<m_MergeDuplicateThreshold)
          {
            merge_indices.push_back(k2);
            flip_indices.push_back(f);
          }
        }
      }

      for (unsigned int i=0; i<merge_indices.size(); ++i)
      {
        Cluster c2 = clusters.at(merge_indices.at(i));
        for (int i=0; i<c2.n; ++i)
        {
          clusters[k1].I.push_back(c2.I.at(i));
          clusters[k1].n += 1;
        }
        if (!flip_indices.at(i))
          clusters[k1].h += c2.h;
        else
          clusters[k1].h += c2.h.fliplr();
      }

      std::sort(merge_indices.begin(), merge_indices.end());
      for (unsigned int i=0; i<merge_indices.size(); ++i)
      {
        clusters.erase (clusters.begin()+merge_indices.at(i)-i);
        found = true;
      }
      if (found)
        break;
    }
    ++start;
  }
  MITK_INFO << "\nNumber of clusters after merging duplicates: " << clusters.size();
}

std::vector<TractClusteringFilter::Cluster> TractClusteringFilter::AddToKnownClusters(std::vector< unsigned int > f_indices, std::vector<vnl_matrix<float> >& centroids)
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

#pragma omp parallel for
  for (int i=0; i<N; ++i)
  {
    vnl_matrix<float> t = T.at(f_indices.at(i));

    int min_cluster_index = -1;
    float min_cluster_distance = 99999;
    bool flip = false;

    if (CalcOverlap(t)>=m_OverlapThreshold)
    {
      int c_idx = 0;
      for (vnl_matrix<float> centroid : centroids)
      {
        bool f = false;
        float d = 0;
        for (auto m : m_Metrics)
          d += m->CalculateDistance(t, centroid, f);
        d /= m_Metrics.size();

        if (d<min_cluster_distance)
        {
          min_cluster_distance = d;
          min_cluster_index = c_idx;
          flip = f;
        }
        ++c_idx;
      }
    }

    if (min_cluster_index>=0 && min_cluster_distance<dist_thres)
    {
#pragma omp critical
      {
        C[min_cluster_index].I.push_back(f_indices.at(i));
        if (!flip)
          C[min_cluster_index].h += t;
        else
          C[min_cluster_index].h += t.fliplr();
        C[min_cluster_index].n += 1;
      }
    }
    else
    {
#pragma omp critical
      {
        no_fit.I.push_back(f_indices.at(i));
        no_fit.n++;
      }
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

  if (m_Metrics.empty())
  {
    mitkThrow() << "No metric selected!";
    return;
  }

  T = ResampleFibers(m_Tractogram);
  if (T.empty())
  {
    MITK_INFO << "No fibers in tractogram!";
    return;
  }

  std::vector< unsigned int > f_indices;
  for (unsigned int i=0; i<T.size(); ++i)
    f_indices.push_back(i);
  //  std::random_shuffle(f_indices.begin(), f_indices.end());

  Cluster no_match;
  std::vector< Cluster > clusters;
  if (m_InCentroids.IsNull())
  {
    MITK_INFO << "Clustering fibers";
    clusters = ClusterStep(f_indices, m_Distances);
    MITK_INFO << "Number of clusters: " << clusters.size();
    clusters = MergeDuplicateClusters2(clusters);
    std::sort(clusters.begin(),clusters.end());
  }
  else
  {
    std::vector<vnl_matrix<float> > centroids = ResampleFibers(m_InCentroids);
    if (centroids.empty())
    {
      MITK_INFO << "No fibers in centroid tractogram!";
      return;
    }
    MITK_INFO << "Clustering with input centroids";
    clusters = AddToKnownClusters(f_indices, centroids);
    no_match = clusters.back();
    clusters.pop_back();
    MITK_INFO << "Number of clusters: " << clusters.size();
    clusters = MergeDuplicateClusters2(clusters);
  }

  MITK_INFO << "Clustering finished";
  int max = clusters.size()-1;
  if (m_MaxClusters>0 && clusters.size()-1>m_MaxClusters)
    max = m_MaxClusters;
  int skipped = 0;
  for (int i=clusters.size()-1; i>=0; --i)
  {
    Cluster c = clusters.at(i);
    if ( c.n>=(int)m_MinClusterSize && !(m_MaxClusters>0 && clusters.size()-i>m_MaxClusters) )
    {
      m_OutClusters.push_back(c);

      vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();
      vtkSmartPointer<vtkPolyData> pTmp = m_Tractogram->GeneratePolyDataByIds(c.I, weights);
      mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(pTmp);
      if (max>0)
        fib->SetFiberWeights((float)i/max);
      m_OutTractograms.push_back(fib);
      m_OutFiberIndices.push_back(c.I);

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
    else
    {
      skipped++;
    }
  }
  MITK_INFO << "Final number of clusters: " << m_OutTractograms.size() << " (discarded " << skipped << " clusters)";

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
    m_OutFiberIndices.push_back(no_match.I);
    m_OutTractograms.push_back(fib);
  }
}

}




