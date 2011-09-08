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


#ifndef MITKIMAGEDESCRIPTOR_H
#define MITKIMAGEDESCRIPTOR_H

#include <itkObjectFactory.h>

#include <vector>
#include <string>

#include "mitkChannelDescriptor.h"
#include "mitkCommon.h"

/// Defines the maximum of 8 dimensions per image channel taken from ipPicDescriptor
#define MAX_IMAGE_DIMENSIONS 8

namespace mitk
{

/** \brief An object to hold all essential information about an Image object

  The ImageDescriptor holds an std::vector of pointers to ChannelDescriptor together with the
  information about the image's dimensions. The general assumption ist, that each channel of an image
  has to have the same geometry.

  \sa Image, ChannelDescriptor
  */
class MITK_CORE_EXPORT ImageDescriptor : public itk::Object
{
  public:
    mitkClassMacro(ImageDescriptor, itk::Object);

    itkNewMacro(Self);

    /** Insert new channel

       @param chDesc Channel Descriptor
       @param name channel's name
    */
    void AddNewChannel( mitk::PixelType ptype, const char* name = 0);

    /** \brief Initialize the image descriptor by the dimensions */
    void Initialize( const unsigned int* dims, const unsigned int dim);

    /** \brief Initialize the descriptor by an referenced Descriptor */
    void Initialize( const ImageDescriptor::Pointer refDescriptor, unsigned int channel = 0);

    /** \brief Get the C-array of unsigned int holding the size for each dimension of the image

      The C-array has allways lenght of MAX_IMAGE_DIMENSIONS
      */
    const unsigned int* GetDimensions() const
    { return m_Dimensions; }

    /** \brief Get the number dimensions used (e.g. non-zero size)

      The return value does not exceed MAX_IMAGE_DIMENSIONS
      */
    unsigned int GetNumberOfDimensions() const
    { return m_NumberOfDimensions; }

    /** \brief Get the name of selected channel

      If the name of the channel wasn't initialized, the string returned is set to "Unnamed [ <PixelTypeName> ]"

      \sa PixelType, ChannelDescriptor
    */
    const std::string GetChannelName( unsigned int id ) const;

    /** \brief Get the pixel type of a channel specified by its name

      Returns an uninitialized PixelType object if no channel with given name was found
      */
    PixelType GetChannelTypeByName( const char* name) const;

    /** \brief Get the pixel type of a channel specified by its id

      Returns an uninitialized PixelType object if no channel with given id was found
      */
    PixelType GetChannelTypeById( unsigned int id) const;

    /** \brief Get the ChannelDescriptor for a channel specified by its id      */
    ChannelDescriptor GetChannelDescriptor( unsigned int id = 0) const;

    /** \brief Get the count of channels used */
    unsigned int GetNumberOfChannels() const
    { return m_NumberOfChannels; }

  protected:
    /** Protected constructor */
    ImageDescriptor();

    /** Protected desctructor */
    ~ImageDescriptor(){};

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
#endif // MITKIMAGEDESCRIPTOR_H
