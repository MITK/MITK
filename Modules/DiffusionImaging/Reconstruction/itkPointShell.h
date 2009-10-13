#ifndef __itkPointShell_h__
#define __itkPointShell_h__

#include "mitkQBallImage.h"
#include "MitkDiffusionImagingExports.h"

//#if QBALL_ODFSIZE != 12 && QBALL_ODFSIZE != 42 && QBALL_ODFSIZE != 92 \
//  && QBALL_ODFSIZE != 162 && QBALL_ODFSIZE != 252 && QBALL_ODFSIZE != 362 \
//   && QBALL_ODFSIZE != 492 && QBALL_ODFSIZE != 642 && QBALL_ODFSIZE != 812 \
//    && QBALL_ODFSIZE != 1002
//  #define POINT_SHELL_USE_GENERIC_TEMPLATE
//#endif
//
//#ifdef POINT_SHELL_USE_GENERIC_TEMPLATE
//#define MY_POINT_SHELL_EXPORT
//#else
//#define MY_POINT_SHELL_EXPORT MitkDiffusionImaging_EXPORT
//#endif

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

//#ifdef POINT_SHELL_USE_GENERIC_TEMPLATE
#include "itkPointShell.txx"
//#endif

#endif //__itkPointShell_h__
