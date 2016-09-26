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

const std::string mitk::AbstractAnnotationRenderer::US_INTERFACE_NAME = "org.mitk.services.AbstractAnnotationRenderer";
const std::string mitk::AbstractAnnotationRenderer::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
const std::string mitk::AbstractAnnotationRenderer::US_PROPKEY_RENDERER_ID = US_INTERFACE_NAME + ".rendererId";

mitk::AbstractAnnotationRenderer::AbstractAnnotationRenderer(const std::string& rendererID)
  :m_RendererID(rendererID)
{
}

mitk::AbstractAnnotationRenderer::~AbstractAnnotationRenderer()
{
}

std::string mitk::AbstractAnnotationRenderer::GetRendererID()
{
  return m_RendererID;
}
