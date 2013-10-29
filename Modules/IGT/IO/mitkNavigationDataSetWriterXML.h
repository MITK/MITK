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

#include <mitkNavigationDataSet.h>

namespace mitk {
  class MitkIGT_EXPORT NavigationDataSetWriterXML
  {
  public:

    NavigationDataSetWriterXML();
    ~NavigationDataSetWriterXML();

    virtual void Write (std::string path, mitk::NavigationDataSet::Pointer );
    virtual void Write (std::ostream* stream, mitk::NavigationDataSet::Pointer);

  protected:

    virtual void StreamHeader (std::ostream* stream, mitk::NavigationDataSet::Pointer data);
    virtual void StreamData (std::ostream* stream, mitk::NavigationDataSet::Pointer data);
    virtual void StreamFooter (std::ostream* stream);
  };
}

#endif // MITKNavigationDataSetWriterXML_H_HEADER_INCLUDED_
