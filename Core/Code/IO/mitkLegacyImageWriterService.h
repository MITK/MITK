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

// MITK
#include <mitkLegacyFileWriterService.h>

namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
class LegacyImageWriterService : public mitk::LegacyFileWriterService
{
public:

  LegacyImageWriterService(const std::string& basedataType, const std::string& extension, const std::string& description);

private:

  LegacyImageWriterService* Clone() const;
};

} // namespace mitk


#endif /* LegacyImageWriterService_H_HEADER_INCLUDED_C1E7E521 */
