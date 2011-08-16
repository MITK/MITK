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

// The maximum of 8 dimensions per image channel taken from ipPicDescriptor
#define MAX_IMAGE_DIMENSIONS 8

namespace mitk
{

class ImageDescriptor : public itk::Object
{
  public:
    mitkClassMacro(ImageDescriptor, itk::Object);

    itkNewMacro(Self);

    /** Insert new channel

       @param chDesc Channel Descriptor
       @param name channel's name
    */
    void AddNewChannel( mitk::PixelType ptype, const char* name = 0);

    void Initialize( const unsigned int* dims, const unsigned int dim);

    void Initialize( const ImageDescriptor::Pointer refDescriptor, unsigned int channel = 0);

    const unsigned int* GetDimensions()
    { return this->m_Dimensions; }

    const unsigned int GetNumberOfDimensions()
    { return this->m_NumberOfDimensions; }

    const std::string GetChannelName( unsigned int id ) const;

    PixelType GetChannelTypeByName( const char* name) const;

    PixelType GetChannelTypeById( unsigned int id) const;

    ChannelDescriptor::Pointer GetChannelDescriptor( unsigned int id) const;

    unsigned int GetNumberOfChannels()
    { return this->m_NumberOfChannels; }

  protected:
    ImageDescriptor();

    ~ImageDescriptor(){};

  private:
    std::vector<ChannelDescriptor::Pointer> m_ChannelDesc;

    std::vector<std::string> m_ChannelNames;

    typedef std::vector<std::string>::const_iterator ConstChannelNamesIter;

    typedef std::vector<ChannelDescriptor::Pointer>::const_iterator ConstChannelsIter;

    unsigned int m_NumberOfChannels;

    unsigned int m_NumberOfDimensions;

    unsigned int m_Dimensions[MAX_IMAGE_DIMENSIONS];
};

} // end namespace
#endif // MITKIMAGEDESCRIPTOR_H
