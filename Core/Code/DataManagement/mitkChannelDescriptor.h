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


#ifndef MITKCHANNELDESCRIPTOR_H
#define MITKCHANNELDESCRIPTOR_H

#include <itkObjectFactory.h>
#include "mitkPixelType.h"

#include <string>


namespace mitk
{

class ChannelDescriptor : public itk::Object
{
public:
    mitkClassMacro(ChannelDescriptor, itk::Object)

    itkNewMacro(Self)

    void Initialize(mitk::PixelType& type, size_t numOfElements, bool allocate = false);

    PixelType GetPixelType() const
    { return m_PixelType; }

    const size_t GetSize() const
    { return m_Size; }

    void* GetData() const
    { return m_Data; }

protected:
    void AllocateData();

    ChannelDescriptor();

    ~ChannelDescriptor(){}

    std::string m_Name;

    PixelType m_PixelType;

    size_t m_Size;

    unsigned char* m_Data;

};

} // end namespace mitk
#endif // MITKCHANNELDESCRIPTOR_H
