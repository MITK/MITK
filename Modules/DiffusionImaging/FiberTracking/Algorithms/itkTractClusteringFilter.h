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
#ifndef itkTractClusteringFilter_h
#define itkTractClusteringFilter_h

// MITK
#include <mitkPlanarEllipse.h>
#include <mitkFiberBundle.h>
#include <mitkFiberfoxParameters.h>
#include <mitkClusteringMetric.h>

// ITK
#include <itkProcessObject.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

namespace itk{

/**
* \brief    */

class TractClusteringFilter : public ProcessObject
{
public:

  struct Cluster
  {
    Cluster() : n(0), f_id(-1) {}

    vnl_matrix<float> h;
    std::vector< unsigned int > I;
    int n;
    int f_id;

    bool operator <(Cluster const& b) const
    {
      return this->n < b.n;
    }
  };

  typedef TractClusteringFilter Self;
  typedef ProcessObject                                       Superclass;
  typedef SmartPointer< Self >                                Pointer;
  typedef SmartPointer< const Self >                          ConstPointer;
  typedef itk::Image< float, 3 >                              FloatImageType;
  typedef itk::Image< unsigned char, 3 >                      UcharImageType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( TractClusteringFilter, ProcessObject )

  itkSetMacro(NumPoints, unsigned int)  ///< Fibers are resampled to the specified number of points. If scalar maps are used, a larger number of points is recommended.
  itkGetMacro(NumPoints, unsigned int)  ///< Fibers are resampled to the specified number of points. If scalar maps are used, a larger number of points is recommended.
  itkSetMacro(MinClusterSize, unsigned int) ///< Clusters with too few fibers are discarded
  itkGetMacro(MinClusterSize, unsigned int) ///< Clusters with too few fibers are discarded
  itkSetMacro(MaxClusters, unsigned int) ///< Only the N largest clusters are kept
  itkGetMacro(MaxClusters, unsigned int) ///< Only the N largest clusters are kept
  itkSetMacro(MergeDuplicateThreshold, float) ///< Clusters with centroids very close to each other are merged. Set to 0 to avoid merging and to -1 to use the original cluster size.
  itkGetMacro(MergeDuplicateThreshold, float) ///< Clusters with centroids very close to each other are merged. Set to 0 to avoid merging and to -1 to use the original cluster size.
  itkSetMacro(DoResampling, bool) ///< Resample fibers to equal number of points. This is mandatory, but can be performed outside of the filter if desired.
  itkGetMacro(DoResampling, bool) ///< Resample fibers to equal number of points. This is mandatory, but can be performed outside of the filter if desired.
  itkSetMacro(OverlapThreshold, float)  ///< Overlap threshold used in conjunction with the filter mask when clustering around known centroids.
  itkGetMacro(OverlapThreshold, float)  ///< Overlap threshold used in conjunction with the filter mask when clustering around known centroids.

  itkSetMacro(Tractogram, mitk::FiberBundle::Pointer)   ///< The streamlines to be clustered
  itkSetMacro(InCentroids, mitk::FiberBundle::Pointer)  ///< If a tractogram containing known tract centroids is set, the input fibers are assigned to the closest centroid. If no centroid is found within the specified smallest clustering distance, the fiber is assigned to the no-fit cluster.
  itkSetMacro(FilterMask, UcharImageType::Pointer)  ///< If fibers are clustered around the nearest input centroids (see SetInCentroids), the complete input tractogram can additionally be pre-filtered with this binary mask and a given overlap threshold (see SetOverlapThreshold).

  virtual void Update() override{
    this->GenerateData();
  }

  void SetDistances(const std::vector<float> &Distances); ///< Set clustering distances that are traversed recoursively. The distances have to be sorted in an ascending manner. The actual cluster size is determined by the smallest entry in the distance-list (index 0).

  std::vector<mitk::FiberBundle::Pointer> GetOutTractograms() const;
  std::vector<mitk::FiberBundle::Pointer> GetOutCentroids() const;
  std::vector<Cluster> GetOutClusters() const;

  void SetMetrics(const std::vector<mitk::ClusteringMetric *> &Metrics);

  std::vector<std::vector<unsigned int> > GetOutFiberIndices() const;

protected:

  void GenerateData() override;
  std::vector< vnl_matrix<float> > ResampleFibers(FiberBundle::Pointer tractogram);
  float CalcOverlap(vnl_matrix<float>& t);

  std::vector< Cluster > ClusterStep(std::vector< unsigned int > f_indices, std::vector< float > distances);

  std::vector< TractClusteringFilter::Cluster > MergeDuplicateClusters2(std::vector< TractClusteringFilter::Cluster >& clusters);
  void MergeDuplicateClusters(std::vector< TractClusteringFilter::Cluster >& clusters);
  std::vector< Cluster > AddToKnownClusters(std::vector< unsigned int > f_indices, std::vector<vnl_matrix<float> > &centroids);
  void AppendCluster(std::vector< Cluster >& a, std::vector< Cluster >&b);

  TractClusteringFilter();
  virtual ~TractClusteringFilter();

  unsigned int                                m_NumPoints;
  std::vector< float >                        m_Distances;
  mitk::FiberBundle::Pointer                  m_Tractogram;
  mitk::FiberBundle::Pointer                  m_InCentroids;
  std::vector< mitk::FiberBundle::Pointer >   m_OutTractograms;
  std::vector< mitk::FiberBundle::Pointer >   m_OutCentroids;
  std::vector<vnl_matrix<float> >             T;
  unsigned int                                m_MinClusterSize;
  unsigned int                                m_MaxClusters;
  float                                       m_MergeDuplicateThreshold;
  std::vector< Cluster >                      m_OutClusters;
  bool                                        m_DoResampling;
  UcharImageType::Pointer                     m_FilterMask;
  float                                       m_OverlapThreshold;
  std::vector< mitk::ClusteringMetric* >      m_Metrics;
  std::vector< std::vector< unsigned int > >          m_OutFiberIndices;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractClusteringFilter.cpp"
#endif

#endif
