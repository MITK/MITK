/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelSubDivisionFilter_h
#define mitkContourModelSubDivisionFilter_h

#include <mitkCommon.h>
#include <mitkContourModel.h>
#include <mitkContourModelSource.h>
#include <MitkContourModelExports.h>

namespace mitk
{
  /** \brief Filter that interpolates a subdivision curve between control points of a contour.
   *
   * Uses the Dyn-Levin-Gregory (DLG) four-point interpolation scheme to insert
   * new vertices between existing control points:
   * \code
   * F(2i)   = C(i)
   * F(2i+1) = -1/16 * C(i-1) + 9/16 * C(i) + 9/16 * C(i+1) - 1/16 * C(i+2)
   * \endcode
   *
   * The number of subdivision iterations defaults to 4 and can be adjusted
   * via SetNumberOfIterations(). At least 4 input vertices are required for
   * the filter to produce output; otherwise the input is passed through unchanged.
   *
   * \sa ContourModel, ContourModelSource, ContourModelMapper2D
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelSubDivisionFilter : public ContourModelSource
  {
  public:
    mitkClassMacro(ContourModelSubDivisionFilter, ContourModelSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief The output data type. */
      typedef ContourModel OutputType;
    /** \brief Smart pointer type for the output. */
    typedef OutputType::Pointer OutputTypePointer;
    /** \brief The input data type. */
    typedef mitk::ContourModel InputType;

    /** \brief Set the number of subdivision iterations.
     *
     * Each iteration doubles the number of vertices between control points.
     * Default is 4.
     *
     * \param[in] iterations Number of subdivision iterations to perform.
     */
    void SetNumberOfIterations(int iterations) { this->m_InterpolationIterations = iterations; }
    using Superclass::SetInput;

    /** \brief Set the input contour model.
     * \param[in] input The contour model to subdivide.
     */
    virtual void SetInput(const InputType *input);

    /** \brief Set the input contour model at the given index.
     * \param[in] idx Input port index.
     * \param[in] input The contour model to subdivide.
     */
    virtual void SetInput(unsigned int idx, const InputType *input);

    /** \brief Get the first input contour model.
     * \return Const pointer to the input ContourModel, or nullptr if none is set.
     */
    const InputType *GetInput(void);

    /** \brief Get the input contour model at the given index.
     * \param[in] idx Input port index.
     * \return Const pointer to the input ContourModel, or nullptr if none is set.
     */
    const InputType *GetInput(unsigned int idx);

  protected:
    ContourModelSubDivisionFilter();

    ~ContourModelSubDivisionFilter() override;

    void GenerateOutputInformation() override{};

    void GenerateData() override;

    int m_InterpolationIterations;
  };
}

#endif
