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

#ifndef __mitkNrrdOdfImageReader_h
#define __mitkNrrdOdfImageReader_h

#include "mitkCommon.h"
#include "itkVectorContainer.h"
#include "vnl/vnl_vector_fixed.h"
#include "mitkOdfImage.h"
#include "itkVectorImage.h"
#include <mitkAbstractFileReader.h>
#include <mitkBaseData.h>
#include <mitkMimeType.h>

namespace mitk
{

  /** \brief
  */

  class NrrdOdfImageReader : public mitk::AbstractFileReader
  {
  public:

    typedef mitk::OdfImage OutputType;

    NrrdOdfImageReader(const NrrdOdfImageReader& other);
    NrrdOdfImageReader();
    ~NrrdOdfImageReader() override;

    using AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData> > Read() override;

  protected:


  private:
  private:
    NrrdOdfImageReader* Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK

#endif // __mitkNrrdOdfImageReader_h
