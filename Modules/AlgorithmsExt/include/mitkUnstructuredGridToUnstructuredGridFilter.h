/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITKUNSTRUCTUREDGRIDTOUNSTRUCTUREDGRID_h__
#define _MITKUNSTRUCTUREDGRIDTOUNSTRUCTUREDGRID_h__

#include <MitkAlgorithmsExtExports.h>

#include <mitkCommon.h>

#include <mitkImage.h>
#include <mitkUnstructuredGrid.h>
#include <mitkUnstructuredGridSource.h>

namespace mitk
{
  class MITKALGORITHMSEXT_EXPORT UnstructuredGridToUnstructuredGridFilter : public UnstructuredGridSource
  {
  public:
    mitkClassMacro(UnstructuredGridToUnstructuredGridFilter, UnstructuredGridSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Initializes the output information */
      void GenerateOutputInformation() override;

    /** Returns a const reference to the input image */
    const mitk::UnstructuredGrid *GetInput(void);

    virtual const mitk::UnstructuredGrid *GetInput(unsigned int idx);

    /** Set the source grid. As input every mitk unstructured grid can be used. */
    using itk::ProcessObject::SetInput;
    virtual void SetInput(const UnstructuredGrid *grid);

    virtual void SetInput(unsigned int idx, const UnstructuredGrid *grid);

    virtual void CreateOutputsForAllInputs(unsigned int idx);

  protected:
    /** Constructor */
    UnstructuredGridToUnstructuredGridFilter();

    /** Destructor */
    ~UnstructuredGridToUnstructuredGridFilter() override;

  private:
    /** The output of the filter */
    mitk::UnstructuredGrid::Pointer m_UnstructGrid;
  };

} // namespace mitk

#endif //_MITKUNSTRUCTUREDGRIDTOUNSTRUCTUREDGRID_h__
