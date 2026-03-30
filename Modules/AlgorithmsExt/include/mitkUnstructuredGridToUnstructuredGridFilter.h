/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUnstructuredGridToUnstructuredGridFilter_h
#define mitkUnstructuredGridToUnstructuredGridFilter_h

#include <MitkAlgorithmsExtExports.h>

#include <mitkCommon.h>

#include <mitkImage.h>
#include <mitkUnstructuredGrid.h>
#include <mitkUnstructuredGridSource.h>

namespace mitk
{
  /**
   * \brief Base class for filters that take an UnstructuredGrid as input and produce
   * an UnstructuredGrid as output.
   *
   * This filter provides the infrastructure for connecting UnstructuredGrid inputs
   * and outputs. Subclasses should override GenerateData() to implement the actual
   * filtering logic.
   *
   * \sa UnstructuredGrid
   * \sa UnstructuredGridSource
   * \sa UnstructuredGridClusteringFilter
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT UnstructuredGridToUnstructuredGridFilter : public UnstructuredGridSource
  {
  public:
    mitkClassMacro(UnstructuredGridToUnstructuredGridFilter, UnstructuredGridSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief Initializes the output information based on the input. */
      void GenerateOutputInformation() override;

    /**
     * \brief Returns the input unstructured grid.
     * \return Const pointer to the input UnstructuredGrid, or nullptr if not set.
     */
    const mitk::UnstructuredGrid *GetInput(void);

    /**
     * \brief Returns the input unstructured grid at a given index.
     * \param[in] idx Zero-based index of the input to retrieve.
     * \return Const pointer to the input UnstructuredGrid at index \p idx.
     */
    virtual const mitk::UnstructuredGrid *GetInput(unsigned int idx);

    /**
     * \brief Set the input unstructured grid.
     * \param[in] grid The unstructured grid to use as input.
     */
    using itk::ProcessObject::SetInput;
    virtual void SetInput(const UnstructuredGrid *grid);

    /**
     * \brief Set the input unstructured grid at a specified index.
     * \param[in] idx Zero-based index of the input to set.
     * \param[in] grid The unstructured grid to use as input.
     */
    virtual void SetInput(unsigned int idx, const UnstructuredGrid *grid);

    /**
     * \brief Creates output objects for all inputs starting at the given index.
     * \param[in] idx The starting index for creating outputs.
     */
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

#endif
