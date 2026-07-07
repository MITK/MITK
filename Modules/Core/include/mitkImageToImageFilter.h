/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageToImageFilter_h
#define mitkImageToImageFilter_h

#include <mitkImageSource.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Superclass of all classes having one or more Images as input and
   * generating Images as output.
   *
   * \ingroup Process
   * \sa ImageSource
   */
  class MITKCORE_EXPORT ImageToImageFilter : public ImageSource
  {
  public:
    mitkClassMacro(ImageToImageFilter, ImageSource);

    /** \brief Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief Superclass typedefs. */
      typedef Superclass::OutputImageRegionType OutputImageRegionType;

    /** \brief Some convenient typedefs. */
    typedef mitk::Image InputImageType;
    typedef InputImageType::Pointer InputImagePointer;
    typedef InputImageType::ConstPointer InputImageConstPointer;
    typedef SlicedData::RegionType InputImageRegionType;

    using itk::ProcessObject::SetInput;

    /**
     * \brief Set the image input of this process object.
     *
     * \param image The input image.
     */
    virtual void SetInput(const InputImageType *image);

    /**
     * \brief Set the image input at the specified index.
     *
     * \param idx The input index.
     * \param image The input image.
     */
    virtual void SetInput(unsigned int idx, const InputImageType *image);

    /** \brief Get the input image (non-const). */
    InputImageType *GetInput(void);

    /**
     * \brief Get the input image at the specified index (non-const).
     *
     * \param idx The input index.
     * \return The input image at the given index.
     */
    InputImageType *GetInput(unsigned int idx);

    /** \brief Get the input image (const). */
    const InputImageType *GetInput(void) const;

    /**
     * \brief Get the input image at the specified index (const).
     *
     * \param idx The input index.
     * \return The input image at the given index.
     */
    const InputImageType *GetInput(unsigned int idx) const;

  protected:
    ImageToImageFilter();
    ~ImageToImageFilter() override;

    /** \brief Print self information to the given stream. */
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    /** \brief Determine the input requested region to produce the output requested region.
     *
     * The base assumption for image processing filters is that the input
     * requested region can be set to match the output requested region.
     * If a filter requires more input (for instance a filter that uses
     * neighborhoods needs more input than output to avoid introducing
     * artificial boundary conditions) or less input (for instance a magnify
     * filter) will have to override this method. In doing so, it should call
     * its superclass' implementation as its first step.
     *
     * \sa ProcessObject::GenerateInputRequestedRegion(),
     *     ImageSource::GenerateInputRequestedRegion() */
    void GenerateInputRequestedRegion() override;

  private:
    void operator=(const Self &); // purposely not implemented
  };

} // namespace mitk

#endif
