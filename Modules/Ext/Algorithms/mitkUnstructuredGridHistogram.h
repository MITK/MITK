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

#ifndef UNSTRUCTURED_GIRD_HISTOGRAM_H_HEADER_INCLUDED
#define UNSTRUCTURED_GIRD_HISTOGRAM_H_HEADER_INCLUDED

#include <itkHistogram.h>
#include "MitkExtExports.h"
#include "mitkUnstructuredGrid.h"

namespace mitk {

//##Documentation
//## @brief Subclass of itk::Histogram, specialized for a mitk::UnstructuredGrid
//##
class MitkExt_EXPORT UnstructuredGridHistogram : public itk::Statistics::Histogram<double>
{
public:
  mitkClassMacro(UnstructuredGridHistogram,itk::Statistics::Histogram<double>)

  itkNewMacro(Self)

  void Initialize(mitk::UnstructuredGrid*);

  void SetUsePointData() { m_UsePointData = true; }
  void SetUseCellData() { m_UsePointData = false; }

protected:
  UnstructuredGridHistogram() : m_UsePointData(true) {}

  virtual ~UnstructuredGridHistogram();

  bool m_UsePointData;
};

} // namespace mitk

#endif /* UNSTRUCTURED_GIRD_HISTOGRAM_H_HEADER_INCLUDED */
