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

#ifndef __mitkFiberBundleXReader_h
#define __mitkFiberBundleXReader_h

#include <mitkCommon.h>
#include <mitkFileReader.h>
#include <mitkFiberBundleX.h>
#include <vtkSmartPointer.h>

#include <mitkAbstractFileReader.h>

namespace mitk
{

  /** \brief
  */

  class FiberBundleXReader : public AbstractFileReader
  {
  public:

    FiberBundleXReader();
    virtual ~FiberBundleXReader(){}
    FiberBundleXReader(const FiberBundleXReader& other);
    virtual FiberBundleXReader * Clone() const;

    using mitk::AbstractFileReader::Read;
    virtual std::vector<itk::SmartPointer<BaseData> > Read();

  private:

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK

#endif // __mitkFiberBundleXReader_h
