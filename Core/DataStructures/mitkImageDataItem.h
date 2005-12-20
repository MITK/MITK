/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef IMAGEDATAITEM_H_HEADER_INCLUDED_C1F4DAB4
#define IMAGEDATAITEM_H_HEADER_INCLUDED_C1F4DAB4

#include "mitkCommon.h"
#include "mitkPixelType.h"

class vtkImageData;

namespace mitk {

//##ModelId=3D7B424502D7
//##Documentation
//## @brief Internal class for managing references on sub-images
//## @ingroup Data
class ImageDataItem : public itk::LightObject
{
public:
  mitkClassMacro(ImageDataItem, itk::LightObject);
  
  //##ModelId=3E0B7882024B
  ImageDataItem(const ImageDataItem& aParent, unsigned int dimension, size_t offset = 0);
  
  //##ModelId=3E0B78820287
  ~ImageDataItem();
  //##ModelId=3E159C240213
  ImageDataItem(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions);
  //##ModelId=3E19F8F00098
  void* GetData() const
  {
    return m_Data;
  }
  //##ModelId=3E19F8F00188
  bool IsComplete() const
  {
    return m_IsComplete;
  }
  //##ModelId=3E1A02A10244
  void SetComplete(bool complete)
  {
    m_IsComplete = complete;
  }
  //##ModelId=3E19F8F0025A
  int GetOffset() const
  {
    return m_Offset;
  }
  
  PixelType GetPixelType() const
  {
    return m_PixelType;    
  }
  
  //##ModelId=3E19F8F0032C
  ImageDataItem::ConstPointer GetParent() const
  {
    return m_Parent;
  }
  //##ModelId=3E19F8F1002B
  ipPicDescriptor* GetPicDescriptor() const
  {
    return m_PicDescriptor;
  }
  //##ModelId=3E19F8F10111
  vtkImageData* GetVtkImageData() const
  {
    if(m_VtkImageData==NULL)
      ConstructVtkImageData();
    return m_VtkImageData;
  }
  //##ModelId=3E33F08A03B8
  virtual void ConstructVtkImageData() const;
  
  //##ModelId=3E70F51001F2
  virtual void Modified() const;
  
protected:
  //##ModelId=3D7B42E90201
  unsigned char* m_Data;
  //##ModelId=3E0B487201C0
  ipPicDescriptor* m_PicDescriptor;
  //##ModelId=3E0B488C0380
  mutable vtkImageData* m_VtkImageData;
  //##ModelId=3E0B89080207
  int m_Offset;
  //##ModelId=3E156DC500D4
  bool m_IsComplete;
  
  PixelType m_PixelType;
  
private:
  //##ModelId=3D7B425E0337
  ImageDataItem::ConstPointer m_Parent;
  
};

} // namespace mitk



#endif /* IMAGEDATAITEM_H_HEADER_INCLUDED_C1F4DAB4 */
