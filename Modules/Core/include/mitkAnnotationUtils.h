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
#include <vector>

class vtkObject;

namespace mitk
{
  class AbstractAnnotationRenderer;
  class Annotation;
  class BaseRenderer;

  class MITKCORE_EXPORT AnnotationUtils
  {
  public:
    typedef std::vector<std::unique_ptr<AbstractAnnotationRenderer>> AnnotationRendererServices;
    AnnotationUtils();
    ~AnnotationUtils();

    static AbstractAnnotationRenderer *GetAnnotationRenderer(const std::string &arTypeID,
                                                             const std::string &rendererID);

    static void RegisterAnnotationRenderer(AbstractAnnotationRenderer *annotationRenderer);

    static std::vector<AbstractAnnotationRenderer *> GetAnnotationRenderer(const std::string &rendererID);

    static void UpdateAnnotationRenderer(const std::string &rendererID);

    static void BaseRendererChanged(BaseRenderer *renderer);

    static mitk::Annotation *GetAnnotation(const std::string &AnnotationID);

  private:
    AnnotationUtils(const AnnotationUtils &);
    AnnotationUtils &operator=(const AnnotationUtils &);

    static void RenderWindowCallback(vtkObject *caller, unsigned long, void *, void *);
  };
}

#endif
