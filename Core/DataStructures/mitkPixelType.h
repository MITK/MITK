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
#include <ipPic/ipPic.h>

namespace mitk {

//##ModelId=3DF45FE9023C
//##Documentation
//## @brief Class for defining the data type of pixels 
//## @ingroup Data
class MITK_CORE_EXPORT PixelType
{
public:
  itkTypeMacro(PixelType, None);

  //##ModelId=3E1400C40198
  PixelType(const std::type_info& aTypeId, int numberOfComponents = 1);

  //##ModelId=3E1400150088
  PixelType(ipPicType_t type, int bpe, int numberOfComponents = 1);

  //##ModelId=3E1400060113
  PixelType(const ipPicDescriptor* pic);
  //##ModelId=3E1412720060
  PixelType(const mitk::PixelType & aPixelType);
  //##ModelId=3E14131D0246
  inline const std::type_info * const GetTypeId() const
  {
    return m_TypeId;
  }


  //##ModelId=3E1413040237
  inline ipPicType_t GetType() const
  {
    return m_Type;
  }


  //##ModelId=3E1413110195
  inline int GetBpe() const
  {
    return m_Bpe;
  }

  inline int GetNumberOfComponents() const
  {
    return m_NumberOfComponents;    
  }
  
  inline int GetBitsPerComponent() const
  {
    return m_BitsPerComponent;    
  }
  
  
  //##ModelId=3E1550B401ED
  inline PixelType& operator=(const PixelType& aPixelType)
  {
    m_TypeId=aPixelType.GetTypeId();
    m_Type=aPixelType.GetType();
    m_Bpe=aPixelType.GetBpe();
    m_NumberOfComponents = aPixelType.GetNumberOfComponents();
    m_BitsPerComponent = aPixelType.GetBitsPerComponent();
    return *this;
  }

  bool operator==(const PixelType& rhs) const;
  bool operator!=(const PixelType& rhs) const;

  //##ModelId=3E15F73502BB
  PixelType();

  void Initialize(const std::type_info& aTypeId, int numberOfCompontents = 1);

  //##ModelId=3E140E4F00E9
  void Initialize(ipPicType_t type, int bpe, int numberOfComponents = 1);
  virtual ~PixelType() {}

private:
  //##ModelId=3E13FFAD038C
  const std::type_info* m_TypeId;

  //##ModelId=3E1400610236
  ipPicType_t m_Type;

  //##ModelId=3E14006C02E6
  int m_Bpe;
  
  int m_NumberOfComponents;
  
  int m_BitsPerComponent;

};

} // namespace mitk



#endif /* PIXELTYPE_H_HEADER_INCLUDED_C1EBF565 */
