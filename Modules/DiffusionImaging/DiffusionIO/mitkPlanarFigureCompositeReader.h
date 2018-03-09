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

#ifndef __mitkPlanarFigureCompositeReader_h
#define __mitkPlanarFigureCompositeReader_h

#include <mitkCommon.h>
#include <mitkFileReader.h>
#include <mitkFiberBundle.h>
#include <vtkSmartPointer.h>

#include <mitkAbstractFileReader.h>

namespace mitk
{

  /** \brief
  */

  class PlanarFigureCompositeReader : public AbstractFileReader
  {
  public:

    PlanarFigureCompositeReader();
    ~PlanarFigureCompositeReader() override{}
    PlanarFigureCompositeReader(const PlanarFigureCompositeReader& other);
    PlanarFigureCompositeReader * Clone() const override;

    using mitk::AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData> > Read() override;

  private:

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK

#endif // __mitkFiberBundleReader_h
