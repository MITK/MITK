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
   * @brief The AnnotationUtils class provides static functions for accsessing registered AnnotationRenderers and
   * Annotations
   */
  class MITKCORE_EXPORT AnnotationUtils
  {
  public:
    typedef std::vector<std::unique_ptr<AbstractAnnotationRenderer>> AnnotationRendererServices;
    AnnotationUtils();
    ~AnnotationUtils();

    /**
     * @brief GetAnnotationRenderer returns a registered AnnotationRenderer of a specific type and for a BaseRenderer
     * @param arTypeID name specifier of the AnnotationRenderer
     * @param rendererID name specifier of the BaseRenderer
     * @return
     */
    static AbstractAnnotationRenderer *GetAnnotationRenderer(const std::string &arTypeID,
                                                             const std::string &rendererID);

    /**
     * @brief RegisterAnnotationRenderer registers an AnnotationRenderer as a microservice and saves a reference to it
     * in a local static list.
     * @param annotationRenderer
     */
    static void RegisterAnnotationRenderer(AbstractAnnotationRenderer *annotationRenderer);

    /**
     * @brief GetAnnotationRenderer returns a list of registered AnnotationRenderers for a specified BaseRenderer
     * @param rendererID name specifier of the BaseRenderer
     * @return
     */
    static std::vector<AbstractAnnotationRenderer *> GetAnnotationRenderer(const std::string &rendererID);

    /**
     * @brief UpdateAnnotationRenderer is a convenience function which calls AbstractAnnotationRenderer::Update for each
     * registered AnnotationRenderer of a specific BaseRenderer.
     * @param rendererID
     */
    static void UpdateAnnotationRenderer(const std::string &rendererID);

    /**
     * @brief BaseRendererChanged has to be called in the case that the actual BaseRenderer object for a BaseRenderer ID
     * has changed. E.g. if a RenderWindow was closed and reopened.
     * @param renderer The new BaseRenderer
     */
    static void BaseRendererChanged(BaseRenderer *renderer);

    /**
     * @brief GetAnnotation returns a registered Annotation for a specified ID.
     * @param AnnotationID
     * @return
     */
    static mitk::Annotation *GetAnnotation(const std::string &AnnotationID);

  private:
    AnnotationUtils(const AnnotationUtils &);
    AnnotationUtils &operator=(const AnnotationUtils &);

    static void RenderWindowCallback(vtkObject *caller, unsigned long, void *, void *);
  };
}

#endif
