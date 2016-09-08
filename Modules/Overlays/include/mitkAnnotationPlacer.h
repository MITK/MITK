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

#ifndef ANNOTATIONPLACER_H
#define ANNOTATIONPLACER_H

#include "MitkOverlaysExports.h"
#include <itkLightObject.h>
#include <vtkSmartPointer.h>
#include "mitkOverlay.h"
#include "mitkAbstractAnnotationRenderer.h"
#include "mitkLocalStorageHandler.h"
#include "itkEventObject.h"

namespace mitk {

class BaseRenderer;

/** \brief The AnnotationPlacer updates and manages Overlays and the respective Layouters. */
/** An Instance of the AnnotationPlacer can be registered to several BaseRenderer instances in order to
 * call the update method of each Overlay during the rendering phase of the renderer.
 * See \ref OverlaysPage for more info.
*/
class MITKOVERLAYS_EXPORT AnnotationPlacer : public mitk::AbstractAnnotationRenderer {

public:
  mitkClassMacro(AnnotationPlacer, mitk::AbstractAnnotationRenderer);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

protected:

    /** \brief explicit constructor which disallows implicit conversions */
    explicit AnnotationPlacer();

    /** \brief virtual destructor in order to derive from this class */
    virtual ~AnnotationPlacer();

private:

  /** \brief copy constructor */
  AnnotationPlacer( const AnnotationPlacer &);

  /** \brief assignment operator */
  AnnotationPlacer &operator=(const AnnotationPlacer &);

};

} // namespace mitk

#endif // ANNOTATIONPLACER_H


