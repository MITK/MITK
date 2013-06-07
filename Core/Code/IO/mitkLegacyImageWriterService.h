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


#ifndef LegacyImageWriterService_H_HEADER_INCLUDED_C1E7E521
#define LegacyImageWriterService_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkExports.h>
#include <mitkCommon.h>

// MITK
#include <mitkBaseData.h>
#include <mitkAbstractFileWriter.h>
#include <mitkFileWriter.h>
#include <mitkLegacyFileWriterService.h>

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
  class MITK_CORE_EXPORT LegacyImageWriterService : public mitk::LegacyFileWriterService
{
  public:

    mitkClassMacro( LegacyImageWriterService, LegacyFileWriterService);

    mitkNewMacro3Param( Self, std::string, std::string, std::string);

    virtual void Write(const itk::SmartPointer<BaseData> data, const std::string& path = 0);

protected:
    LegacyImageWriterService(std::string basedataType, std::string extension, std::string description);
    virtual ~LegacyImageWriterService();

};
} // namespace mitk


#endif /* LegacyImageWriterService_H_HEADER_INCLUDED_C1E7E521 */
