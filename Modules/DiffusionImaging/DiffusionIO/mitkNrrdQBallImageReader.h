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

#ifndef __mitkNrrdQBallImageReader_h
#define __mitkNrrdQBallImageReader_h

#include "mitkCommon.h"
#include "itkVectorContainer.h"
#include "vnl/vnl_vector_fixed.h"
#include "mitkQBallImage.h"
#include "itkVectorImage.h"
#include <mitkAbstractFileReader.h>
#include <mitkBaseData.h>
#include <mitkMimeType.h>

namespace mitk
{

  /** \brief
  */

  class NrrdQBallImageReader : public mitk::AbstractFileReader
  {
  public:

    typedef mitk::QBallImage OutputType;

    NrrdQBallImageReader(const NrrdQBallImageReader& other);
    NrrdQBallImageReader();
    virtual ~NrrdQBallImageReader();

    using AbstractFileReader::Read;
    virtual std::vector<itk::SmartPointer<BaseData> > Read();

  protected:


  private:
  private:
    NrrdQBallImageReader* Clone() const;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK

#endif // __mitkNrrdQBallImageReader_h
