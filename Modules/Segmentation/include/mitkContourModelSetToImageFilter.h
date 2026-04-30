/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSetToImageFilter_h
#define mitkContourModelSetToImageFilter_h

#include <MitkSegmentationExports.h>
#include <mitkImageSource.h>

namespace mitk
{
  class ContourModelSet;
  class ContourModel;

  /**
   * \brief Fills a given mitk::ContourModelSet into a given mitk::Image.
   *
   * Rasterizes contour model sets into image volumes by filling each contour
   * on the appropriate slice of the output image.
   *
   * \ingroup Process
   * \sa ContourModelSet, ContourModel
   */
  class MITKSEGMENTATION_EXPORT ContourModelSetToImageFilter : public ImageSource
  {
  public:
    mitkClassMacro(ContourModelSetToImageFilter, ImageSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    virtual void SetMakeOutputBinary(bool makeOutputBinary);
    virtual void SetMakeOutputLabelPixelType(bool makeOutputLabelPixelType);
    itkSetMacro(PaintingPixelValue, int);
    itkSetMacro(TimeStep, unsigned int);

    itkGetMacro(MakeOutputBinary, bool);
    itkGetMacro(MakeOutputLabelPixelType, bool);
    itkGetMacro(PaintingPixelValue, int);

    itkBooleanMacro(MakeOutputBinary);
    itkBooleanMacro(MakeOutputLabelPixelType);

    /**
     * \brief Allocates a new output object and returns it.
     *
     * Currently the index is not evaluated.
     *
     * \param[in] idx The index of the output for which an object should be created.
     * \return The new output data object.
     */
    itk::DataObject::Pointer MakeOutput(DataObjectPointerArraySizeType idx) override;

    /**
       * This is a default implementation to make sure we have something.
       * Once all the subclasses of ProcessObject provide an appropriate
       * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
       * virtual.
       */
    itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    const mitk::ContourModelSet *GetInput(void);

    using itk::ProcessObject::SetInput;
    virtual void SetInput(const mitk::ContourModelSet *input);

    /**
     * \brief Sets the image which will be used to initialize the output of this filter.
     * \param[in] refImage The image used to initialize the output image geometry and properties.
     */
    void SetImage(const mitk::Image *refImage);

    const mitk::Image *GetImage(void);

  protected:
    ContourModelSetToImageFilter();

    ~ContourModelSetToImageFilter() override;

    /**
     * \brief Initializes the volume of the output image with zeros.
     */
    void InitializeOutputEmpty();

    bool m_MakeOutputBinary;
    bool m_MakeOutputLabelPixelType;
    int m_PaintingPixelValue;

    unsigned int m_TimeStep;

    const mitk::Image *m_ReferenceImage;
  };

  /**
   * \brief Converts a contour model set into a binary label mask image.
   *
   * \param[in] refImage Image that serves as geometry template for the output.
   * \param[in] contourSet The contour set to convert.
   * \return A binary image suitable for use as label content in a segmentation.
   */
  MITKSEGMENTATION_EXPORT Image::Pointer ConvertContourModelSetToLabelMask(const mitk::Image* refImage, mitk::ContourModelSet* contourSet);

  /**
   * \brief Converts a contour model into a binary label mask image.
   *
   * \param[in] refImage Image that serves as geometry template for the output.
   * \param[in] contourModel The contour model to convert.
   * \return A binary image suitable for use as label content in a segmentation.
   */
  MITKSEGMENTATION_EXPORT Image::Pointer ConvertContourModelToLabelMask(const mitk::Image* refImage, mitk::ContourModel* contourModel);
}
#endif
