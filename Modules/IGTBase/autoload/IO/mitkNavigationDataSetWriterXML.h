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

#ifndef MITKNavigationDataSetWriterXML_H_HEADER_INCLUDED_
#define MITKNavigationDataSetWriterXML_H_HEADER_INCLUDED_

#include <MitkIGTIOExports.h>

#include <mitkNavigationDataSet.h>
#include <mitkAbstractFileWriter.h>

namespace mitk {
  class MITKIGTIO_EXPORT NavigationDataSetWriterXML : public AbstractFileWriter
  {
  public:

    NavigationDataSetWriterXML();
    ~NavigationDataSetWriterXML() override;


    using AbstractFileWriter::Write;
    void Write() override;

  protected:

    NavigationDataSetWriterXML(const NavigationDataSetWriterXML& other);

    mitk::NavigationDataSetWriterXML* Clone() const override;

    virtual void StreamHeader (std::ostream* stream, mitk::NavigationDataSet::ConstPointer data);
    virtual void StreamData (std::ostream* stream, mitk::NavigationDataSet::ConstPointer data);
    virtual void StreamFooter (std::ostream* stream);
  };
}

#endif // MITKNavigationDataSetWriterXML_H_HEADER_INCLUDED_
