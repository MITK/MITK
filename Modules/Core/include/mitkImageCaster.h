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

#include <itkCastImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkImage.h>
#include <itkVectorIndexSelectionCastImageFilter.h>

#include <vtkRenderWindow.h>

#include <mitkImageCast.h>
#include <mitkPPSeqForEach.h>
#include <mitkSurface.h>

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
  class MITKCORE_EXPORT ImageCaster
  {
  public:

    MITK_PP_SEQ_FOR_EACH(DeclareMitkImageCasterMethods, _, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(2))
    MITK_PP_SEQ_FOR_EACH(DeclareMitkImageCasterMethods, _, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(3))

    static void CastBaseData(mitk::BaseData* const, itk::SmartPointer<mitk::Image>&);

  };

  class MITKCORE_EXPORT Caster
  {
  public:
    static void Cast(BaseData* dat, Surface* surface);
  };

  class MITKCORE_EXPORT RendererAccess
  {
  public:
    static void Set3DRenderer(vtkRenderer* renderer);
    static vtkRenderer* Get3DRenderer();
  protected:
    static vtkRenderer* m_3DRenderer;
  };

  template<typename TPixel, unsigned int VImageDimension>
  void extractComponentFromVectorByItk(itk::VectorImage<TPixel, VImageDimension>* itkVectorImage, mitk::Image* mitkImage, int component)
  {
    typedef itk::VectorImage<TPixel, VImageDimension> InputImageType;
    typedef itk::Image<TPixel, VImageDimension> OutputImageType;
    typedef itk::VectorIndexSelectionCastImageFilter<InputImageType, OutputImageType> ComponentFilterType;

    typename ComponentFilterType::Pointer extractor = ComponentFilterType::New();

    extractor->SetInput(itkVectorImage);
    extractor->SetIndex(component);
    extractor->Update();

    mitkImage->InitializeByItk<OutputImageType>(extractor->GetOutput());
    mitkImage->SetVolume(extractor->GetOutput()->GetBufferPointer());
  }

  template<typename TPixel, unsigned int VImageDimension = 3U>
  void paste3Dto4DByItk(itk::Image<TPixel, VImageDimension>* itkImage3d, mitk::Image* image4d, unsigned int t)
  {
    image4d->SetVolume(itkImage3d->GetBufferPointer(), t);
  }
}

#endif // MITKIMAGECASTER_H
