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

#ifndef _itk_ADCAverageFunctor_h_
#define _itk_ADCAverageFunctor_h_

#include "itkDWIVoxelFunctor.h"
#include <cmath>


namespace itk
{

class MITKDIFFUSIONCORE_EXPORT ADCAverageFunctor : public DWIVoxelFunctor
{
public:
  ADCAverageFunctor(){}
  ~ADCAverageFunctor() override{}

  typedef ADCAverageFunctor                       Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef DWIVoxelFunctor                         Superclass;
  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  /** Runtime information support. */
  itkTypeMacro(ADCAverageFunctor, DWIVoxelFunctor)

  void operator()(vnl_matrix<double> & newSignal, const vnl_matrix<double> & SignalMatrix, const double & S0) override;

  void setTargetBValue(const double & targetBValue){m_TargetBvalue = targetBValue;}
  void setListOfBValues(const vnl_vector<double> & BValueList){m_BValueList = BValueList;}

protected:
  double m_TargetBvalue;
  vnl_vector<double> m_BValueList;

};

}

#endif
