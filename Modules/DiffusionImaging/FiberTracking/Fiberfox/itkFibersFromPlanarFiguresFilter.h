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
#ifndef itkFibersFromPlanarFiguresFilter_h
#define itkFibersFromPlanarFiguresFilter_h

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
* \brief Generates artificial fibers distributed in and interpolated between the input planar figures.   */

class FibersFromPlanarFiguresFilter : public ProcessObject
{
public:

  typedef FibersFromPlanarFiguresFilter Self;
  typedef ProcessObject                                       Superclass;
  typedef SmartPointer< Self >                                Pointer;
  typedef SmartPointer< const Self >                          ConstPointer;
  typedef mitk::FiberBundle::Pointer                          FiberType;
  typedef std::vector< mitk::FiberBundle::Pointer >           FiberContainerType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( FibersFromPlanarFiguresFilter, ProcessObject )

  void Update() override{
    this->GenerateData();
  }

  // input
  void SetParameters( FiberGenerationParameters param )  ///< Simulation parameters.
  {
    m_Parameters = param;
  }

  // output
  FiberContainerType GetFiberBundles(){ return m_FiberBundles; }

  void SetFixSeed(bool FixSeed);

protected:

  void GenerateData() override;

  FibersFromPlanarFiguresFilter();
  ~FibersFromPlanarFiguresFilter() override;
  void GeneratePoints();

  FiberContainerType              m_FiberBundles;    ///< container for the output fiber bundles
  std::vector< mitk::Vector2D >   m_2DPoints;        ///< container for the 2D fiber waypoints
  FiberGenerationParameters       m_Parameters;
  bool                            m_FixSeed;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFibersFromPlanarFiguresFilter.cpp"
#endif

#endif
