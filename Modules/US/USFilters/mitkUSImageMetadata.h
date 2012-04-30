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


#ifndef MITKUSIMAGEMETADATA_H_HEADER_INCLUDED_
#define MITKUSIMAGEMETADATA_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <itkObject.h>
#include "mitkUSDevice.h"
#include "mitkUSProbe.h"
#include <itkObjectFactory.h>

namespace mitk {

    /**Documentation
    * \brief TODO
    * \ingroup US
    */
  class MitkUS_EXPORT USImageMetadata : public itk::Object
    {
    public:
      mitkClassMacro(USImageMetadata, itk::Object);
      itkNewMacro(Self);
    
      //## getter and setter ##
      itkGetMacro(Device, mitk::USDevice::Pointer);
      itkSetMacro(Device, mitk::USDevice::Pointer);
      itkGetMacro(Probe,  mitk::USProbe::Pointer);
      itkSetMacro(Probe,  mitk::USProbe::Pointer);

    protected:
      USImageMetadata();
      virtual ~USImageMetadata();

      USDevice::Pointer m_Device;
      USProbe::Pointer  m_Probe;

    

    };
} // namespace mitk
#endif