/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkImageAccessorBase.h"
#include "mitkImage.h"

  mitk::ImageAccessorBase::ImageAccessorBase(
      ImagePointer iP,
      ImageDataItem* imageDataItem,
      int OptionFlags
    ) :
    m_Image(iP),
//    imageDataItem(iDI),
    m_SubRegion(NULL),
    m_Options(OptionFlags),
    m_CoherentMemory(false)
    {
      // Initialize WaitLock
      m_WaitLock = new ImageAccessorWaitLock();
      m_WaitLock->m_WaiterCount = 0;


      // Check validity of ImageAccessor

      // Is there an Image?
      if(!m_Image)
      {
        mitkThrow() << "Invalid ImageAccessor: No Image was specified in constructor of ImageAccessor";
      }

      // Make sure, that the Image is initialized properly
      if(m_Image->m_Initialized==false)
      {
        if(m_Image->GetSource().IsNull())
          mitkThrow() << "ImageAccessor: No image source is defined";
        if(m_Image->GetSource()->Updating()==false)
          m_Image->GetSource()->UpdateOutputInformation();
      }
      m_Image->m_CompleteData=m_Image->GetChannelData();

      // update channel's data
      // if data was not available at creation point, the m_Data of channel descriptor is NULL
      // if data present, it won't be overwritten
      m_Image->m_ImageDescriptor->GetChannelDescriptor(0).SetData(m_Image->m_CompleteData->m_Data);

      // Investigate 4 cases of possible image parts/regions

      // Case 1: No ImageDataItem and no Subregion => Whole Image is accessed
      if(imageDataItem == NULL && m_SubRegion == NULL)
      {
        m_CoherentMemory = true;

        // Organize first image channel
        imageDataItem = m_Image->GetChannelData();

        // Set memory area
        m_AddressBegin = imageDataItem->m_Data;
        m_AddressEnd   = (unsigned char*) m_AddressBegin + imageDataItem->m_Size;
      }

      // Case 2: ImageDataItem but no Subregion
      if(imageDataItem && m_SubRegion == NULL)
      {
        m_CoherentMemory = true;

        // Set memory area
        m_AddressBegin = imageDataItem->m_Data;
        m_AddressEnd   = (unsigned char*) m_AddressBegin + imageDataItem->m_Size;
      }

      // Case 3: No ImageDataItem but a SubRegion
      if(imageDataItem == NULL && m_SubRegion)
      {
          mitkThrow() << "Invalid ImageAccessor: The use of a SubRegion is not supported (yet).";
      }

      // Case 4: ImageDataItem and SubRegion
      if(imageDataItem == NULL && m_SubRegion)
      {
          mitkThrow() << "Invalid ImageAccessor: The use of a SubRegion is not supported (yet).";
      }

    }



  /** \brief Computes if there is an Overlap of the image part between this instantiation and another ImageAccessor object
    * \throws mitk::Exception if memory area is incoherent (not supported yet)
    */
  bool mitk::ImageAccessorBase::Overlap(const ImageAccessorBase* iAB)
  {
    if(m_CoherentMemory)
    {
      if((iAB->m_AddressBegin >= m_AddressBegin && iAB->m_AddressBegin <  m_AddressEnd) ||
         (iAB->m_AddressEnd   >  m_AddressBegin && iAB->m_AddressEnd   <= m_AddressEnd))
      {
        return true;
      }
      if((m_AddressBegin >= iAB->m_AddressBegin && m_AddressBegin <  iAB->m_AddressEnd) ||
         (m_AddressEnd   >  iAB->m_AddressBegin && m_AddressEnd   <= iAB->m_AddressEnd))
      {
        return true;
      }

    }
    else mitkThrow() << "ImageAccessor: incoherent memory area is not supported yet";

    return false;
  }

  /** \brief Uses the WaitLock to wait for another ImageAccessor*/
  void mitk::ImageAccessorBase::WaitForReleaseOf(ImageAccessorWaitLock* wL) {
    wL->m_Mutex.Lock();

    // Decrement
    wL->m_WaiterCount -= 1;

    // If there are no more waiting ImageAccessors, delete the Mutex
    // (Der Letzte macht das Licht aus!)
    if(wL->m_WaiterCount <= 0)
    {
      wL->m_Mutex.Unlock();
      delete wL;
    }
    else
    {
      wL->m_Mutex.Unlock();
    }
  }

