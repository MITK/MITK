/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageToContourModelFilter_h
#define mitkImageToContourModelFilter_h

#include <mitkCommon.h>
#include <mitkContourModel.h>
#include <mitkContourModelSource.h>
#include <MitkContourModelExports.h>
#include <mitkImage.h>

namespace mitk
{
  /** \brief Filter that extracts contour lines from a 2D image at a given iso-value.
   *
   * Takes a 2D mitk::Image as input and extracts contour lines at the specified
   * contour value (default: 0.5) using ITK's ContourExtractor2DImageFilter.
   * The input image is padded by one pixel on each side to handle contours
   * touching image boundaries. The extracted contours are output as closed
   * ContourModel instances in world coordinates.
   *
   * \pre The input image must be 2-dimensional.
   *
   * \sa ContourModel, ContourModelSource, Image
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ImageToContourModelFilter : public ContourModelSource
  {
  public:
    mitkClassMacro(ImageToContourModelFilter, ContourModelSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief The input image type. */
      typedef mitk::Image InputType;

    using Superclass::SetInput;

    /** \brief Set the input image from which contours are extracted.
     * \param[in] input Pointer to a 2D mitk::Image.
     */
    virtual void SetInput(const InputType *input);

    /** \brief Set the input image at the given index.
     * \param[in] idx Input port index.
     * \param[in] input Pointer to a 2D mitk::Image.
     */
    virtual void SetInput(unsigned int idx, const InputType *input);

    /** \brief Get the first input image.
     * \return Const pointer to the input Image, or nullptr if none is set.
     */
    const InputType *GetInput(void);

    /** \brief Get the input image at the given index.
     * \param[in] idx Input port index.
     * \return Const pointer to the input Image, or nullptr if none is set.
     */
    const InputType *GetInput(unsigned int idx);

    /** \brief Set the iso-value at which contours are extracted.
     * \param[in] contourValue The iso-value threshold. Default is 0.5.
     */
    void SetContourValue(float contourValue);

    /** \brief Get the current iso-value used for contour extraction.
     * \return The contour iso-value.
     */
    float GetContourValue();

  protected:
    ImageToContourModelFilter();

    ~ImageToContourModelFilter() override;

    void GenerateData() override;

    template <typename TPixel, unsigned int VImageDimension>
    void Itk2DContourExtraction(const itk::Image<TPixel, VImageDimension> *sliceImage);

  private:
    const BaseGeometry *m_SliceGeometry;
    float m_ContourValue;
  };
}

#endif
