/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExtractImageFilter_h
#define mitkExtractImageFilter_h

#include <mitkCommon.h>
#include <mitkImageToImageFilter.h>
#include <MitkImageExtractionExports.h>

#include <itkImage.h>

namespace mitk
{
  /**
    \brief Extracts a 2D slice from a 3D image.

    \sa ExtractSliceFilter

    \sa SegTool2D

    \ingroup Process
    \ingroup ToolManagerEtAl

    This class takes a 3D mitk::Image as input and tries to extract one slice from it.

    Two parameters determine which slice is extracted: the "slice dimension" is that one, which is constant for all
    points in the plane, e.g. axial would mean 2.
    The "slice index" is the slice index in the image direction you specified with "affected dimension". Indices count
    from zero.

    Output will not be set if there was a problem extracting the desired slice.

    Last contributor: $Author$
  */
  class MITKIMAGEEXTRACTION_EXPORT ExtractImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(ExtractImageFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
        \brief Set/Get the index of the slice to extract (zero-based).
        \param[in] _arg The zero-based index of the slice along the slice dimension.
      */
      itkSetMacro(SliceIndex, unsigned int);
    itkGetConstMacro(SliceIndex, unsigned int);

    /**
      \brief Set/Get the dimension along which to extract the slice.

      This is the dimension that remains constant for all pixels in the
      extracted 2D slice: 0 = Sagittal, 1 = Coronal, 2 = Axial.

      \param[in] _arg The dimension index (0, 1, or 2).
    */
    itkSetMacro(SliceDimension, unsigned int);
    itkGetConstMacro(SliceDimension, unsigned int);

    /**
      \brief Set/Get the time step to extract from a 4D image.

      For 3D images this value is ignored. Default is 0.
      \param[in] _arg The zero-based time step index.
     */
    itkSetMacro(TimeStep, unsigned int);
    itkGetConstMacro(TimeStep, unsigned int);

    /**
     * \brief Enumeration of direction collapse strategies for the ITK
     *        ExtractImageFilter used internally.
     *
     * Controls how the direction cosine matrix is handled when reducing
     * dimensionality from 3D to 2D.
     */
    typedef enum DirectionCollapseStrategyEnum {
      DIRECTIONCOLLAPSETOUNKOWN = 0,     ///< Unknown strategy (may cause errors).
      DIRECTIONCOLLAPSETOIDENTITY = 1,   ///< Set direction to identity matrix.
      DIRECTIONCOLLAPSETOSUBMATRIX = 2,  ///< Extract the sub-matrix from the direction.
      DIRECTIONCOLLAPSETOGUESS = 3       ///< Let ITK guess the best strategy (default).
    } DIRECTIONCOLLAPSESTRATEGY;

    /**
      \brief Set/Get the direction collapse strategy.

      Controls how the 3D direction matrix is collapsed to 2D when extracting
      a slice. Default is DIRECTIONCOLLAPSETOGUESS.
      \param[in] _arg One of the DIRECTIONCOLLAPSESTRATEGY enum values.
     */
    itkSetMacro(DirectionCollapseToStrategy, DIRECTIONCOLLAPSESTRATEGY);
    itkGetConstMacro(DirectionCollapseToStrategy, DIRECTIONCOLLAPSESTRATEGY);

  protected:
    ExtractImageFilter(); // purposely hidden
    ~ExtractImageFilter() override;

    void GenerateOutputInformation() override;
    void GenerateInputRequestedRegion() override;

    void GenerateData() override;

    template <typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing(const itk::Image<TPixel, VImageDimension> *image);

    unsigned int m_SliceIndex;
    unsigned int m_SliceDimension;
    unsigned int m_TimeStep;
    DIRECTIONCOLLAPSESTRATEGY m_DirectionCollapseToStrategy;
  };

} // namespace

#endif
