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

#ifndef _MITKUNSTRUCTUREDGRIDCLUSTERINGFILTER_h__
#define _MITKUNSTRUCTUREDGRIDCLUSTERINGFILTER_h__

#include <MitkAlgorithmsExtExports.h>

#include <mitkCommon.h>

#include <mitkUnstructuredGrid.h>
#include <mitkUnstructuredGridToUnstructuredGridFilter.h>


namespace mitk {

  class MitkAlgorithmsExt_EXPORT UnstructuredGridClusteringFilter
      : public UnstructuredGridToUnstructuredGridFilter
  {
    public:

      mitkClassMacro(UnstructuredGridClusteringFilter, UnstructuredGridToUnstructuredGridFilter)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      virtual void GenerateOutputInformation();

    protected:

      UnstructuredGridClusteringFilter();

      virtual ~UnstructuredGridClusteringFilter();

  private:

      mitk::UnstructuredGrid::Pointer m_UnstructGrid;

  };

} // namespace mitk

#endif //_MITKUNSTRUCTUREDGRIDCLUSTERINGFILTER_h__


