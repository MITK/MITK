/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMAPRegistrationWrapperIO_h
#define mitkMAPRegistrationWrapperIO_h

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

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------
    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    MAPRegistrationWrapperIO(const MAPRegistrationWrapperIO& other);
    MAPRegistrationWrapperIO* IOClone() const override;
  };


} // end of namespace mitk

#endif
