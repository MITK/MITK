/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageDescriptor_h
#define mitkImageDescriptor_h

#include <itkObjectFactory.h>

#include <string>
#include <vector>

#include <mitkChannelDescriptor.h>
#include <mitkCommon.h>

/** \brief Maximum number of dimensions supported per image channel. */
#define MAX_IMAGE_DIMENSIONS 8

namespace mitk
{
  /**
   * \brief Holds all essential metadata about an Image object.
   *
   * The ImageDescriptor stores a vector of ChannelDescriptor objects together with
   * the image's dimension sizes. The general assumption is that every channel of
   * an image shares the same geometry; only the pixel type may differ between
   * channels.
   *
   * \sa Image, ChannelDescriptor, PixelType
   * \ingroup Data
   */
  class MITKCORE_EXPORT ImageDescriptor : public itk::Object
  {
  public:
    mitkClassMacroItkParent(ImageDescriptor, itk::Object);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /**
     * \brief Add a new channel to this image descriptor.
     *
     * The channel's element count is computed from the current dimension sizes.
     * If no name is given, a default name "Unnamed [PixelTypeName]" is assigned.
     *
     * \param[in] ptype The pixel type of the new channel.
     * \param[in] name  Optional human-readable name for the channel.
     */
    void AddNewChannel(mitk::PixelType ptype, const char *name = nullptr);

    /**
     * \brief Initialize the image descriptor with dimension sizes.
     * \param[in] dims Array of dimension sizes (must have at least \p dim elements).
     * \param[in] dim  Number of dimensions to use (must not exceed MAX_IMAGE_DIMENSIONS).
     */
    void Initialize(const unsigned int *dims, const unsigned int dim);

    /**
     * \brief Initialize this descriptor from an existing ImageDescriptor.
     *
     * Copies dimension information and one channel descriptor from the reference.
     *
     * \param[in] refDescriptor The source descriptor to copy from.
     * \param[in] channel       Index of the channel to copy (default: 0).
     */
    void Initialize(const ImageDescriptor::Pointer refDescriptor, unsigned int channel = 0);

    /**
     * \brief Get the array of dimension sizes.
     *
     * The returned C-array always has MAX_IMAGE_DIMENSIONS elements. Unused
     * dimensions are set to 1.
     *
     * \return Pointer to the internal dimension size array.
     */
    const unsigned int *GetDimensions() const { return m_Dimensions; }

    /**
     * \brief Get the number of active dimensions.
     *
     * The return value does not exceed MAX_IMAGE_DIMENSIONS.
     *
     * \return The number of dimensions with meaningful size.
     */
    unsigned int GetNumberOfDimensions() const { return m_NumberOfDimensions; }

    /**
     * \brief Get the name of a channel by its index.
     *
     * If the channel name was not explicitly set, the returned string is
     * "Unnamed [PixelTypeName]". Returns "Out-of-range-access" if \p id
     * exceeds the channel count.
     *
     * \param[in] id Zero-based channel index.
     * \return The channel name string.
     *
     * \sa PixelType, ChannelDescriptor
     */
    const std::string GetChannelName(unsigned int id) const;

    /**
     * \brief Get the pixel type of a channel identified by name.
     *
     * \param[in] name The channel name to search for.
     * \return The PixelType of the matching channel.
     * \throw std::invalid_argument If no channel with the given name is found.
     */
    PixelType GetChannelTypeByName(const char *name) const;

    /**
     * \brief Get the pixel type of a channel identified by index.
     *
     * \param[in] id Zero-based channel index.
     * \return The PixelType of the specified channel.
     * \throw std::invalid_argument If \p id exceeds the number of active channels.
     */
    PixelType GetChannelTypeById(unsigned int id) const;

    /**
     * \brief Get the ChannelDescriptor for a specific channel.
     * \param[in] id Zero-based channel index (default: 0).
     * \return A copy of the ChannelDescriptor.
     * \throw std::invalid_argument If \p id exceeds the number of active channels.
     */
    ChannelDescriptor GetChannelDescriptor(unsigned int id = 0) const;

    /**
     * \brief Get the total number of channels.
     * \return The channel count.
     */
    unsigned int GetNumberOfChannels() const { return m_NumberOfChannels; }
  protected:
    /** Protected constructor */
    ImageDescriptor();

    /** Protected destructor */
    ~ImageDescriptor() override{};

  private:
    /** A std::vector holding a pointer to a ChannelDescriptor for each active channel of the image */
    std::vector<ChannelDescriptor> m_ChannelDesc;

    /** A vector holding the names of corresponding channels */
    std::vector<std::string> m_ChannelNames;

    /** Constant iterator for traversing the vector of channel's names */
    typedef std::vector<std::string>::const_iterator ConstChannelNamesIter;

    /** Constant iterator for traversing the vector of ChannelDescriptors */
    typedef std::vector<ChannelDescriptor>::const_iterator ConstChannelsIter;

    unsigned int m_NumberOfChannels;

    unsigned int m_NumberOfDimensions;

    unsigned int m_Dimensions[MAX_IMAGE_DIMENSIONS];
  };

} // end namespace
#endif
