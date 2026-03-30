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
  /**
   * \brief Proxy class for global image casting functions needed by the Python wrapping process.
   *
   * Global functions cannot be wrapped by the Python binding generator, but static methods
   * can be wrapped. This class provides static wrappers around CastToItkImage and
   * CastToMitkImage for all supported pixel type and dimension combinations.
   *
   * \ingroup Adaptor
   */
  class MITKCORE_EXPORT ImageCaster
  {
  public:
    BOOST_PP_SEQ_FOR_EACH(DeclareMitkImageCasterMethods, _, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(2))
    BOOST_PP_SEQ_FOR_EACH(DeclareMitkImageCasterMethods, _, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(3))

    /** \brief Cast a BaseData pointer to a mitk::Image smart pointer via dynamic_cast. */
    static void CastBaseData(mitk::BaseData *const, itk::SmartPointer<mitk::Image> &);
  };

  /**
   * \brief Helper class providing a static cast method for BaseData to Surface.
   *
   * Used as a proxy for Python wrapping.
   */
  class MITKCORE_EXPORT Caster
  {
  public:
    /** \brief Cast a BaseData pointer to a Surface pointer. */
    static void Cast(BaseData *dat, Surface *surface);
  };

  /**
   * \brief Helper class to store and retrieve a vtkRenderer for 3D rendering access.
   *
   * Used as a proxy for Python wrapping.
   */
  class MITKCORE_EXPORT RendererAccess
  {
  public:
    /** \brief Set the 3D renderer instance. */
    static void Set3DRenderer(vtkRenderer *renderer);

    /** \brief Get the 3D renderer instance. */
    static vtkRenderer *Get3DRenderer();

  protected:
    static vtkRenderer *m_3DRenderer; ///< \brief The stored 3D renderer instance.
  };
} // namespace mitk

#endif
