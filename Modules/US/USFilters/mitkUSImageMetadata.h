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
      itkGetMacro(DeviceManufacturer, std::string);
      itkSetMacro(DeviceManufacturer, std::string);
      itkGetMacro(DeviceModel, std::string);
      itkSetMacro(DeviceModel, std::string);
      itkGetMacro(DeviceComment, std::string);
      itkSetMacro(DeviceComment, std::string);
      itkGetMacro(ProbeName,  std::string);
      itkSetMacro(ProbeName,  std::string);
      itkGetMacro(ProbeFrequency,  std::string);
      itkSetMacro(ProbeFrequency,  std::string);
      itkGetMacro(Zoom,  std::string);
      itkSetMacro(Zoom,  std::string);
      itkGetMacro(IsVideoOnly, bool);
      itkSetMacro(IsVideoOnly, bool);

    protected:
      USImageMetadata();
      virtual ~USImageMetadata();

      std::string m_DeviceManufacturer;
      std::string m_DeviceModel;
      std::string m_DeviceComment;
      std::string m_ProbeName;
      std::string m_ProbeFrequency;
      std::string m_Zoom;
      bool m_IsVideoOnly;

    

    };
} // namespace mitk
#endif