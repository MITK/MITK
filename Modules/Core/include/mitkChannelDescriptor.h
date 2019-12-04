/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCHANNELDESCRIPTOR_H
#define MITKCHANNELDESCRIPTOR_H

#include "mitkPixelType.h"
#include <string>

namespace mitk
{
  /** \brief An object which holds all essential information about a single channel of an Image.

    The channel descriptor is designed to be used only as a part of the ImageDescriptor. A consequence
  to this is that the ChannelDescriptor does not hold the geometry information, only the PixelType.
  The pixel type is the single information that can differ among an image with multiple channels.
  */
  class MITKCORE_EXPORT ChannelDescriptor
  {
  public:
    ChannelDescriptor(mitk::PixelType type, size_t numOfElements, bool allocate = false);

    ~ChannelDescriptor();

    /** \brief Get the type of channel's elements */
    PixelType GetPixelType() const { return m_PixelType; }
    /** \brief Get the size in bytes of the channel */
    size_t GetSize() const { return m_Size; }
    /** \brief Get the pointer to the actual data of the channel

      \warning Such access to the image's data is not safe and will be replaced
      \todo new memory management design
    */
    unsigned char *GetData() const { return m_Data; }
  protected:
    friend class Image;
    friend class ImageAccessorBase;

    void SetData(void *dataPtr)
    {
      if (dataPtr == nullptr)
      {
        m_Data = (unsigned char *)dataPtr;
      }
    }

    void AllocateData();

    /** Name of the channel */
    std::string m_Name;

    /** The type of each element of the channel

      \sa PixelType */
    PixelType m_PixelType;

    /** Size of the channel in bytes */
    size_t m_Size;

    /** Pointer to the data of the channel

      \warning Not safe
      \todo Replace in new memory management design
      */
    unsigned char *m_Data;
  };

} // end namespace mitk
#endif // MITKCHANNELDESCRIPTOR_H
