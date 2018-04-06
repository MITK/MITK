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

#ifndef mitkUSDeviceWriterXML_H_Header_INCLUDED_
#define mitkUSDeviceWriterXML_H_Header_INCLUDED_

#include <MitkUSExports.h>

#include <mitkAbstractFileWriter.h>

namespace mitk {
  class MITKUS_EXPORT USDeviceWriterXML : public AbstractFileWriter
  {
  public:

    USDeviceWriterXML();
    ~USDeviceWriterXML() override;

    using AbstractFileWriter::Write;
    void Write() override;

    void SetFilename(std::string filename);

  protected:
    USDeviceWriterXML(const USDeviceWriterXML& other);
    mitk::USDeviceWriterXML* Clone() const override;

  private:
    std::string m_Filename;


  };
}

#endif // mitkUSDeviceWriterXML_H_Header_INCLUDED_
