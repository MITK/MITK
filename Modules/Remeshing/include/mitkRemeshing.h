/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRemeshing_h
#define mitkRemeshing_h

#include <mitkSurface.h>
#include <mitkSurfaceToSurfaceFilter.h>
#include <mitkTimeGeometry.h>
#include <MitkRemeshingExports.h>

namespace mitk
{
  class ProgressTask;

  /** \brief Remesh a surface and store the result in a new surface.
   *
   * The %ACVD library is used for remeshing which is based on the paper "Approximated Centroidal Voronoi Diagrams for
   * Uniform Polygonal Mesh Coarsening" by S. Valette, and J. M. Chassery.
   * There are a few rules of thumbs regarding the ranges of parameters to gain high quality remeshed surfaces:
   *
   * <ul>
   *  <li> numVertices is exact, however, if boundaryFixing is enabled, additional vertices are generated at
   * boundaries
   *  <li> %Set gradation to zero in case you want polygons of roughly the same size all over the remeshed surface;
   * start with 0.5 otherwise, as greater values turn a growing share of the polygons into slivers
   *  <li> subsampling has direct influence on the quality of the remeshed surface (higher values take more time)
   *  <li> edgeSplitting is useful for surfaces that contain long and thin triangles but takes a long time
   *  <li> Leave optimizationLevel set to 1 as greater values result in degenerated polygons
   *  <li> Irregular shrinking of boundaries during remeshing can be avoided by boundaryFixing, however this results
   * in additional, lower quality polygons at boundaries
   * </ul>
   *
   * \param[in] surface Input surface.
   * \param[in] t Time step of a four-dimensional input surface, zero otherwise.
   * \param[in] numVertices Desired number of vertices in the remeshed surface, set to zero to keep original vertex
   * count.
   * \param[in] gradation Influence of surface curvature on polygon size.
   * \param[in] subsampling Subsample input surface until number of vertices exceeds initial count times this
   * parameter.
   * \param[in] edgeSplitting Recursively split edges that are longer than the average edge length times this
   * parameter.
   * \param[in] optimizationLevel Minimize distance between input surface and remeshed surface.
   * \param[in] forceManifold
   * \param[in] boundaryFixing Keep original surface boundaries by adding additional polygons.
   * \param[in] progressTask Polled for a cancel request, or nullptr. Remeshing reports no steps of its own, so an
   * indeterminate task is the one to pass.
   * \return The remeshed surface.
   * \throw mitk::Exception The input surface is missing or has no polygons at time step \p t.
   * \throw itk::ProcessAborted A cancel was requested through \p progressTask, also before remeshing started.
   */
  MITKREMESHING_EXPORT Surface::Pointer Remesh(const Surface* surface,
                                               TimeStepType t,
                                               int numVertices,
                                               double gradation,
                                               int subsampling = 10,
                                               double edgeSplitting = 0.0,
                                               int optimizationLevel = 1,
                                               bool forceManifold = false,
                                               bool boundaryFixing = false,
                                               ProgressTask* progressTask = nullptr);

  /** \brief Estimate how much memory Remesh() needs at its peak.
   *
   * The memory grows with the number of vertices ACVD clusters, which is the number of input vertices after
   * subdividing the input until it has at least \p subsampling times \p numVertices of them. A high subsampling
   * at a high density therefore needs a multiple of what the input itself takes.
   *
   * The estimate includes one copy of the input surface at time step \p t, so it is what remeshing needs on top of a
   * surface that is already in memory when Remesh() gets a copy of it, as it should if the original stays on display
   * while Remesh() runs on another thread. It leaves out the vertices added by edge splitting and boundary fixing.
   *
   * \param[in] surface Input surface.
   * \param[in] t Time step of a four-dimensional input surface, zero otherwise.
   * \param[in] numVertices Desired number of vertices in the remeshed surface, zero for the original vertex count.
   * \param[in] subsampling Subsampling as passed to Remesh().
   * \return The estimated peak memory in bytes.
   * \throw mitk::Exception The input surface is missing or has no polygons at time step \p t.
   */
  MITKREMESHING_EXPORT size_t EstimateRemeshingMemory(const Surface* surface,
                                                      TimeStepType t,
                                                      int numVertices,
                                                      int subsampling = 10);

  /**
   * \brief ITK/VTK-style filter that remeshes every time step of a surface with mitk::Remesh().
   *
   * Each time step with polygons is remeshed to the given density of its own vertex count, but to at least the
   * minimum number of vertices. Time steps without polygons stay empty. The output keeps the time geometry of the
   * input, so its time steps and their geometries match those of the input.
   *
   * Default parameter values:
   * - Density: 1.0 (keep the vertex count of each time step)
   * - MinNumVertices: 100
   * - Gradation: 0.5
   * - Subsampling: 10
   * - EdgeSplitting: 0.0 (disabled)
   * - OptimizationLevel: 1
   * - ForceManifold: false
   * - BoundaryFixing: false
   *
   * \sa Remesh()
   * \sa SurfaceToSurfaceFilter
   */
  class MITKREMESHING_EXPORT RemeshFilter : public mitk::SurfaceToSurfaceFilter
  {
  public:
    mitkClassMacro(RemeshFilter, SurfaceToSurfaceFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Set the vertex count of each remeshed time step relative to that of the input time step. */
    itkSetMacro(Density, double);
    /** \brief Set the lower bound of the vertex count of each remeshed time step. */
    itkSetMacro(MinNumVertices, int);
    /** \brief Set the gradation parameter controlling curvature influence on polygon size. */
    itkSetMacro(Gradation, double);
    /** \brief Set the subsampling factor for input surface refinement before remeshing. */
    itkSetMacro(Subsampling, int);
    /** \brief Set the edge splitting threshold (0.0 to disable). */
    itkSetMacro(EdgeSplitting, double);
    /** \brief Set the optimization level for minimizing surface distance (0 to disable). */
    itkSetMacro(OptimizationLevel, int);
    /** \brief Set whether to force the output surface to be manifold. */
    itkSetMacro(ForceManifold, bool);
    /** \brief Set whether to fix boundaries by adding extra polygons. */
    itkSetMacro(BoundaryFixing, bool);

    /**
     * \brief Poll the given task for a cancel request, or nothing if it is nullptr.
     *
     * A requested cancel makes Update() throw itk::ProcessAborted. With several time steps to remesh, the task gets
     * one step per time step, which is reported once the time step is remeshed.
     *
     * \sa Remesh()
     */
    void SetProgressTask(ProgressTask* task);
    ProgressTask* GetProgressTask() const;

    /**
     * \brief Estimate how much memory Update() needs at its peak.
     *
     * The estimate of EstimateRemeshingMemory() for the most demanding time step, plus the other time steps of the
     * input, which should be a copy if the original stays on display while the filter runs on another thread. It
     * leaves out the time steps that are already remeshed.
     *
     * \return The estimated peak memory in bytes.
     * \throw mitk::Exception The input surface is missing or has no polygons at any time step.
     */
    size_t EstimateMemory() const;

  protected:
    void GenerateData() override;

    /**
     * \brief Unlike other surface filters, does not copy the input into the output.
     *
     * GenerateData() replaces the output anyway, and a copy of every time step
     * would double the memory of the input on each SetInput().
     */
    void CreateOutputForInput(unsigned int idx) override;

  private:
    RemeshFilter();
    ~RemeshFilter() override;

    double m_Density;
    int m_MinNumVertices;
    double m_Gradation;
    int m_Subsampling;
    double m_EdgeSplitting;
    int m_OptimizationLevel;
    bool m_ForceManifold;
    bool m_BoundaryFixing;
    ProgressTask* m_ProgressTask;
  };
}

#endif
