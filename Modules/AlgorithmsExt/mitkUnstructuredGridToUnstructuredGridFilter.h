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

#ifndef _MITKUNSTRUCTUREDGRIDTOUNSTRUCTUREDGRID_h__
#define _MITKUNSTRUCTUREDGRIDTOUNSTRUCTUREDGRID_h__

#include <MitkAlgorithmsExtExports.h>

#include <mitkCommon.h>

#include <mitkUnstructuredGrid.h>
#include <mitkUnstructuredGridSource.h>
#include <mitkImage.h>


namespace mitk {

  class MitkAlgorithmsExt_EXPORT UnstructuredGridToUnstructuredGridFilter
      : public UnstructuredGridSource
  {
    public:

      mitkClassMacro(UnstructuredGridToUnstructuredGridFilter, UnstructuredGridSource)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      /** Initializes the output information */
      virtual void GenerateOutputInformation();

      /** Returns a const reference to the input image */
      const mitk::UnstructuredGrid* GetInput(void);

      /** Set the source grid. As input every mitk unstructured grid can be used. */
      using itk::ProcessObject::SetInput;
      virtual void SetInput(const UnstructuredGrid *image);


    protected:

      /** Constructor */
      UnstructuredGridToUnstructuredGridFilter();

      /** Destructor */
      virtual ~UnstructuredGridToUnstructuredGridFilter();


  private:

      /** The output of the filter */
      mitk::UnstructuredGrid::Pointer m_UnstructGrid;

  };

} // namespace mitk

#endif //_MITKUNSTRUCTUREDGRIDTOUNSTRUCTUREDGRID_h__


