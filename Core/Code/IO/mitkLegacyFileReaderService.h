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


#ifndef LegacyFileReaderService_H_HEADER_INCLUDED_C1E7E521
#define LegacyFileReaderService_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkExports.h>
#include <mitkCommon.h>

// MITK
#include <mitkBaseData.h>
#include <mitkAbstractFileReader.h>
#include <mitkFileReader.h>

// ITK
#include <itkObjectFactory.h>

// Microservices
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
  class MITK_CORE_EXPORT LegacyFileReaderService : public itk::LightObject, public mitk::AbstractFileReader
{
  public:

    mitkClassMacro( LegacyFileReaderService, AbstractFileReader);

    mitkNewMacro1Param( Self, std::string );

    virtual itk::SmartPointer<BaseData> Read(const std::string& path = 0);

    virtual itk::SmartPointer<BaseData> Read(const std::istream& stream );


protected:
    LegacyFileReaderService(std::string extension);
    virtual ~LegacyFileReaderService();

};
} // namespace mitk


#endif /* LegacyFileReaderService_H_HEADER_INCLUDED_C1E7E521 */
