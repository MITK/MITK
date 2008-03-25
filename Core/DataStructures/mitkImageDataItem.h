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
class MITK_CORE_EXPORT ImageDataItem : public itk::LightObject
{
public:
  mitkClassMacro(ImageDataItem, itk::LightObject);
  
  ImageDataItem(const ImageDataItem& aParent, unsigned int dimension, void *data = NULL, bool manageMemory = false, size_t offset = 0);
  
  ~ImageDataItem();

  ImageDataItem(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions, void *data, bool manageMemory);

  void* GetData() const
  {
    return m_Data;
  }

  bool IsComplete() const
  {
    return m_IsComplete;
  }
  void SetComplete(bool complete)
  {
    m_IsComplete = complete;
  }

  int GetOffset() const
  {
    return m_Offset;
  }
  
  PixelType GetPixelType() const
  {
    return m_PixelType;    
  }
  
  ImageDataItem::ConstPointer GetParent() const
  {
    return m_Parent;
  }

  ipPicDescriptor* GetPicDescriptor() const
  {
    return m_PicDescriptor;
  }

  vtkImageData* GetVtkImageData() const
  {
    if(m_VtkImageData==NULL)
      ConstructVtkImageData();
    return m_VtkImageData;
  }

  bool GetManageMemory() const
  {
    return m_ManageMemory;
  }

  virtual void ConstructVtkImageData() const;

  unsigned long GetSize() const
  {
    return m_Size;
  }

  virtual void Modified() const;
  
protected:
  unsigned char* m_Data;

  PixelType m_PixelType;
  
  bool m_ManageMemory;

  ipPicDescriptor* m_PicDescriptor;
  mutable vtkImageData* m_VtkImageData;
  int m_Offset;

  bool m_IsComplete;

  unsigned long m_Size;
  
private:
  ImageDataItem::ConstPointer m_Parent;
};

} // namespace mitk

#endif /* IMAGEDATAITEM_H_HEADER_INCLUDED_C1F4DAB4 */
