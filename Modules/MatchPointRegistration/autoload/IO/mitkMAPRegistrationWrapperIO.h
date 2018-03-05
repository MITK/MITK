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

#ifndef _MITK_MAP_REGISTRATION_WRAPPER_IO_H
#define _MITK_MAP_REGISTRATION_WRAPPER_IO_H

#include <mitkAbstractFileIO.h>

#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{
  /**
  * Offers IO capability for MatchPoint registration wrappers
  */
  class MAPRegistrationWrapperIO : public AbstractFileIO
  {
  public:

    MAPRegistrationWrapperIO();

    // -------------- AbstractFileReader -------------
    using AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData> > Read() override;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------
    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  private:
    MAPRegistrationWrapperIO(const MAPRegistrationWrapperIO& other);
    MAPRegistrationWrapperIO* IOClone() const override;
  };


} // end of namespace mitk

#endif
