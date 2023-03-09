/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageCaster_h
#define mitkImageCaster_h

#include <itkCastImageFilter.h>
#include <itkImage.h>
#include <mitkImageCast.h>
#include <mitkSurface.h>
#include <vtkRenderWindow.h>

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/rem.hpp>

#define DeclareMitkImageCasterMethods(r, data, type)                                                                   \
  static void CastToItkImage(const mitk::Image *, itk::SmartPointer<itk::Image<BOOST_PP_TUPLE_REM(2) type>> &);         \
  static void CastToMitkImage(const itk::Image<BOOST_PP_TUPLE_REM(2) type> *, itk::SmartPointer<mitk::Image> &);

namespace mitk
{
  ///
  /// \brief This class is just a proxy for global functions which are needed by the
  /// python wrapping process since global functions cannot be wrapped. Static method
  /// can be wrapped though.
  ///
  class MITKCORE_EXPORT ImageCaster
  {
  public:
    BOOST_PP_SEQ_FOR_EACH(DeclareMitkImageCasterMethods, _, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(2))
    BOOST_PP_SEQ_FOR_EACH(DeclareMitkImageCasterMethods, _, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(3))

    static void CastBaseData(mitk::BaseData *const, itk::SmartPointer<mitk::Image> &);
  };

  class MITKCORE_EXPORT Caster
  {
  public:
    static void Cast(BaseData *dat, Surface *surface);
  };

  class MITKCORE_EXPORT RendererAccess
  {
  public:
    static void Set3DRenderer(vtkRenderer *renderer);
    static vtkRenderer *Get3DRenderer();

  protected:
    static vtkRenderer *m_3DRenderer;
  };
} // namespace mitk

#endif
