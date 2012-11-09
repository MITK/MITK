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

#ifndef __itkPointShell_h__
#define __itkPointShell_h__

#include "mitkQBallImage.h"
#include "MitkDiffusionImagingExports.h"


namespace itk
{
  // generate by n-fold subdivisions of an icosahedron
  template<int NPoints, class TMatrixType >
  class PointShell
  {
  public:
    static TMatrixType *DistributePointShell();
  };

  template<int NpointsShell1, int NpointsShell2, int NpointsShell3, class TMatrixTypeshell1, class TMatrixTypeshell2, class TMatrixTypeshell3>
  class ThreeLayerPointShell
  {
  public:
    static TMatrixTypeshell1 *DistributePointShell1();
    static TMatrixTypeshell2 *DistributePointShell2();
    static TMatrixTypeshell3 *DistributePointShell3();
  };

}

#include "itkPointShell.txx"

#endif //__itkPointShell_h__
