/*===================================================================
 *
The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef ABSTRACTANNOTATIONRENDERER_H
#define ABSTRACTANNOTATIONRENDERER_H

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include "mitkOverlay.h"
#include "mitkServiceInterface.h"
#include "usGetModuleContext.h"
#include "mitkOverlayServiceTracker.h"

namespace mitk {

class BaseRenderer;

/** @brief Baseclass of Overlay layouters */
/**
 *A AbstractAnnotationRenderer can be implemented to control a set of Overlays by means of position and size.
 *AbstractAnnotationRenderer::PrepareLayout() should be implemented with a routine to set the position of the internal m_ManagedOverlays List.
 *A layouter is always connected to one BaseRenderer, so there is one instance of the layouter for each BaseRenderer.
 *One type of layouter should always have a unique identifier.
 *@ingroup Overlays
*/
class MITKCORE_EXPORT AbstractAnnotationRenderer {

public:

  AbstractAnnotationRenderer(const std::string& rendererID);

  /** \brief virtual destructor in order to derive from this class */
  virtual ~AbstractAnnotationRenderer();

  virtual std::string GetID() = 0;
  std::string GetRendererID();

  virtual void RegisterAnnotationRenderer(const std::string& rendererID) = 0;

  static const std::string US_INTERFACE_NAME;
  static const std::string US_PROPKEY_ID;
  static const std::string US_PROPKEY_RENDERER_ID;

private:

  /** \brief copy constructor */
  AbstractAnnotationRenderer( const AbstractAnnotationRenderer &);

  /** \brief assignment operator */
  AbstractAnnotationRenderer &operator=(const AbstractAnnotationRenderer &);

  OverlayServiceTracker* m_OverlayServiceTracker;

  const std::string m_RendererID;
};

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::AbstractAnnotationRenderer, "org.mitk.services.AbstractAnnotationRenderer")

#endif // ABSTRACTANNOTATIONRENDERER_H


