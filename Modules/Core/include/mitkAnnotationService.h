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

#include <memory>
#include <vector>
#include <mitkAbstractAnnotationRenderer.h>

namespace mitk
{
  class AbstractAnnotationRenderer;

  class AnnotationService
  {
  public:
    typedef std::vector<std::unique_ptr<AbstractAnnotationRenderer> > AnnotationRendererServices;

    AnnotationService();
    ~AnnotationService();

    static AbstractAnnotationRenderer* GetAnnotationRenderer(const std::string& arTypeID,
                                                             const std::string& rendererID);

    template<typename T>
    static void RegisterAnnotationRenderer(const std::string &rendererID)
    {
      std::unique_ptr<T> ar(new T(rendererID));
      // Define ServiceProps
      us::ServiceProperties props;
      props[ AbstractAnnotationRenderer::US_PROPKEY_RENDERER_ID ] = rendererID;
      props[ AbstractAnnotationRenderer::US_PROPKEY_ID ] = ar->GetID();

      us::GetModuleContext()->RegisterService(ar.get(),props);
      m_AnnotationRendererServices.push_back(std::move(ar));
    }

  private:

    AnnotationService(const AnnotationService&);
    AnnotationService& operator=(const AnnotationService&);

    static AnnotationRendererServices m_AnnotationRendererServices;
  };
}

#endif
