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

  // Used to cast 4d Mitk image to 3d Mitk Image
  template<typename TPixel, unsigned int VImageDimension = 4U>
  void extract3Dfrom4DByItk(itk::Image<TPixel, VImageDimension>* itkImage4d, mitk::Image* image3d, unsigned int t)
  {
    typedef itk::Image<TPixel, VImageDimension> InputImageType;
    typedef itk::Image<TPixel, VImageDimension - 1U> OutputImageType;
    typedef itk::ExtractImageFilter<InputImageType, OutputImageType> ExtractImageFilterType;

    auto region = itkImage4d->GetLargestPossibleRegion();
    auto index = region.GetIndex();
    auto size = region.GetSize();

    // We use region size AxBxCx0 for extracting 3D image
    index.Fill(0);
    index[3] = t;
    size[3] = 0;

    region.SetIndex(index);
    region.SetSize(size);

    auto extractor = ExtractImageFilterType::New();

    extractor->SetInput(itkImage4d);
    extractor->SetDirectionCollapseToSubmatrix();
    extractor->SetExtractionRegion(region);
    extractor->Update();

    image3d->InitializeByItk<OutputImageType>(extractor->GetOutput());
    image3d->SetVolume(extractor->GetOutput()->GetBufferPointer());
  }
}

#endif // MITKIMAGECASTER_H
