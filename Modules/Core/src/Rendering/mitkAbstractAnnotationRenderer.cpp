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

#include "mitkAbstractAnnotationRenderer.h"
#include <memory>
#include "usGetModuleContext.h"
#include "mitkOverlay.h"

namespace mitk
{

const std::string AbstractAnnotationRenderer::US_INTERFACE_NAME = "org.mitk.services.AbstractAnnotationRenderer";
const std::string AbstractAnnotationRenderer::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
const std::string AbstractAnnotationRenderer::US_PROPKEY_RENDERER_ID = US_INTERFACE_NAME + ".rendererId";

AbstractAnnotationRenderer::AbstractAnnotationRenderer(const std::string& rendererID)
  :m_RendererID(rendererID)
{
  std::string specificRenderer = "(rendererName=" + rendererID +")";
  std::string classInteractionEventObserver = "(" + us::ServiceConstants::OBJECTCLASS() + "=" + ")";
  us::LDAPFilter filter( "(&(|"+ specificRenderer + ")"+classInteractionEventObserver+")" );

  m_OverlayServiceTracker = new OverlayServiceTracker(filter);
}

AbstractAnnotationRenderer::~AbstractAnnotationRenderer()
{
}

const std::string AbstractAnnotationRenderer::GetRendererID() const
{
  return m_RendererID;
}

}
