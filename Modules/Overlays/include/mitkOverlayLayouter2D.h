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

#ifndef OVERLAYLAYOUTER2D_H
#define OVERLAYLAYOUTER2D_H

#include "MitkOverlaysExports.h"
#include "mitkAbstractAnnotationRenderer.h"

namespace mitk
{
  class BaseRenderer;

  /** \brief The OverlayLayouter2D updates and manages Overlays and the respective Layouters. */
  /** An Instance of the OverlayLayouter2D can be registered to several BaseRenderer instances in order to
   * call the update method of each Overlay during the rendering phase of the renderer.
   * See \ref OverlaysPage for more info.
  */
  class MITKOVERLAYS_EXPORT OverlayLayouter2D : public AbstractAnnotationRenderer
  {
  public:
    static const std::string PROP_LAYOUT;
    static const std::string PROP_LAYOUT_PRIORITY;
    static const std::string PROP_LAYOUT_ALIGNMENT;
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
    typedef std::vector<mitk::Overlay *> OverlayVector;
    typedef std::map<Alignment, OverlayVector> OverlayLayouterContainerMap;

    /** \brief virtual destructor in order to derive from this class */
    virtual ~OverlayLayouter2D();

    const std::string GetID() const;

    static OverlayLayouter2D *GetAnnotationRenderer(const std::string &rendererID);

    static void AddOverlay(Overlay *overlay,
                           const std::string &rendererID,
                           Alignment alignment = TopLeft,
                           int priority = -1);

    static void AddOverlay(Overlay *overlay, BaseRenderer *renderer, Alignment alignment = TopLeft, int priority = -1);

    void PrepareLayout();

  private:
    OverlayLayouter2D(const std::string &rendererId);

    static void AddAlignmentProperty(Overlay *overlay, Alignment activeAlignment, int priority);

    void PrepareTopLeftLayout(int *displaySize);
    void PrepareTopLayout(int *displaySize);
    void PrepareTopRightLayout(int *displaySize);
    void PrepareBottomLeftLayout(int *displaySize);
    void PrepareBottomLayout(int *displaySize);
    void PrepareBottomRightLayout(int *displaySize);
    void PrepareLeftLayout(int *displaySize);
    void PrepareRightLayout(int *displaySize);

    static double GetHeight(OverlayVector &overlays, BaseRenderer *renderer);

    virtual void OnAnnotationRenderersChanged();
    static const std::string ANNOTATIONRENDERER_ID;
    OverlayLayouterContainerMap m_OverlayContainerMap;
  };

} // namespace mitk

#endif // OVERLAYLAYOUTER2D_H
