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


#ifndef IMAGEDATAITEM_H
#define IMAGEDATAITEM_H

#include "mitkCommon.h"
#include <MitkExports.h>
//#include <mitkIpPic.h>
//#include "mitkPixelType.h"
#include "mitkImageDescriptor.h"

class vtkImageData;

namespace mitk {

  class PixelType;

  //##Documentation
  //## @brief Internal class for managing references on sub-images
  //##
  //## ImageDataItem is a container for image data which is used internal in  
  //## mitk::Image to handle the communication between the different data types for images
  //## used in MITK (ipPicDescriptor, mitk::Image, vtkImageData). Common for these image data 
  //## types is the actual image data, but they differ in representation of pixel type etc.
  //## The class is also used to convert ipPic images to vtkImageData. 
  //##
  //## The class is mainly used to extract sub-images inside of mitk::Image, like single slices etc.
  //## It should not be used outside of this.
  //## 
  //## @param manageMemory Determines if image data is removed while destruction of ImageDataItem or not.
  //## @ingroup Data
  class MITK_CORE_EXPORT ImageDataItem : public itk::LightObject
  {
  public:
    mitkClassMacro(ImageDataItem, itk::LightObject);

    ImageDataItem(const ImageDataItem& aParent, const mitk::ImageDescriptor::Pointer desc, unsigned int dimension, void *data = NULL, bool manageMemory = false, size_t offset = 0);

    ~ImageDataItem();

    ImageDataItem(const mitk::ImageDescriptor::Pointer desc, void *data, bool manageMemory);

    ImageDataItem(const mitk::PixelType& type, unsigned int dimension, unsigned int* dimensions, void* data, bool manageMemory);

    ImageDataItem(const ImageDataItem &other);

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
      return *m_PixelType;
    }

    int GetDimension() const
    {
      return m_Dimension;
    }

    int GetDimension(int i) const
    {
      int returnValue = 0;

      // return the true size if dimension available
      if (i< (int) m_Dimension)
        returnValue = m_Dimensions[i];

      return returnValue;
    }

    ImageDataItem::ConstPointer GetParent() const
    {
      return m_Parent;
    }

    //## Returns a vtkImageData; if non is present, a new one is constructed.
    vtkImageData* GetVtkImageData() const
    {
      if(m_VtkImageData==NULL)
        ConstructVtkImageData();
      return m_VtkImageData;
    }

    // Returns if image data should be deleted on destruction of ImageDataItem.
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

    PixelType *m_PixelType;

    bool m_ManageMemory;

    mutable vtkImageData* m_VtkImageData;
    int m_Offset;

    bool m_IsComplete;

    unsigned long m_Size;

  private:
    void ComputeItemSize( const unsigned int* dimensions, unsigned int dimension);

    ImageDataItem::ConstPointer m_Parent;

    template <class TPixeltype>
    unsigned char *ConvertTensorsToRGB() const;

    unsigned int m_Dimension;

    unsigned int m_Dimensions[MAX_IMAGE_DIMENSIONS];

  };

} // namespace mitk

#endif /* IMAGEDATAITEM_H */
