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

#ifndef __mitkFiberBundleTckReader_h
#define __mitkFiberBundleTckReader_h

#include <mitkCommon.h>
#include <mitkFileReader.h>
#include <mitkFiberBundle.h>
#include <vtkSmartPointer.h>

#include <mitkAbstractFileReader.h>

namespace mitk
{

  /** \brief
  */

  class FiberBundleTckReader : public AbstractFileReader
  {
  public:

    FiberBundleTckReader();
    ~FiberBundleTckReader() override{}
    FiberBundleTckReader(const FiberBundleTckReader& other);
    FiberBundleTckReader * Clone() const override;

    using mitk::AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData> > Read() override;

  private:

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK

#endif // __mitkFiberBundleReader_h
