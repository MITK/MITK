/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
