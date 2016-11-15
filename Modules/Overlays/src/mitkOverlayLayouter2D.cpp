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
#include <mitkVtkLayerController.h>

namespace mitk
{
  const std::string OverlayLayouter2D::ANNOTATIONRENDERER_ID = "OverlayLayouter2D";

  const std::string OverlayLayouter2D::PROP_LAYOUT = "Layout";
  const std::string OverlayLayouter2D::PROP_LAYOUT_PRIORITY = PROP_LAYOUT + ".priority";
  const std::string OverlayLayouter2D::PROP_LAYOUT_ALIGNMENT = PROP_LAYOUT + ".alignment";
  const std::string OverlayLayouter2D::PROP_LAYOUT_MARGIN = PROP_LAYOUT + ".margin";

  void OverlayLayouter2D::SetMargin2D(Overlay *overlay, const Point2D &OffsetVector)
  {
    mitk::Point2dProperty::Pointer OffsetVectorProperty = mitk::Point2dProperty::New(OffsetVector);
    overlay->SetProperty(PROP_LAYOUT_MARGIN, OffsetVectorProperty.GetPointer());
  }

  Point2D OverlayLayouter2D::GetMargin2D(Overlay *overlay)
  {
    mitk::Point2D OffsetVector;
    OffsetVector.Fill(0);
    overlay->GetPropertyValue<mitk::Point2D>(PROP_LAYOUT_MARGIN, OffsetVector);
    return OffsetVector;
  }

  OverlayLayouter2D::OverlayLayouter2D(const std::string &rendererId)
    : AbstractAnnotationRenderer(rendererId, OverlayLayouter2D::ANNOTATIONRENDERER_ID)
  {
  }

  void OverlayLayouter2D::AddAlignmentProperty(Overlay *overlay,
                                               Alignment activeAlignment,
                                               Point2D margin,
                                               int priority)
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
    SetMargin2D(overlay, margin);
  }

  void OverlayLayouter2D::OnAnnotationRenderersChanged()
  {
    if (!this->GetCurrentBaseRenderer())
      return;
    m_OverlayContainerMap.clear();
    for (Overlay *overlay : this->GetServices())
    {
      if (!overlay)
        continue;
      BaseProperty *prop = overlay->GetProperty(PROP_LAYOUT_ALIGNMENT);
      EnumerationProperty *enumProb = dynamic_cast<EnumerationProperty *>(prop);
      Alignment currentAlignment = TopLeft;
      Point2D margin;
      margin.Fill(5);
      int priority = -1;
      overlay->GetIntProperty(PROP_LAYOUT_PRIORITY, priority);
      if (!enumProb)
      {
        AddAlignmentProperty(overlay, currentAlignment, margin, priority);
      }
      else
      { // TODO19786 insert
        currentAlignment = static_cast<Alignment>(enumProb->GetValueAsId());
      }
      OverlayRankedMap &overlayVec = m_OverlayContainerMap[currentAlignment];
      if (!overlayVec.empty() && priority < 0)
      {
        int max = overlayVec.rbegin()->first;
        if (max < 100)
          priority = 100;
        else
          priority = max + 1;
      }
      overlayVec.insert(std::pair<int, Overlay *>(priority, overlay));
    }
    this->PrepareLayout();
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

  void OverlayLayouter2D::OnRenderWindowModified() { PrepareLayout(); }
  void OverlayLayouter2D::AddOverlay(
    Overlay *overlay, const std::string &rendererID, Alignment alignment, double marginX, double marginY, int priority)
  {
    GetAnnotationRenderer(rendererID);
    us::ServiceProperties props;
    props[Overlay::US_PROPKEY_AR_ID] = ANNOTATIONRENDERER_ID;
    props[Overlay::US_PROPKEY_RENDERER_ID] = rendererID;
    overlay->RegisterAsMicroservice(props);
    Point2D margin;
    margin[0] = marginX;
    margin[1] = marginY;
    AddAlignmentProperty(overlay, alignment, margin, priority);
  }

  void OverlayLayouter2D::AddOverlay(
    Overlay *overlay, BaseRenderer *renderer, Alignment alignment, double marginX, double marginY, int priority)
  {
    AddOverlay(overlay, renderer->GetName(), alignment, marginX, marginY, priority);
  }

  void OverlayLayouter2D::PrepareLayout()
  {
    if (!this->GetCurrentBaseRenderer())
      return;
    int *size = this->GetCurrentBaseRenderer()->GetVtkRenderer()->GetSize();
    PrepareTopLeftLayout(size);
    PrepareTopLayout(size);
    PrepareTopRightLayout(size);
    PrepareBottomLeftLayout(size);
    PrepareBottomLayout(size);
    PrepareBottomRightLayout(size);
    PrepareLeftLayout(size);
    PrepareRightLayout(size);
  }
  void OverlayLayouter2D::PrepareTopLeftLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = displaySize[1];
    mitk::Overlay::Bounds bounds;
    OverlayRankedMap &overlayMap = m_OverlayContainerMap[TopLeft];
    for (auto it = overlayMap.cbegin(); it != overlayMap.cend(); ++it)
    {
      Overlay *overlay = it->second;
      margin = GetMargin2D(overlay);
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = posX + margin[0];
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
    OverlayRankedMap &overlayMap = m_OverlayContainerMap[Top];
    for (auto it = overlayMap.cbegin(); it != overlayMap.cend(); ++it)
    {
      Overlay *overlay = it->second;
      margin = GetMargin2D(overlay);
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
    OverlayRankedMap &overlayMap = m_OverlayContainerMap[TopRight];
    for (auto it = overlayMap.cbegin(); it != overlayMap.cend(); ++it)
    {
      Overlay *overlay = it->second;
      margin = GetMargin2D(overlay);
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] - (bounds.Size[0] + margin[0]);
      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }

  void OverlayLayouter2D::PrepareRightLayout(int *displaySize)
  {
    double posY;
    Point2D margin;
    double height = GetHeight(m_OverlayContainerMap[Right], GetCurrentBaseRenderer());
    posY = (height / 2.0 + displaySize[1]) / 2.0;
    mitk::Overlay::Bounds bounds;
    OverlayRankedMap &overlayMap = m_OverlayContainerMap[Right];
    for (auto it = overlayMap.cbegin(); it != overlayMap.cend(); ++it)
    {
      Overlay *overlay = it->second;
      margin = GetMargin2D(overlay);
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = displaySize[0] - (bounds.Size[0] + margin[0]);
      bounds.Position[1] = posY + margin[1];
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }

  void OverlayLayouter2D::PrepareLeftLayout(int *displaySize)
  {
    double posY;
    Point2D margin;
    double height = GetHeight(m_OverlayContainerMap[Left], GetCurrentBaseRenderer());
    posY = (height / 2.0 + displaySize[1]) / 2.0;
    mitk::Overlay::Bounds bounds;
    OverlayRankedMap &overlayMap = m_OverlayContainerMap[Left];
    for (auto it = overlayMap.cbegin(); it != overlayMap.cend(); ++it)
    {
      Overlay *overlay = it->second;
      margin = GetMargin2D(overlay);
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = margin[0];
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
    OverlayRankedMap &overlayMap = m_OverlayContainerMap[BottomLeft];
    for (auto it = overlayMap.cbegin(); it != overlayMap.cend(); ++it)
    {
      Overlay *overlay = it->second;
      margin = GetMargin2D(overlay);
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
    OverlayRankedMap &overlayMap = m_OverlayContainerMap[Bottom];
    for (auto it = overlayMap.cbegin(); it != overlayMap.cend(); ++it)
    {
      Overlay *overlay = it->second;
      margin = GetMargin2D(overlay);
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
    OverlayRankedMap &overlayMap = m_OverlayContainerMap[BottomRight];
    for (auto it = overlayMap.cbegin(); it != overlayMap.cend(); ++it)
    {
      Overlay *overlay = it->second;
      margin = GetMargin2D(overlay);
      bounds = overlay->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] - (bounds.Size[0] + margin[0]);
      bounds.Position[0] = posX;
      bounds.Position[1] = posY + margin[1];
      overlay->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
      posY += bounds.Size[1] + margin[1];
    }
  }

  double OverlayLayouter2D::GetHeight(OverlayRankedMap &overlays, BaseRenderer *renderer)
  {
    double height = 0;
    for (auto it = overlays.cbegin(); it != overlays.cend(); ++it)
    {
      Overlay *overlay = it->second;
      Overlay::Bounds bounds = overlay->GetBoundsOnDisplay(renderer);
      height += bounds.Size[0];
      height += GetMargin2D(overlay)[0];
    }
    return height;
  }
}
