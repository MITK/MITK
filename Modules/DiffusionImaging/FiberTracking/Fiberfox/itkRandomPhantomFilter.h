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
#ifndef itkRandomPhantomFilter_h
#define itkRandomPhantomFilter_h

// MITK
#include <mitkPlanarEllipse.h>
#include <mitkFiberBundle.h>
#include <mitkFiberfoxParameters.h>

// ITK
#include <itkProcessObject.h>

namespace itk{

/**
* \brief  */

class RandomPhantomFilter : public ProcessObject
{
public:

  typedef RandomPhantomFilter Self;
  typedef ProcessObject                                       Superclass;
  typedef SmartPointer< Self >                                Pointer;
  typedef SmartPointer< const Self >                          ConstPointer;
  typedef mitk::FiberBundle::Pointer                          FiberType;
  typedef std::vector< mitk::FiberBundle::Pointer >           FiberContainerType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( RandomPhantomFilter, ProcessObject )

  void Update() override{
    this->GenerateData();
  }

  // output
  FiberContainerType GetFiberBundles(){ return m_FiberBundles; }

  void SetNumTracts(unsigned int NumTracts);

  void setStartRadiusMin(unsigned int StartRadiusMin);

  void setStartRadiusMax(unsigned int StartRadiusMax);

  void setCurvynessMin(unsigned int CurvynessMin);

  void setCurvynessMax(unsigned int CurvynessMax);

  void setStepSizeMin(unsigned int StepSizeMin);

  void setStepSizeMax(unsigned int StepSizeMax);

  void setVolumeSize(const mitk::Vector3D &VolumeSize);

  void setMaxTwist(unsigned int MaxTwist);

  void setMinStreamlineDensity(unsigned int MinStreamlinDensity);

  void setMaxStreamlineDensity(unsigned int MaxStreamlineDensity);

  void setMinTwist(unsigned int MinTwist);

protected:

  void GenerateData() override;

  RandomPhantomFilter();
  ~RandomPhantomFilter() override;
  void TransformPlanarFigure(mitk::PlanarEllipse* pe, mitk::Vector3D translation, mitk::Vector3D rotation, double twistangle, double radius1, double radius2);
  mitk::PlanarEllipse::Pointer CreatePlanarFigure();
  void GetPfOnBoundingPlane(mitk::Vector3D& pos, mitk::Vector3D& rot);
  bool IsInVolume(mitk::Vector3D pos);

  FiberContainerType  m_FiberBundles;    ///< container for the output fiber bundles
  unsigned int        m_NumTracts;
  unsigned int        m_MinStreamlineDensity;
  unsigned int        m_MaxStreamlineDensity;
  unsigned int        m_StartRadiusMin;
  unsigned int        m_StartRadiusMax;
  unsigned int        m_CurvynessMin;
  unsigned int        m_CurvynessMax;
  unsigned int        m_StepSizeMin;
  unsigned int        m_StepSizeMax;
  unsigned int        m_MinTwist;
  unsigned int        m_MaxTwist;
  mitk::Vector3D      m_VolumeSize;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRandomPhantomFilter.cpp"
#endif

#endif
