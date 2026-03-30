/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelToSurfaceFilter_h
#define mitkContourModelToSurfaceFilter_h

#include <mitkCommon.h>
#include <MitkContourModelExports.h>

#include <mitkContourModel.h>
#include <mitkContourModelSource.h>

#include <mitkSurfaceSource.h>

namespace mitk
{
  /** \brief Filter that converts a mitk::ContourModel into a mitk::Surface.
   *
   * Creates VTK polydata (polygons and line segments) from the contour vertices
   * for each time step. A polygon is created from all vertices, and line segments
   * connect consecutive vertices. If the contour is closed, an additional line
   * segment connects the last vertex back to the first.
   *
   * \note The input contour must have at least 3 vertices for polygon generation.
   *       For contours with fewer vertices, an empty polydata is produced.
   *
   * \sa ContourModel, Surface, SurfaceSource
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelToSurfaceFilter : public SurfaceSource
  {
  public:
    mitkClassMacro(ContourModelToSurfaceFilter, SurfaceSource);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief The output data type. */
      typedef mitk::Surface OutputType;

    /** \brief The input data type. */
    typedef mitk::ContourModel InputType;

    /** \brief Generate output information (no-op for this filter). */
    void GenerateOutputInformation() override;

    using Superclass::SetInput;

    /** \brief Set the input contour model.
     * \param[in] input The ContourModel to convert.
     */
    virtual void SetInput(const InputType *input);

    /** \brief Set the input contour model at the given index.
     * \param[in] idx Input port index.
     * \param[in] input The ContourModel to convert.
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
    ContourModelToSurfaceFilter();
    ~ContourModelToSurfaceFilter() override;

    void GenerateData() override;
  };
}
#endif
