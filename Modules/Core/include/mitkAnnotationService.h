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

#ifndef mitkAnnotationService_h
#define mitkAnnotationService_h

#include <MitkCoreExports.h>
#include <memory>
#include <vector>

namespace mitk
{
  class AbstractAnnotationRenderer;
  class Overlay;

  class MITKCORE_EXPORT AnnotationService
  {
  public:
    typedef std::vector<std::unique_ptr<AbstractAnnotationRenderer>> AnnotationRendererServices;
    AnnotationService();
    ~AnnotationService();

    static AbstractAnnotationRenderer *GetAnnotationRenderer(const std::string &arTypeID,
                                                             const std::string &rendererID);

    static void RegisterAnnotationRenderer(AbstractAnnotationRenderer *annotationRenderer);

    static std::vector<AbstractAnnotationRenderer *> GetAnnotationRenderer(const std::string &rendererID);

    static void UpdateAnnotationRenderer(const std::string &rendererID);

    static void BaseRendererChanged(const std::string &rendererID);

    static mitk::Overlay *GetOverlay(const std::string &overlayID);

  private:
    AnnotationService(const AnnotationService &);
    AnnotationService &operator=(const AnnotationService &);
  };
}

#endif
