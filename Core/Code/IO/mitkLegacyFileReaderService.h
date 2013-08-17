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

// MITK
#include <mitkBaseData.h>
#include <mitkAbstractFileReader.h>


namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
class LegacyFileReaderService : public mitk::AbstractFileReader
{

public:

  LegacyFileReaderService(const std::string& extension, const std::string& description);
  virtual ~LegacyFileReaderService();

  virtual std::list< mitk::BaseData::Pointer > Read(const std::string& path, mitk::DataStorage* ds = 0);

  virtual std::list< mitk::BaseData::Pointer > Read(const std::istream& stream, mitk::DataStorage* ds = 0 );

};

} // namespace mitk

#endif /* LegacyFileReaderService_H_HEADER_INCLUDED_C1E7E521 */
