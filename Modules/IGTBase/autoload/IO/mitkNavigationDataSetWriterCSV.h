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

#include <mitkNavigationDataSet.h>
#include <mitkAbstractFileWriter.h>

namespace mitk {
  class NavigationDataSetWriterCSV : public AbstractFileWriter
  {
  public:
    NavigationDataSetWriterCSV();
    virtual~NavigationDataSetWriterCSV();

    using AbstractFileWriter::Write;
    virtual void Write() override;

  protected:
    NavigationDataSetWriterCSV(const NavigationDataSetWriterCSV& other);

    virtual mitk::NavigationDataSetWriterCSV* Clone() const override;
  };
}

#endif // MITKNavigationDataSetWriterCSV_H_HEADER_INCLUDED_
