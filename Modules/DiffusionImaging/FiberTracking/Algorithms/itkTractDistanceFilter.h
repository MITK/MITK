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
#ifndef itkTractDistanceFilter_h
#define itkTractDistanceFilter_h

#include <mitkPlanarEllipse.h>
#include <mitkFiberBundle.h>
#include <mitkFiberfoxParameters.h>
#include <mitkClusteringMetric.h>
#include <itkProcessObject.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <boost/progress.hpp>

namespace itk{

/**
* \brief    */

class TractDistanceFilter : public ProcessObject
{
public:

  typedef TractDistanceFilter Self;
  typedef ProcessObject                                       Superclass;
  typedef SmartPointer< Self >                                Pointer;
  typedef SmartPointer< const Self >                          ConstPointer;
  typedef itk::Image< float, 3 >                              FloatImageType;
  typedef itk::Image< unsigned char, 3 >                      UcharImageType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( TractDistanceFilter, ProcessObject )

  itkSetMacro(NumPoints, unsigned int)  ///< Fibers are resampled to the specified number of points. If scalar maps are used, a larger number of points is recommended.

  virtual void Update() override{
    this->GenerateData();
  }

  void SetMetrics(const std::vector<mitk::ClusteringMetric *> &Metrics);

  void SetTracts1(const std::vector<FiberBundle::Pointer> &Tracts1);

  void SetTracts2(const std::vector<FiberBundle::Pointer> &Tracts2);

  vnl_vector<float> GetMinDistances() const;

  vnl_vector<int> GetMinIndices() const;

  vnl_matrix<float> GetAllDistances() const;

protected:

  void GenerateData() override;
  std::vector< vnl_matrix<float> > ResampleFibers(FiberBundle::Pointer tractogram);
  float calc_distance(const std::vector<vnl_matrix<float> > &T1, const std::vector<vnl_matrix<float> > &T2);

  TractDistanceFilter();
  virtual ~TractDistanceFilter();

  std::vector<FiberBundle::Pointer>         m_Tracts1;
  std::vector<FiberBundle::Pointer>         m_Tracts2;
  unsigned int                              m_NumPoints;
  std::vector<mitk::ClusteringMetric*>      m_Metrics;
  vnl_vector<float>                         m_MinDistances;
  vnl_vector<int>                           m_MinIndices;
  vnl_matrix<float>                         m_AllDistances;
  boost::progress_display                   disp;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractDistanceFilter.cpp"
#endif

#endif
