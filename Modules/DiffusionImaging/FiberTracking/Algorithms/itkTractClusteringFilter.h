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
    vnl_matrix<float> h;
    std::vector< long > I;
    int n;

    bool operator <(Cluster const& b) const
    {
      return this->n < b.n;
    }
  };

  enum Metric
  {
    MDF,
    MDF_VAR,
    MAX_MDF
  };

  typedef TractClusteringFilter Self;
  typedef ProcessObject                                       Superclass;
  typedef SmartPointer< Self >                                Pointer;
  typedef SmartPointer< const Self >                          ConstPointer;
  typedef itk::Image< float, 3 >                              FloatImageType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( TractClusteringFilter, ProcessObject )

  itkSetMacro(NumPoints, unsigned int)
  itkGetMacro(NumPoints, unsigned int)
  itkSetMacro(MinClusterSize, unsigned int)
  itkGetMacro(MinClusterSize, unsigned int)
  itkSetMacro(MaxClusters, unsigned int)
  itkGetMacro(MaxClusters, unsigned int)
  itkSetMacro(Scale, float)
  itkGetMacro(Scale, float)
  itkSetMacro(MergeDuplicateThreshold, float)
  itkGetMacro(MergeDuplicateThreshold, float)

  itkSetMacro(Tractogram, mitk::FiberBundle::Pointer)
  itkSetMacro(InCentroids, mitk::FiberBundle::Pointer)
  itkSetMacro(ScalarMap, FloatImageType::Pointer)

  virtual void Update() override{
    this->GenerateData();
  }

  void SetDistances(const std::vector<float> &Distances);

  std::vector<mitk::FiberBundle::Pointer> GetOutTractograms() const;

  void SetMetric(const Metric &Metric);

  std::vector<mitk::FiberBundle::Pointer> GetOutCentroids() const;

  std::vector<Cluster> GetOutClusters() const;

protected:

  void GenerateData() override;
  std::vector< vnl_matrix<float> > ResampleFibers(FiberBundle::Pointer tractogram);
  float CalcMDF(vnl_matrix<float>& s, vnl_matrix<float>& t, bool &flipped);
  float CalcMDF_VAR(vnl_matrix<float>& s, vnl_matrix<float>& t, bool &flipped);
  float CalcMAX_MDF(vnl_matrix<float>& s, vnl_matrix<float>& t, bool &flipped);

  std::vector< Cluster > ClusterStep(std::vector< long > f_indices, std::vector< float > distances);
  void MergeDuplicateClusters(std::vector< TractClusteringFilter::Cluster >& clusters);
  std::vector< Cluster > AddToKnownClusters(std::vector< long > f_indices, std::vector<vnl_matrix<float> > &centroids);
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
  Metric                                      m_Metric;
  FloatImageType::Pointer                     m_ScalarMap;
  float                                       m_Scale;
  float                                       m_MergeDuplicateThreshold;
  std::vector< Cluster >                      m_OutClusters;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractClusteringFilter.cpp"
#endif

#endif
