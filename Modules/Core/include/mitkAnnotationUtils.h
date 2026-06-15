/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAnnotationUtils_h
#define mitkAnnotationUtils_h

#include <MitkCoreExports.h>
#include <memory>
#include <string>
#include <vector>

class vtkObject;

namespace mitk
{
  class AbstractAnnotationRenderer;
  class Annotation;
  class BaseRenderer;

  /**
   * \brief Provides static utility functions for accessing registered AnnotationRenderers
   *        and Annotations.
   */
  class MITKCORE_EXPORT AnnotationUtils
  {
  public:
    /** \brief Container type for managed AnnotationRenderer instances. */
    typedef std::vector<std::unique_ptr<AbstractAnnotationRenderer>> AnnotationRendererServices;

    /** \brief Constructor. */
    AnnotationUtils();

    /** \brief Destructor. */
    ~AnnotationUtils();

    /**
     * \brief Get a registered AnnotationRenderer of a specific type for a given BaseRenderer.
     *
     * \param[in] arTypeID    Name specifier of the AnnotationRenderer type.
     * \param[in] rendererID  Name specifier of the BaseRenderer.
     * \return The matching AnnotationRenderer, or nullptr if not found.
     */
    static AbstractAnnotationRenderer *GetAnnotationRenderer(const std::string &arTypeID,
                                                             const std::string &rendererID);

    /**
     * \brief Register an AnnotationRenderer as a microservice.
     *
     * Saves a reference to it in a local static list.
     *
     * \param[in] annotationRenderer  The AnnotationRenderer to register.
     */
    static void RegisterAnnotationRenderer(AbstractAnnotationRenderer *annotationRenderer);

    /**
     * \brief Get all registered AnnotationRenderers for a specified BaseRenderer.
     *
     * \param[in] rendererID  Name specifier of the BaseRenderer.
     * \return A vector of matching AnnotationRenderer pointers.
     */
    static std::vector<AbstractAnnotationRenderer *> GetAnnotationRenderer(const std::string &rendererID);

    /**
     * \brief Call AbstractAnnotationRenderer::Update for each registered AnnotationRenderer
     *        of the specified BaseRenderer.
     *
     * \param[in] rendererID  Name specifier of the BaseRenderer.
     */
    static void UpdateAnnotationRenderer(const std::string &rendererID);

    /**
     * \brief Notify that the actual BaseRenderer object for a BaseRenderer ID has changed.
     *
     * This must be called when a RenderWindow is closed and reopened, so that
     * AnnotationRenderers can update their reference.
     *
     * \param[in] renderer  The new BaseRenderer instance.
     */
    static void BaseRendererChanged(BaseRenderer *renderer);

    /**
     * \brief Get a registered Annotation by its unique ID.
     *
     * \param[in] AnnotationID  The unique identifier of the Annotation.
     * \return The matching Annotation, or nullptr if not found.
     */
    static mitk::Annotation *GetAnnotation(const std::string &AnnotationID);

  private:
    AnnotationUtils(const AnnotationUtils &);
    AnnotationUtils &operator=(const AnnotationUtils &);

    /** \brief VTK callback registered on render windows for annotation updates. */
    static void RenderWindowCallback(vtkObject *caller, unsigned long, void *, void *);
  };
}

#endif
