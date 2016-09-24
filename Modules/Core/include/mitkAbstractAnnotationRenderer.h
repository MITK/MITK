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
#include <itkObject.h>
#include <mitkCommon.h>
#include "mitkOverlay.h"
#include "mitkServiceInterface.h"
#include "usGetModuleContext.h"

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
class MITKCORE_EXPORT AbstractAnnotationRenderer : public itk::LightObject {

  public:

  mitkClassMacroItkParent(AbstractAnnotationRenderer, itk::LightObject);

  protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit AbstractAnnotationRenderer();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~AbstractAnnotationRenderer();

private:

  /** \brief copy constructor */
  AbstractAnnotationRenderer( const AbstractAnnotationRenderer &);

  /** \brief assignment operator */
  AbstractAnnotationRenderer &operator=(const AbstractAnnotationRenderer &);
};

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::AbstractAnnotationRenderer, "org.mitk.services.AbstractAnnotationRenderer")

#endif // ABSTRACTANNOTATIONRENDERER_H


