/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPixelType_h
#define mitkPixelType_h

#include <mitkCommon.h>
#include <mitkPixelTypeTraits.h>
#include <MitkCoreExports.h>

#include <string>
#include <typeinfo>

#include <itkImage.h>
#include <itkImageIOBase.h>
#include <vtkImageData.h>

namespace mitk
{
  /**
   * \brief Return the ITK component type name for the given C++ scalar type.
   * \tparam T The C++ scalar type (e.g. float, short).
   * \return A string such as "float", "short", etc.
   */
  template <typename T>
  std::string PixelComponentTypeToString()
  {
    return itk::ImageIOBase::GetComponentTypeAsString(itk::ImageIOBase::MapPixelType<T>::CType);
  }

  /**
   * \brief Return the ITK pixel type name for the given pixel type.
   *
   * The default implementation returns an empty string. Specializations
   * may provide meaningful names for compound pixel types.
   *
   * \tparam PixelT The pixel type.
   * \return A string describing the pixel type, or empty for primitive types.
   */
  template <typename PixelT>
  std::string PixelTypeToString()
  {
    return std::string();
  }

  /**
   * \brief Describes the data type of image pixels.
   *
   * Encapsulates all information about an image's pixel type, including the
   * scalar component type, the composite pixel type (e.g. scalar, RGB, vector),
   * the number of components per pixel, and the byte sizes.
   *
   * To obtain additional type information not provided by this class,
   * itk::ImageIOBase can be used by passing the return value of
   * PixelType::GetComponentType() to itk::ImageIOBase::SetPixelTypeInfo
   * and using the itk::ImageIOBase methods GetComponentType,
   * GetComponentTypeAsString, GetPixelType, GetPixelTypeAsString.
   *
   * Instances are created via the MakePixelType() family of factory functions
   * rather than constructing directly.
   *
   * \sa MakePixelType, MakeScalarPixelType, ChannelDescriptor, ImageDescriptor
   * \ingroup Data
   */
  class MITKCORE_EXPORT PixelType
  {
  public:
    typedef itk::IOPixelEnum ItkIOPixelType;      ///< ITK I/O pixel type enumeration.
    typedef itk::IOComponentEnum ItkIOComponentType; ///< ITK I/O component type enumeration.

    /**
     * \brief Copy constructor.
     * \param[in] aPixelType The PixelType to copy from.
     */
    PixelType(const mitk::PixelType &aPixelType);

    /**
     * \brief Copy assignment operator.
     * \param[in] other The PixelType to assign from.
     * \return Reference to this object.
     */
    PixelType &operator=(const PixelType &other);

    /**
     * \brief Get the ITK I/O pixel type enumeration value.
     *
     * Identifies the composite pixel type (e.g. SCALAR, RGB, VECTOR).
     *
     * \return The pixel type enum.
     */
    ItkIOPixelType GetPixelType() const;

    /**
     * \brief Get the scalar component type.
     *
     * Each pixel element may contain multiple components of this scalar type.
     * For example, an RGB pixel of floats has component type FLOAT.
     *
     * \return The component type enum.
     */
    ItkIOComponentType GetComponentType() const;

    /**
     * \brief Get the ITK pixel type as a human-readable string.
     * \return A string such as "scalar", "rgb", "vector", etc.
     */
    std::string GetPixelTypeAsString() const;

    /**
     * \brief Get the scalar component type as a human-readable string.
     * \return A string such as "float", "unsigned_char", etc.
     */
    std::string GetComponentTypeAsString() const;

    /**
     * \brief Get a combined string of pixel type and component type.
     *
     * The format is "PixelTypeName (ComponentTypeName)".
     *
     * \return A descriptive string such as "rgb (float)".
     */
    std::string GetTypeAsString() const;

    /**
     * \brief Get the total size of one pixel in bytes.
     *
     * Equals GetNumberOfComponents() * bytes-per-component. For example,
     * an RGBA float pixel returns 4 * sizeof(float) = 16.
     *
     * \return Size in bytes.
     */
    size_t GetSize() const;

    /**
     * \brief Get the total number of bits per pixel element.
     *
     * Equals GetSize() * 8. For example, a 3-component double vector
     * returns 8 * sizeof(double) * 3 = 192.
     *
     * \return Number of bits per pixel element.
     * \sa GetBitsPerComponent, GetSize
     */
    size_t GetBpe() const;

    /**
     * \brief Get the number of components per pixel.
     *
     * Each pixel can consist of multiple components (e.g. RGB has 3, RGBA has 4,
     * a scalar has 1).
     *
     * \return The number of components.
     */
    size_t GetNumberOfComponents() const;

    /**
     * \brief Get the number of bits per single scalar component.
     *
     * Equals bytes-per-component * 8.
     *
     * \return Number of bits per component.
     * \sa GetBpe
     */
    size_t GetBitsPerComponent() const;

    /**
     * \brief Compare two PixelType objects for equality.
     *
     * Two PixelTypes are equal if they share the same pixel type, component type,
     * number of components, and bytes per component.
     *
     * \param[in] rhs The PixelType to compare against.
     * \return True if equal.
     */
    bool operator==(const PixelType &rhs) const;

    /**
     * \brief Compare two PixelType objects for inequality.
     * \param[in] rhs The PixelType to compare against.
     * \return True if not equal.
     */
    bool operator!=(const PixelType &rhs) const;

    /** \brief Destructor. */
    ~PixelType();

  private:
    friend PixelType MakePixelType(const itk::ImageIOBase *imageIO);

    template <typename ComponentT, typename PixelT>
    friend PixelType MakePixelType(std::size_t numOfComponents);

    template <typename ItkImageType>
    friend PixelType MakePixelType();

    template <typename ItkImageType>
    friend PixelType MakePixelType(size_t);

    PixelType(ItkIOComponentType componentType,
              ItkIOPixelType pixelType,
              std::size_t bytesPerComponent,
              std::size_t numOfComponents,
              const std::string &componentTypeName,
              const std::string &pixelTypeName);

    // default constructor is disabled on purpose
    PixelType(void);

    /** \brief the \a type_info of the scalar (!) component type. Each element
      may contain m_NumberOfComponents (more than one) of these scalars.
    */
    ItkIOComponentType m_ComponentType;

    ItkIOPixelType m_PixelType;

    std::string m_ComponentTypeName;

    std::string m_PixelTypeName;

    std::size_t m_NumberOfComponents;

    std::size_t m_BytesPerComponent;
  };

  /**
   * \brief Deduce and create a PixelType from a vtkImageData object.
   *
   * Maps the VTK scalar type to the corresponding MITK PixelType.
   *
   * \param[in] vtkimagedata The VTK image whose scalar type is used.
   * \return The corresponding mitk::PixelType.
   * \throw mitk::Exception If the VTK scalar type is not supported.
   *
   * \sa MakePixelType, MakeScalarPixelType
   */
  MITKCORE_EXPORT mitk::PixelType MakePixelType(vtkImageData *vtkimagedata);

  /**
   * \brief Create a PixelType from explicit component and pixel type parameters.
   *
   * \tparam ComponentT The C++ scalar component type (e.g. float, short).
   * \tparam PixelT     The pixel type (e.g. itk::RGBPixel<float>).
   * \param[in] numOfComponents The number of components per pixel.
   * \return The constructed PixelType.
   */
  template <typename ComponentT, typename PixelT>
  PixelType MakePixelType(std::size_t numOfComponents)
  {
    return PixelType(MapPixelType<PixelT, isPrimitiveType<PixelT>::value>::IOComponentType,
                     MapPixelType<PixelT, isPrimitiveType<PixelT>::value>::IOPixelType,
                     sizeof(ComponentT),
                     numOfComponents,
                     PixelComponentTypeToString<ComponentT>(),
                     PixelTypeToString<PixelT>());
  }

  /**
   * \brief Create a PixelType with a compile-time number of components.
   *
   * \tparam ComponentT       The C++ scalar component type.
   * \tparam PixelT           The pixel type.
   * \tparam numOfComponents  Compile-time number of components.
   * \return The constructed PixelType.
   */
  template <typename ComponentT, typename PixelT, std::size_t numOfComponents>
  PixelType MakePixelType()
  {
    return MakePixelType<ComponentT, PixelT>(numOfComponents);
  }

  /**
   * \brief Compile-time check for supported ITK image types.
   *
   * The primary template is defined (i.e. valid). Unsupported image types
   * have specializations with missing definitions, causing a compile error
   * if used.
   *
   * \tparam ItkImageType The ITK image type to validate.
   */
  template <typename ItkImageType>
  struct AssertImageTypeIsValid
  {
  };

  // The itk::VariableLengthVector pixel type is not supported in MITK if it is
  // used with an itk::Image (it cannot be represented as a mitk::Image object).
  // Use a itk::VectorImage instead.
  template <typename TPixelType, unsigned int VImageDimension>
  struct AssertImageTypeIsValid<itk::Image<itk::VariableLengthVector<TPixelType>, VImageDimension>>;

  /**
   * \brief Create a MITK PixelType from an ITK image type with a runtime component count.
   *
   * \tparam ItkImageType The ITK image type (e.g. itk::Image<float, 3>).
   * \param[in] numOfComponents The number of components per pixel.
   * \return The constructed PixelType.
   */
  template <typename ItkImageType>
  PixelType MakePixelType(std::size_t numOfComponents)
  {
    AssertImageTypeIsValid<ItkImageType>();

    // define new type, since the ::PixelType is used to distinguish between simple and compound types
    typedef typename ItkImageType::PixelType ImportPixelType;

    // get the component type ( is either directly ImportPixelType or ImportPixelType::ValueType for compound types )
    typedef typename GetComponentType<ImportPixelType>::ComponentType ComponentT;

    // The PixelType is the same as the ComponentT for simple types
    typedef typename ItkImageType::PixelType PixelT;

    // call the constructor
    return PixelType(MapPixelType<PixelT, isPrimitiveType<PixelT>::value>::IOComponentType,
                     MapPixelType<PixelT, isPrimitiveType<PixelT>::value>::IOPixelType,
                     sizeof(ComponentT),
                     numOfComponents,
                     PixelComponentTypeToString<ComponentT>(),
                     PixelTypeToString<PixelT>());
  }

  /** \brief Create a MITK PixelType from an ITK pixel type and image dimension.
   *
   * Convenience overload that constructs the ITK image type from the pixel type and
   * dimension, then delegates to MakePixelType<ItkImageType>(std::size_t).
   *
   * \tparam TPixelType       The ITK pixel type (e.g. itk::RGBPixel<unsigned char>).
   * \tparam VImageDimension  The image dimension (e.g. 2 or 3).
   * \return The constructed PixelType.
   */
  template <typename TPixelType, unsigned int VImageDimension>
  PixelType MakePixelType(std::size_t numOfComponents)
  {
    typedef typename ImageTypeTrait<TPixelType, VImageDimension>::ImageType ItkImageType;
    return MakePixelType<ItkImageType>(numOfComponents);
  }

  /**
   * \brief Create a MITK PixelType from an ITK image type with compile-time component count.
   *
   * The number of components is deduced from the ITK pixel type at compile time.
   * For images where the component count is determined at runtime (e.g.
   * itk::VectorImage), use MakePixelType<ItkImageType>(std::size_t) instead.
   *
   * \tparam ItkImageType The ITK image type.
   * \return The constructed PixelType.
   * \throw mitk::Exception If the pixel type has a variable length (e.g. VectorImage).
   */
  template <typename ItkImageType>
  PixelType MakePixelType()
  {
    if (ImageTypeTrait<ItkImageType>::IsVectorImage)
    {
      mitkThrow() << " Variable pixel type given but the length is not specified. Use the parametric MakePixelType( "
                     "size_t ) method instead.";
    }

    // Use the InternalPixelType to get "1" for the number of components in case of
    // a itk::VectorImage
    typedef typename ItkImageType::InternalPixelType PixelT;

    const std::size_t numComp = ComponentsTrait<isPrimitiveType<PixelT>::value, ItkImageType>::Size;

    // call the constructor
    return MakePixelType<ItkImageType>(numComp);
  }

  /**
   * \brief Create a MITK PixelType from an itk::ImageIOBase object.
   *
   * Extracts component type, pixel type, sizes, and names from the I/O object.
   *
   * \param[in] imageIO The ITK ImageIOBase instance to query.
   * \return The constructed PixelType.
   */
  inline PixelType MakePixelType(const itk::ImageIOBase *imageIO)
  {
    return mitk::PixelType(imageIO->GetComponentType(),
                           imageIO->GetPixelType(),
                           imageIO->GetComponentSize(),
                           imageIO->GetNumberOfComponents(),
                           imageIO->GetComponentTypeAsString(imageIO->GetComponentType()),
                           imageIO->GetPixelTypeAsString(imageIO->GetPixelType()));
  }

  /**
   * \brief Create a scalar (single-component) PixelType from a C++ type.
   *
   * Convenience wrapper equivalent to MakePixelType<T, T, 1>().
   *
   * Usage example:
   * \code
   * mitk::PixelType pt = mitk::MakeScalarPixelType<short>();
   * \endcode
   *
   * \tparam T The C++ scalar type (e.g. short, float, unsigned char).
   * \return A PixelType representing a single-component pixel of type T.
   */
  template <typename T>
  PixelType MakeScalarPixelType()
  {
    return MakePixelType<T, T, 1>();
  }

} // namespace mitk

#endif
