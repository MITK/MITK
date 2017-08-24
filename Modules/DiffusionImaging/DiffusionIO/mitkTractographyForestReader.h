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

#ifndef __mitkTractographyForestReader_h
#define __mitkTractographyForestReader_h

#include <mitkCommon.h>
#include <mitkFileReader.h>
#include <mitkAbstractFileReader.h>
#include <mitkTractographyForest.h>

namespace mitk
{

  /** \brief
  */

  class TractographyForestReader : public AbstractFileReader
  {
  public:

    TractographyForestReader();
    virtual ~TractographyForestReader(){}
    TractographyForestReader(const TractographyForestReader& other);
    virtual TractographyForestReader * Clone() const override;

    using mitk::AbstractFileReader::Read;
    virtual std::vector<itk::SmartPointer<BaseData> > Read() override;

  private:

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK

#endif // __mitkFiberBundleReader_h
