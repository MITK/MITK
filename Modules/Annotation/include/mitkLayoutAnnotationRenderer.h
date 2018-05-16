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

#ifndef LayoutAnnotationRenderer_H
#define LayoutAnnotationRenderer_H

#include "MitkAnnotationExports.h"
#include "mitkAbstractAnnotationRenderer.h"

namespace mitk
{
  class BaseRenderer;

  /** \brief The LayoutAnnotationRenderer is used for the layouted placement of mitk::Annotation Objects.
   *
   * An instance of this service is registered for a specific Baserenderer and is used to manage all annotations which
   * are added to it.
   * The static function AddAnnotation is used to register an annotation to a specific service and to create this
   * service if it does not exist yet. The position of the layouted annotation can be passed as a parameter.
   *
   * See \ref AnnotationPage for more info.
   **/
  class MITKANNOTATION_EXPORT LayoutAnnotationRenderer : public AbstractAnnotationRenderer
  {
  public:
    static const std::string PROP_LAYOUT;
    static const std::string PROP_LAYOUT_PRIORITY;
    static const std::string PROP_LAYOUT_ALIGNMENT;
    static const std::string PROP_LAYOUT_MARGIN;
    enum Alignment
    {
      TopLeft,
      Top,
      TopRight,
      BottomLeft,
      Bottom,
      BottomRight,
      Left,
      Right
    };
    typedef std::multimap<int, mitk::Annotation *> AnnotationRankedMap;
    typedef std::map<Alignment, AnnotationRankedMap> AnnotationLayouterContainerMap;

    /** \brief virtual destructor in order to derive from this class */
    ~LayoutAnnotationRenderer() override;

    const std::string GetID() const;

    static LayoutAnnotationRenderer *GetAnnotationRenderer(const std::string &rendererID);

    void OnRenderWindowModified() override;

    static void AddAnnotation(Annotation *annotation,
                              const std::string &rendererID,
                              Alignment alignment = TopLeft,
                              double marginX = 5,
                              double marginY = 5,
                              int priority = -1);

    static void AddAnnotation(Annotation *annotation,
                              BaseRenderer *renderer,
                              Alignment alignment = TopLeft,
                              double marginX = 5,
                              double marginY = 5,
                              int priority = -1);

    void PrepareLayout();

  private:
    LayoutAnnotationRenderer(const std::string &rendererId);

    static void AddAlignmentProperty(Annotation *annotation, Alignment activeAlignment, Point2D margin, int priority);

    void PrepareTopLeftLayout(int *displaySize);
    void PrepareTopLayout(int *displaySize);
    void PrepareTopRightLayout(int *displaySize);
    void PrepareBottomLeftLayout(int *displaySize);
    void PrepareBottomLayout(int *displaySize);
    void PrepareBottomRightLayout(int *displaySize);
    void PrepareLeftLayout(int *displaySize);
    void PrepareRightLayout(int *displaySize);

    static double GetHeight(AnnotationRankedMap &annotations, BaseRenderer *renderer);

    void OnAnnotationRenderersChanged() override;
    static const std::string ANNOTATIONRENDERER_ID;
    AnnotationLayouterContainerMap m_AnnotationContainerMap;
    static void SetMargin2D(Annotation *annotation, const Point2D &OffsetVector);
    static Point2D GetMargin2D(Annotation *annotation);
  };

} // namespace mitk

#endif // LayoutAnnotationRenderer_H
