#ifndef __itkPointShell_h__
#define __itkPointShell_h__

#include "mitkQBallImage.h"
#include "MitkDiffusionImagingExports.h"

namespace itk
{

#define DIST_POINTSHELL_PI 3.14159265358979323846

  // generate by n-fold subdivisions of an icosahedron
  template<int NPoints, class TMatrixType >
  class PointShell
  {
  public:
    static TMatrixType *DistributePointShell();
  };

}

#include "itkPointShell.txx"

#endif //__itkPointShell_h__
