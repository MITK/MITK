/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef PIXELTYPE_H_HEADER_INCLUDED_C1EBF565
#define PIXELTYPE_H_HEADER_INCLUDED_C1EBF565

#include "mitkCommon.h"
#include <mitkIpPic.h>
#include <itkImageIOBase.h>

namespace mitk {

//##Documentation
//## @brief Class for defining the data type of pixels 
//##
//## To obtain additional type information not provided by this class
//## itk::ImageIOBase can be used by passing the return value of 
//## PixelType::GetItkTypeId() to itk::ImageIOBase::SetPixelTypeInfo
//## and using the itk::ImageIOBase methods GetComponentType,
//## GetComponentTypeAsString, GetPixelType, GetPixelTypeAsString.
//## @ingroup Data
class MITK_CORE_EXPORT PixelType
{
public:
  itkTypeMacro(PixelType, None);

  typedef itk::ImageIOBase::IOPixelType ItkIOPixelType;

  PixelType(const std::type_info& aTypeId, int numberOfComponents = 1, ItkIOPixelType anItkIoPixelType = itk::ImageIOBase::UNKNOWNPIXELTYPE);

  PixelType(ipPicType_t type, int bpe, int numberOfComponents = 1);

  PixelType(const ipPicDescriptor* pic);
  PixelType(const mitk::PixelType & aPixelType);
  /** \brief Get the \a type_info of the scalar (!) type. Each element
  * may contain m_NumberOfComponents (more than one) of these scalars.
  *
  * \sa GetItkTypeId
  */
  inline const std::type_info * const GetTypeId() const
  {
    return m_TypeId;
  }
  /** \brief Get the \a type_info of the ITK-type representing an 
  * element. Can be \a NULL in case the ITK-type is unknown.
  *
  * \sa GetTypeId
  */
  inline const std::type_info * const GetItkTypeId() const
  {
    return m_ItkTypeId;
  }

  /** \brief Get the \a ipPicType_t of the scalar (!) component type. Each element
  * may contain m_NumberOfComponents (more than one) of these scalars.
  *
  * \sa GetItkTypeId
  * \sa GetTypeId
  */
  inline ipPicType_t GetType() const
  {
    return m_Type;
  }

  /** \brief Get the number of bits per element (of an
  * element)
  *
  * A vector of double with three components will return
  * 8*sizeof(double)*3.
  * \sa GetBitsPerComponent
  * \sa GetItkTypeId
  * \sa GetTypeId
  */
  inline int GetBpe() const
  {
    return m_Bpe;
  }

  /** \brief Get the number of components of which each element consists
  *
  * Each pixel can consist of multiple components, e.g. RGB.
  */
  inline int GetNumberOfComponents() const
  {
    return m_NumberOfComponents;    
  }
  
  /** \brief Get the number of bits per components
  * \sa GetBitsPerComponent
  */
  inline int GetBitsPerComponent() const
  {
    return m_BitsPerComponent;    
  }
  
  /** \brief Get the typename of the ITK-type representing an 
  * element as a human-readable string
  *
  * \sa GetItkTypeId
  */
  std::string GetItkTypeAsString() const;

  inline PixelType& operator=(const PixelType& aPixelType)
  {
    m_TypeId=aPixelType.GetTypeId();
    m_ItkTypeId=aPixelType.GetItkTypeId();
    m_Type=aPixelType.GetType();
    m_Bpe=aPixelType.GetBpe();
    m_NumberOfComponents = aPixelType.GetNumberOfComponents();
    m_BitsPerComponent = aPixelType.GetBitsPerComponent();
    return *this;
  }

  bool operator==(const PixelType& rhs) const;
  bool operator!=(const PixelType& rhs) const;

  bool operator==(const std::type_info& typeId) const;
  bool operator!=(const std::type_info& typeId) const;

  PixelType();

  void Initialize(const std::type_info& aTypeId, int numberOfCompontents = 1, ItkIOPixelType anItkIoPixelType = itk::ImageIOBase::UNKNOWNPIXELTYPE);

  void Initialize(ipPicType_t type, int bpe, int numberOfComponents = 1);

  virtual ~PixelType() {}

private:
  /** \brief the \a type_info of the scalar (!) component type. Each element
   * may contain m_NumberOfComponents (more than one) of these scalars.
   *
   * \sa m_ItkTypeId
   */
  const std::type_info* m_TypeId;
  /** \brief the \a type_info of the ITK-type representing an 
   * element
   *
   * \sa m_TypeId
   */
  const std::type_info* m_ItkTypeId;

  ipPicType_t m_Type;

  int m_Bpe;
  
  int m_NumberOfComponents;
  
  int m_BitsPerComponent;

};

} // namespace mitk

#endif /* PIXELTYPE_H_HEADER_INCLUDED_C1EBF565 */
