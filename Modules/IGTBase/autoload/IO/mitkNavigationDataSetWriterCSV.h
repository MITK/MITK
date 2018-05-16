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


#ifndef MITKNavigationDataSetWriterCSV_H_HEADER_INCLUDED_
#define MITKNavigationDataSetWriterCSV_H_HEADER_INCLUDED_

#include <MitkIGTIOExports.h>

#include <mitkNavigationDataSet.h>
#include <mitkAbstractFileWriter.h>

namespace mitk {
  class MITKIGTIO_EXPORT NavigationDataSetWriterCSV : public AbstractFileWriter
  {
  public:
    NavigationDataSetWriterCSV();
    ~NavigationDataSetWriterCSV() override;

    using AbstractFileWriter::Write;
    void Write() override;

  protected:
    NavigationDataSetWriterCSV(const NavigationDataSetWriterCSV& other);

    mitk::NavigationDataSetWriterCSV* Clone() const override;
  };
}

#endif // MITKNavigationDataSetWriterCSV_H_HEADER_INCLUDED_
