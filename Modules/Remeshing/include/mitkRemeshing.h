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
   * start with 1 otherwise
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
   * \return Returns the remeshed surface or nullptr if input surface is invalid.
   */
  MITKREMESHING_EXPORT Surface::Pointer Remesh(const Surface* surface,
                                               TimeStepType t,
                                               int numVertices,
                                               double gradation,
                                               int subsampling = 10,
                                               double edgeSplitting = 0.0,
                                               int optimizationLevel = 1,
                                               bool forceManifold = false,
                                               bool boundaryFixing = false);

  /**
   * \brief ITK/VTK-style filter that encapsulates the mitk::Remesh() function.
   *
   * This filter wraps the free-standing Remesh() function into an ITK pipeline filter
   * derived from SurfaceToSurfaceFilter. Set the desired remeshing parameters via the
   * provided setter macros, assign an input surface, and call Update() to produce
   * a remeshed output surface.
   *
   * Default parameter values:
   * - TimeStep: 0
   * - NumVertices: 0 (keep original vertex count)
   * - Gradation: 1.0
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

    /** \brief Set the time step of the input surface to remesh. */
    itkSetMacro(TimeStep, unsigned int);
    /** \brief Set the desired number of vertices (0 to keep original count). */
    itkSetMacro(NumVertices, int);
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

  protected:
    void GenerateData() override;

  private:
    RemeshFilter();
    ~RemeshFilter() override;

    TimeStepType m_TimeStep;
    int m_NumVertices;
    double m_Gradation;
    int m_Subsampling;
    double m_EdgeSplitting;
    int m_OptimizationLevel;
    bool m_ForceManifold;
    bool m_BoundaryFixing;
  };
}

#endif
