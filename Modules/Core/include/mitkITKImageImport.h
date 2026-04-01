/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkITKImageImport_h
#define mitkITKImageImport_h

#include <itkImageToImageFilterDetail.h>
#include <mitkImageSource.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
  * \brief Pipelined import of itk::Image.
  *
  * The image data contained in the itk::Image is referenced,
  * not copied.
  * The easiest way of use is by the function
  * mitk::ImportItkImage
  * \code
  * mitkImage = mitk::ImportItkImage(itkImage);
  * \endcode
  *
  * \tparam TInputImage The ITK image type to import.
  *
  * \sa ImportItkImage
  * \sa GrabItkImageMemory
  * \ingroup Adaptor
  */
  template <class TInputImage>
  class MITK_EXPORT ITKImageImport : public ImageSource
  {
  public:
    mitkClassMacro(ITKImageImport, ImageSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief The type of the input image. */
    typedef TInputImage InputImageType;
    typedef typename InputImageType::Pointer InputImagePointer;
    typedef typename InputImageType::ConstPointer InputImageConstPointer;
    typedef typename InputImageType::RegionType InputImageRegionType;
    typedef typename InputImageType::PixelType InputImagePixelType;

    /** ImageDimension constants */
    itkStaticConstMacro(InputImageDimension, unsigned int, TInputImage::ImageDimension);
    itkStaticConstMacro(RegionDimension, unsigned int, mitk::SlicedData::RegionDimension);

    /**
     * \brief Get the input itk::Image of this image importer.
     *
     * \return Pointer to the input ITK image.
     */
    InputImageType *GetInput(void);

    /**
     * \brief Set the input itk::Image of this image importer.
     *
     * \param input The ITK image to import.
     */
    void SetInput(const InputImageType *input);
    using itk::ProcessObject::SetInput;

    /**
     * \brief Set the Geometry of the result image (optional).
     *
     * The Geometry has to fit the dimension and size of
     * the input image. The Geometry will be cloned, not
     * referenced!
     *
     * Providing the Geometry is optional.
     * The default behavior is to set the geometry by
     * the itk::Image::GetDirection() information.
     *
     * \param geometry The BaseGeometry to apply to the output image.
     */
    void SetGeometry(const BaseGeometry *geometry);

  protected:
    /** \brief Constructor. */
    ITKImageImport();

    /** \brief Destructor. */
    ~ITKImageImport() override;

    /** \brief Compute output information from the input ITK image. */
    void GenerateOutputInformation() override;

    /** \brief Set the requested region on the input image. */
    void GenerateInputRequestedRegion() override;

    /** \brief Perform the actual data import from the ITK image. */
    void GenerateData() override;

    /** \brief Override to set the nth output. */
    void SetNthOutput(DataObjectPointerArraySizeType num, itk::DataObject *output) override;

    /** Typedef for the region copier function object that converts an
     * output region to an input region. */
    typedef itk::ImageToImageFilterDetail::ImageRegionCopier<itkGetStaticConstMacro(InputImageDimension),
                                                             itkGetStaticConstMacro(RegionDimension)>
      OutputToInputRegionCopierType;

    BaseGeometry::Pointer m_Geometry;
  };

  /**
  * \brief Import an itk::Image (with a specific type) as an mitk::Image.
  * \ingroup Adaptor
  *
  * Instantiates an instance of ITKImageImport.
  * mitk::ITKImageImport does not cast pixel types etc., it just imports
  * image data. If you get a compile error, try image.GetPointer().
  *
  * \tparam ItkOutputImageType The ITK image type to import.
  * \param itkimage The ITK image to import.
  * \param geometry Optional geometry to apply to the output image.
  * \param update If \c true, fill the mitk::Image, which will execute the
  * up-stream pipeline connected to the input itk::Image. Otherwise you
  * need to make sure that Update() is called on the mitk::Image before
  * its data is being used, e.g., by connecting it to an mitk-pipeline
  * and calling Update of a downstream filter at some time.
  * \return The imported mitk::Image.
  *
  * \sa ITKImageImport
  * \sa GrabItkImageMemory
  */
  template <typename ItkOutputImageType>
  Image::Pointer ImportItkImage(const itk::SmartPointer<ItkOutputImageType> &itkimage,
                                const BaseGeometry *geometry = nullptr,
                                bool update = true);

  /**
  * \brief Import an itk::Image (with a specific type) as an mitk::Image.
  * \ingroup Adaptor
  *
  * Instantiates an instance of ITKImageImport.
  * mitk::ITKImageImport does not cast pixel types etc., it just imports
  * image data. If you get a compile error, try image.GetPointer().
  *
  * \tparam ItkOutputImageType The ITK image type to import.
  * \param itkimage The ITK image to import.
  * \param geometry Optional geometry to apply to the output image.
  * \param update If \c true, fill the mitk::Image, which will execute the
  * up-stream pipeline connected to the input itk::Image. Otherwise you
  * need to make sure that Update() is called on the mitk::Image before
  * its data is being used, e.g., by connecting it to an mitk-pipeline
  * and calling Update of a downstream filter at some time.
  * \return The imported mitk::Image.
  *
  * \note If the source (itk image) and the target (mitk image) do not share the same scope, the
  * mitk::GrabItkImageMemory function has to be used instead. Otherwise the image memory managed
  * by the itk image is lost at a scope level change. This affects especially the usage in
  * combination with AccessByItk macros as in the following example code
  *
  * \snippet test/mitkGrabItkImageMemoryTest.cpp OutOfScopeCall
  *
  * which calls an ITK-like filter
  *
  * \snippet test/mitkGrabItkImageMemoryTest.cpp ItkThresholdFilter
  *
  * \sa ITKImageImport
  * \sa GrabItkImageMemory
  */
  template <typename ItkOutputImageType>
  Image::Pointer ImportItkImage(const ItkOutputImageType *itkimage,
                                const BaseGeometry *geometry = nullptr,
                                bool update = true);

  /**
  * \brief Grab the memory of an itk::Image (with a specific type)
  * and put it into an mitk::Image.
  * \ingroup Adaptor
  *
  * The memory is managed by the mitk::Image after calling this
  * function. The itk::Image remains valid until the mitk::Image
  * decides to free the memory.
  *
  * \tparam ItkOutputImageType The ITK image type to grab from.
  * \param itkimage The ITK image whose memory is taken over.
  * \param mitkImage Optional existing mitk::Image to initialize with the grabbed data.
  * \param geometry Optional geometry to apply to the output image.
  * \param update If \c true, fill the mitk::Image, which will execute the
  * up-stream pipeline connected to the input itk::Image. Otherwise you
  * need to make sure that Update() is called on the mitk::Image before
  * its data is being used.
  * \return The mitk::Image containing the grabbed memory.
  *
  * \sa ImportItkImage
  */
  template <typename ItkOutputImageType>
  Image::Pointer GrabItkImageMemory(itk::SmartPointer<ItkOutputImageType> &itkimage,
                                    mitk::Image *mitkImage = nullptr,
                                    const BaseGeometry *geometry = nullptr,
                                    bool update = true);

  /**
  * \brief Grab the memory of an itk::Image (with a specific type)
  * and put it into an mitk::Image.
  * \ingroup Adaptor
  *
  * The memory is managed by the mitk::Image after calling this
  * function. The itk::Image remains valid until the mitk::Image
  * decides to free the memory.
  *
  * \tparam ItkOutputImageType The ITK image type to grab from.
  * \param itkimage The ITK image whose memory is taken over.
  * \param mitkImage Optional existing mitk::Image to initialize with the grabbed data.
  * \param geometry Optional geometry to apply to the output image.
  * \param update If \c true, fill the mitk::Image, which will execute the
  * up-stream pipeline connected to the input itk::Image. Otherwise you
  * need to make sure that Update() is called on the mitk::Image before
  * its data is being used.
  * \return The mitk::Image containing the grabbed memory.
  *
  * \sa ImportItkImage
  */
  template <typename ItkOutputImageType>
  Image::Pointer GrabItkImageMemory(ItkOutputImageType *itkimage,
                                    mitk::Image *mitkImage = nullptr,
                                    const BaseGeometry *geometry = nullptr,
                                    bool update = true);

  /**
  * \brief Grab the memory of an itk::Image and put it into an mitk::Image channel.
  * \ingroup Adaptor
  *
  * The memory is managed by the mitk::Image after calling this
  * function. The itk::Image remains valid until the mitk::Image
  * decides to free the memory.
  *
  * \tparam ItkOutputImageType The ITK image type to grab from.
  * \param itkimage The ITK image whose memory is taken over.
  * \param geometry Optional TimeGeometry to apply to the output image.
  * \param mitkImage Optional existing mitk::Image to initialize with the grabbed data.
  * \param update If \c true, fill the mitk::Image, which will execute the
  * up-stream pipeline connected to the input itk::Image. Otherwise you
  * need to make sure that Update() is called on the mitk::Image before
  * its data is being used.
  * \return The mitk::Image containing the grabbed memory.
  *
  * \sa ImportItkImage
  */
  template <typename ItkOutputImageType>
  Image::Pointer GrabItkImageMemoryChannel(ItkOutputImageType* itkimage,
    const TimeGeometry* geometry = nullptr,
    mitk::Image* mitkImage = nullptr,
    bool update = true);

} // namespace mitk

#ifndef MITK_MANUAL_INSTANTIATION
#include <mitkITKImageImport.tpp>
#endif

#endif
