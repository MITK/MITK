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

#include "mitkOverlayLayouter2D.h"
#include "mitkBaseRenderer.h"

#include "mitkAnnotationService.h"
#include "mitkEnumerationProperty.h"
#include "mitkOverlay2DLayouter.h"
#include <mitkVtkLayerController.h>

namespace mitk
{
  const std::string OverlayLayouter2D::ANNOTATIONRENDERER_ID = "OverlayLayouter2D";

  const std::string OverlayLayouter2D::PROP_LAYOUT = "Layout";
  const std::string OverlayLayouter2D::PROP_LAYOUT_PRIORITY = PROP_LAYOUT + ".priority";
  const std::string OverlayLayouter2D::PROP_LAYOUT_ALIGNMENT = PROP_LAYOUT + ".alignment";

  OverlayLayouter2D::OverlayLayouter2D(const std::string &rendererId)
    : AbstractAnnotationRenderer(rendererId, OverlayLayouter2D::ANNOTATIONRENDERER_ID)
  {
  }

  void OverlayLayouter2D::AddAlignmentProperty(Overlay *overlay, Alignment activeAlignment, int priority)
  {
    EnumerationProperty::Pointer alignmentProperty(mitk::EnumerationProperty::New());
    alignmentProperty->AddEnum("TopLeft", TopLeft);
    alignmentProperty->AddEnum("Top", Top);
    alignmentProperty->AddEnum("TopRight", TopRight);
    alignmentProperty->AddEnum("BottomLeft ", BottomLeft);
    alignmentProperty->AddEnum("Bottom", Bottom);
    alignmentProperty->AddEnum("BottomRight", BottomRight);
    alignmentProperty->AddEnum("Left", Left);
    alignmentProperty->AddEnum("Right", Right);
    alignmentProperty->SetValue(activeAlignment);
    overlay->AddProperty(PROP_LAYOUT_ALIGNMENT, alignmentProperty.GetPointer());
    overlay->SetIntProperty(PROP_LAYOUT_PRIORITY, priority);
  }

  void OverlayLayouter2D::OnAnnotationRenderersChanged()
  {
    m_OverlayContainerMap.clear();
    for (Overlay *overlay : this->GetServices())
    {
      BaseProperty *prop = overlay->GetProperty(PROP_LAYOUT_ALIGNMENT);
      EnumerationProperty *enumProb = dynamic_cast<EnumerationProperty *>(prop);
      Alignment currentAlignment = TopLeft;
      int priority = -1;
      overlay->GetIntProperty(PROP_LAYOUT_PRIORITY, priority);
      if (!enumProb)
      {
        AddAlignmentProperty(overlay, currentAlignment, priority);
      }
      else
      {
        currentAlignment = static_cast<Alignment>(enumProb->GetValueAsId());
      }
      OverlayVector &overlayVec = m_OverlayContainerMap[currentAlignment];
      if (!overlayVec.empty() && priority >= 0)
      {
        for (auto it = overlayVec.begin(); it != overlayVec.end(); ++it)
        {
          int listPrio = -1;
          (*it)->GetIntProperty(PROP_LAYOUT_PRIORITY, listPrio);
          if (listPrio >= priority)
          {
            overlayVec.insert(it + 1, overlay);
          }
        }
      }
      overlayVec.push_back(overlay);
    }
  }

  OverlayLayouter2D::~OverlayLayouter2D() {}
  const std::string OverlayLayouter2D::GetID() const { return ANNOTATIONRENDERER_ID; }
  OverlayLayouter2D *OverlayLayouter2D::GetAnnotationRenderer(const std::string &rendererID)
  {
    OverlayLayouter2D *result = nullptr;
    AbstractAnnotationRenderer *registeredService =
      AnnotationService::GetAnnotationRenderer(ANNOTATIONRENDERER_ID, rendererID);
    if (registeredService)
      result = dynamic_cast<OverlayLayouter2D *>(registeredService);
    if (!result)
    {
      result = new OverlayLayouter2D(rendererID);
      AnnotationService::RegisterAnnotationRenderer(result);
    }
    return result;
  }

  void OverlayLayouter2D::AddOverlay(Overlay *overlay, const std::string &rendererID, Alignment alignment, int priority)
  {
    GetAnnotationRenderer(rendererID);
    us::ServiceProperties props;
    props[Overlay::US_PROPKEY_AR_ID] = ANNOTATIONRENDERER_ID;
    props[Overlay::US_PROPKEY_RENDERER_ID] = rendererID;
    overlay->RegisterAsMicroservice(props);
    AddAlignmentProperty(overlay, alignment, priority);
  }

  void OverlayLayouter2D::AddOverlay(Overlay *overlay, BaseRenderer *renderer, Alignment alignment, int priority)
  {
    AddOverlay(overlay, renderer->GetName(), alignment, priority);
  }

  void OverlayLayouter2D::PrepareLayout()
  {
    //    std::vector<mitk::Overlay *> managedOverlays = this->GetServices();
    //    std::list<mitk::Overlay *>::iterator it;
    //    mitk::Overlay::Bounds bounds;
    //    int *size = this->GetCurrentBaseRenderer()->GetVtkRenderer()->GetSize();

    // The alignment enum defines the type of this layouter
  }

  void OverlayLayouter2D::PrepareTopLeftLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = displaySize[1];
    mitk::Overlay::Bounds bounds;
    for (mitk::Overlay *overlay : m_OverlayContainerMap[TopLeft])
    {
      margin = overlay->GetMargin2D();
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX += margin[0];
      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }
  void OverlayLayouter2D::PrepareTopLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = displaySize[1];
    mitk::Overlay::Bounds bounds;
    for (mitk::Overlay *overlay : m_OverlayContainerMap[Top])
    {
      margin = overlay->GetMargin2D();
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] / 2 - bounds.Size[0] / 2;
      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }
  void OverlayLayouter2D::PrepareTopRightLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = displaySize[1];
    mitk::Overlay::Bounds bounds;
    for (mitk::Overlay *overlay : m_OverlayContainerMap[TopRight])
    {
      margin = overlay->GetMargin2D();
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] - (bounds.Size[0] + margin[0]);
      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }
  void OverlayLayouter2D::PrepareBottomLeftLayout(int * /*displaySize*/)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = 0;
    mitk::Overlay::Bounds bounds;
    for (mitk::Overlay *overlay : m_OverlayContainerMap[BottomLeft])
    {
      margin = overlay->GetMargin2D();
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      bounds.Position[0] = posX + margin[0];
      bounds.Position[1] = posY + margin[1];
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
      posY += bounds.Size[1] + margin[1];
    }
  }
  void OverlayLayouter2D::PrepareBottomLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = 0;
    mitk::Overlay::Bounds bounds;
    for (mitk::Overlay *overlay : m_OverlayContainerMap[Bottom])
    {
      margin = overlay->GetMargin2D();
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] / 2 - bounds.Size[0] / 2;
      bounds.Position[0] = posX;
      bounds.Position[1] = posY + margin[1];
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
      posY += bounds.Size[1] + margin[1];
    }
  }
  void OverlayLayouter2D::PrepareBottomRightLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = 0;
    mitk::Overlay::Bounds bounds;
    for (mitk::Overlay *overlay : m_OverlayContainerMap[BottomRight])
    {
      margin = overlay->GetMargin2D();
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] - (bounds.Size[0] + margin[0]);
      bounds.Position[0] = posX;
      bounds.Position[1] = posY + margin[1];
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
      posY += bounds.Size[1] + margin[1];
    }
  }
  void OverlayLayouter2D::PrepareLeftLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = displaySize[0];
    posY = displaySize[1] / 2.0;
    mitk::Overlay::Bounds bounds;
    for (mitk::Overlay *overlay : m_OverlayContainerMap[Left])
    {
      margin = overlay->GetMargin2D();
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posY = (displaySize[1] - bounds.Size[1]) / 2.0;
      posX = displaySize[0] - (bounds.Size[0] + margin[0]);
      bounds.Position[0] = posX;
      bounds.Position[1] = posY + margin[1];
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
      posY += bounds.Size[1];
    }
  }
  void OverlayLayouter2D::PrepareRightLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = displaySize[1] / 2.0;
    mitk::Overlay::Bounds bounds;
    for (mitk::Overlay *overlay : m_OverlayContainerMap[Right])
    {
      margin = overlay->GetMargin2D();
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posY = (displaySize[1] - bounds.Size[1]) / 2.0;
      bounds.Position[0] = posX + margin[0];
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
      posY += bounds.Size[1] + margin[1];
    }
  }

  double OverlayLayouter2D::GetHeight(OverlayVector &overlays, BaseRenderer *renderer)
  {
    double height = 0;
    for (auto overlay : overlays)
    {
      Overlay::Bounds bounds = overlay->GetBoundsOnDisplay(renderer);
      height += bounds.Size[0];
      height += overlay->GetMargin2D()[0];
    }
    return height;
  }
}
