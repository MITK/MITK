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

#ifndef MITKIMAGECASTER_H
#define MITKIMAGECASTER_H

#include <mitkImageCast.h>
#include <itkImage.h>
#include <itkCastImageFilter.h>
#include <vtkRenderWindow.h>
#include <mitkSurface.h>

#include <mitkPPSeqForEach.h>

#define DeclareMitkImageCasterMethods(r, data, type) \
  static void CastToItkImage(const mitk::Image*, itk::SmartPointer<itk::Image<MITK_PP_TUPLE_REM(2)type> >&); \
  static void CastToMitkImage(const itk::Image<MITK_PP_TUPLE_REM(2)type>*, itk::SmartPointer<mitk::Image>&);

namespace mitk
{
  ///
  /// \brief This class is just a proxy for global functions which are needed by the
  /// python wrapping process since global functions cannnot be wrapped. Static method
  /// can be wrapped though.
  ///
  class MITK_CORE_EXPORT ImageCaster
  {
  public:

    MITK_PP_SEQ_FOR_EACH(DeclareMitkImageCasterMethods, _, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(2))
    MITK_PP_SEQ_FOR_EACH(DeclareMitkImageCasterMethods, _, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(3))

    static void CastBaseData(mitk::BaseData* const, itk::SmartPointer<mitk::Image>&);

  };

  class MITK_CORE_EXPORT Caster
  {
  public:
    static void Cast(BaseData* dat, Surface* surface);
  };

  class MITK_CORE_EXPORT RendererAccess
  {
  public:
    static void Set3DRenderer(vtkRenderer* renderer);
    static vtkRenderer* Get3DRenderer();
  protected:
    static vtkRenderer* m_3DRenderer;
  };
}

#endif // MITKIMAGECASTER_H
