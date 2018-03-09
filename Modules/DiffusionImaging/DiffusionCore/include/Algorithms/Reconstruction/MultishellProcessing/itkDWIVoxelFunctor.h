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

#ifndef _itk_DWIVoxelFunctor_h_
#define _itk_DWIVoxelFunctor_h_

#include <MitkDiffusionCoreExports.h>
#include "vnl/vnl_vector.h"
#include "vnl/vnl_matrix.h"
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace itk
{

/**
 * \brief The DWIVoxelFunctor class
 * Abstract basisclass for voxelprocessing of Diffusion Weighted Images
 */
class MITKDIFFUSIONCORE_EXPORT DWIVoxelFunctor: public Object
{
protected:
  DWIVoxelFunctor(){}
  ~DWIVoxelFunctor() override{}

public:
  typedef DWIVoxelFunctor                         Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef itk::Object                                  Superclass;

  /** Runtime information support. */
  itkTypeMacro(DWIVoxelFunctor, Object)
  /** Method for creation through the object factory. */
  //itkFactorylessNewMacro(Self)
  //itkCloneMacro(Self)

  /**
   * \brief operator ()
   * \param SignalMatrix is a NxM matrix (N = Number of gradients; M = Number of Shells)
   * \param S0 is the reference signal (b=0)
   * \return NxP Signal vector containing the new signal (e.g. [S_1 S_2 S_3 ... S_N] -> only diffusion weighted signal).
   *The first column of the matrix is reserved for the new calculated signal (other columns can hold e.g. the RMS-error)
   */
  virtual void operator()(vnl_matrix<double> & /*newSignal*/,const vnl_matrix<double> & /*SignalMatrix*/, const double & /*S0*/)=0;

};

}

#endif
