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

#include "vnl/vnl_vector.h"
#include "vnl/vnl_matrix.h"

namespace itk
{

/**
 * @brief The DWIVoxelFunctor class
 * Abstract basisclass for voxelprocessing of Diffusion Weighted Images
 */
class DWIVoxelFunctor
{
public:
  virtual ~DWIVoxelFunctor(){}
  /**
   * @brief operator ()
   * @param SignalMatrix is a NxM matrix (N = Number of gradients; M = Number of Shells)
   * @param S0 is the reference signal (b=0)
   * @return 1xP Signal vector containing the new signal (e.g. [S_1 S_2 S_3 ... S_N] -> only diffusion weighted signal)
   */
  vnl_vector<double> operator()(const vnl_matrix<double> & SignalMatrix, const double & S0) = 0;
};

}

#endif
