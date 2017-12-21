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

#ifndef ManualPlacementAnnotationRenderer_H
#define ManualPlacementAnnotationRenderer_H

#include "MitkAnnotationExports.h"
#include "mitkAbstractAnnotationRenderer.h"
#include "mitkAnnotation.h"

namespace mitk
{
  class BaseRenderer;

  /** \brief The ManualPlacementAnnotationRenderer is used for the simple placement of mitk::Annotation Objects.
   *
   * An instance of this service is registered for a specific Baserenderer and is used to manage all annotations which
   * are added to it.
   * The static function AddAnnotation is used to register an annotation to a specific service and to create this
   * service if it does not exist yet.
   *
   * See \ref AnnotationPage for more info.
   **/
  class MITKANNOTATION_EXPORT ManualPlacementAnnotationRenderer : public AbstractAnnotationRenderer
  {
  public:
    /** \brief virtual destructor in order to derive from this class */
    ~ManualPlacementAnnotationRenderer() override;

    static ManualPlacementAnnotationRenderer *GetAnnotationRenderer(const std::string &rendererID);

    static void AddAnnotation(Annotation *Annotation, const std::string &rendererID);

    static void AddAnnotation(Annotation *Annotation, BaseRenderer *renderer);

  private:
    ManualPlacementAnnotationRenderer(const std::string &rendererId);

    static const std::string ANNOTATIONRENDERER_ID;
  };

} // namespace mitk

#endif // ManualPlacementAnnotationRenderer_H
