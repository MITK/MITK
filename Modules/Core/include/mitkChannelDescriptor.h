/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkChannelDescriptor_h
#define mitkChannelDescriptor_h

#include <mitkPixelType.h>
#include <string>

namespace mitk
{
  /**
   * \brief Holds essential information about a single channel of an Image.
   *
   * Designed to be used only as part of an ImageDescriptor. The ChannelDescriptor
   * does not hold geometry information -- only the PixelType, which is the piece
   * of metadata that can differ among channels of a multi-channel image.
   *
   * \sa ImageDescriptor, PixelType, Image
   * \ingroup Data
   */
  class MITKCORE_EXPORT ChannelDescriptor
  {
  public:
    /**
     * \brief Construct a ChannelDescriptor with a given pixel type and element count.
     * \param[in] type           The pixel type for this channel.
     * \param[in] numOfElements  The number of pixel elements in this channel.
     * \param[in] allocate       Currently unused; reserved for future memory management.
     */
    ChannelDescriptor(mitk::PixelType type, size_t numOfElements, bool allocate = false);

    /** \brief Destructor. */
    ~ChannelDescriptor();

    /**
     * \brief Get the pixel type of this channel's elements.
     * \return The PixelType describing the element data type.
     */
    PixelType GetPixelType() const { return m_PixelType; }

    /**
     * \brief Get the number of elements stored in this channel.
     * \return The element count (not the byte size).
     */
    size_t GetSize() const { return m_Size; }

    /**
     * \brief Get a raw pointer to the channel's data buffer.
     *
     * \warning Direct access to image data is unsafe. Prefer
     *          ImageReadAccessor or ImageWriteAccessor instead.
     *
     * \return Pointer to the raw data, or nullptr if not allocated.
     *
     * \todo Replace with a safe accessor in the new memory management design.
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
#endif
