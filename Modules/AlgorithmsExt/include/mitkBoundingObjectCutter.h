/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingObjectCutter_h
#define mitkBoundingObjectCutter_h

#include <MitkAlgorithmsExtExports.h>
#include <itkImage.h>
#include <mitkBoundingObject.h>
#include <mitkCommon.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToImageFilter.h>

namespace mitk
{
  /**
   * \brief Cuts a BoundingObject region out of a mitk::Image.
   *
   * This filter extracts the region defined by a BoundingObject from an input image.
   * If no input image is provided, inside pixels are filled with m_InsideValue.
   * If an input image is provided, inside pixels retain the original image values
   * (unless m_UseInsideValue is set to true, in which case they are replaced with
   * m_InsideValue). Pixels outside the BoundingObject are set to m_OutsideValue
   * (or the pixel type minimum if m_AutoOutsideValue is true).
   *
   * \sa BoundingObject
   * \sa BoundingObjectCutAndCast
   * \sa BoundingObjectToSegmentationFilter
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT BoundingObjectCutter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BoundingObjectCutter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * \brief Set the bounding object that defines the cutting region.
       * \param[in] boundingObject The bounding object to use.
       */
      void SetBoundingObject(const mitk::BoundingObject *boundingObject);

    /**
     * \brief Get the bounding object that defines the cutting region.
     * \return Const pointer to the bounding object.
     */
    const mitk::BoundingObject *GetBoundingObject() const;

    /**
     * \brief Set the pixel value for inside pixels, used when m_UseInsideValue is true.
     * \param[in] _arg The inside pixel value.
     */
    itkSetMacro(InsideValue, ScalarType);

    /**
     * \brief Get the pixel value for inside pixels.
     * \return The inside pixel value.
     */
    itkGetMacro(InsideValue, ScalarType);

    /**
     * \brief Set the pixel value for outside pixels, used when m_AutoOutsideValue is false.
     * \param[in] _arg The outside pixel value (default: 0).
     */
    itkSetMacro(OutsideValue, ScalarType);

    /**
     * \brief Get the pixel value for outside pixels.
     * \return The outside pixel value.
     */
    itkGetMacro(OutsideValue, ScalarType);

    /**
     * \brief Set whether to use m_InsideValue for inside pixels (default: false).
     *
     * If true, all pixels inside the bounding object are set to m_InsideValue.
     * If false, they retain their original image values.
     *
     * \param[in] _arg True to replace inside pixels with m_InsideValue.
     */
    itkSetMacro(UseInsideValue, bool);

    /**
     * \brief Get whether m_InsideValue is used for inside pixels.
     * \return True if inside pixels are replaced with m_InsideValue.
     */
    itkGetMacro(UseInsideValue, bool);

    /** \brief Toggle UseInsideValue on/off. */
    itkBooleanMacro(UseInsideValue);

    /**
     * \brief Set whether to automatically determine the outside value.
     *
     * If true, the minimum value of the output pixel type is used as the
     * outside value (default: false).
     *
     * \param[in] _arg True to use automatic outside value.
     */
    itkSetMacro(AutoOutsideValue, bool);

    /**
     * \brief Get whether the outside value is automatically determined.
     * \return True if automatic outside value is enabled.
     */
    itkGetMacro(AutoOutsideValue, bool);

    /** \brief Toggle AutoOutsideValue on/off. */
    itkBooleanMacro(AutoOutsideValue);

    /**
     * \brief Get the number of pixels inside the bounding object after the last execution.
     * \return The inside pixel count.
     */
    itkGetMacro(InsidePixelCount, unsigned int);

    /**
     * \brief Get the number of pixels outside the bounding object after the last execution.
     * \return The outside pixel count.
     */
    itkGetMacro(OutsidePixelCount, unsigned int);

    /**
     * \brief Set whether to use the whole input region instead of a subregion.
     * \param[in] _arg True to use the whole input region.
     */
    itkSetMacro(UseWholeInputRegion, bool);

    /**
     * \brief Get whether the whole input region is used.
     * \return True if using the whole input region.
     */
    itkGetMacro(UseWholeInputRegion, bool);

  protected:
    BoundingObjectCutter();
    ~BoundingObjectCutter() override;

    virtual const PixelType GetOutputPixelType();

    void GenerateInputRequestedRegion() override;
    void GenerateOutputInformation() override;
    void GenerateData() override;

    template <typename TPixel, unsigned int VImageDimension, typename TOutputPixel>
    friend void CutImageWithOutputTypeSelect(itk::Image<TPixel, VImageDimension> *inputItkImage,
                                             mitk::BoundingObjectCutter *cutter,
                                             int boTimeStep,
                                             TOutputPixel *dummy);
    template <typename TPixel, unsigned int VImageDimension, typename TOutputPixel>
    friend void CutImageWithOutputTypeSelect(itk::VectorImage<TPixel, VImageDimension> *inputItkImage,
                                             mitk::BoundingObjectCutter *cutter,
                                             int boTimeStep,
                                             TOutputPixel *dummy);
    template <typename TPixel, unsigned int VImageDimension>
    friend void CutImage(itk::Image<TPixel, VImageDimension> *itkImage,
                         mitk::BoundingObjectCutter *cutter,
                         int boTimeStep);
    template <typename TPixel, unsigned int VImageDimension>
    friend void CutImage(itk::VectorImage<TPixel, VImageDimension> *itkImage,
                         mitk::BoundingObjectCutter *cutter,
                         int boTimeStep);
    virtual void ComputeData(mitk::Image *input3D, int boTimeStep);

    /** \brief BoundingObject that will be cut. */
    mitk::BoundingObject::Pointer m_BoundingObject;

    /** \brief Value for inside pixels, used when m_UseInsideValue is true.
     * \sa m_UseInsideValue
     */
    ScalarType m_InsideValue;

    /** \brief Value for outside pixels (default: 0).
     * Used only if m_AutoOutsideValue is false.
     */
    ScalarType m_OutsideValue;

    /** \brief If true, the minimum of the output pixel type is used as outside value (default: false). */
    bool m_AutoOutsideValue;

    /** \brief Use m_InsideValue for inside pixels (default: false).
     *
     * If true, pixels that are inside m_BoundingObject will get m_InsideValue
     * in the cutting process. If false, they keep their original value.
     * \sa m_InsideValue
     */
    bool m_UseInsideValue;

    unsigned int m_OutsidePixelCount;
    unsigned int m_InsidePixelCount;

    /** \brief Region of input needed for cutting. */
    mitk::SlicedData::RegionType m_InputRequestedRegion;

    /** \brief Time when header was last initialized. */
    itk::TimeStamp m_TimeOfHeaderInitialization;

    mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
    mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;

    bool m_UseWholeInputRegion;
  };
} // namespace mitk

#endif
