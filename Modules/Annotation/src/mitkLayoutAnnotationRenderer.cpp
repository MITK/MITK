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

#include "mitkLayoutAnnotationRenderer.h"
#include "mitkBaseRenderer.h"

#include "mitkAnnotationUtils.h"
#include "mitkEnumerationProperty.h"
#include <mitkVtkLayerController.h>

namespace mitk
{
  const std::string LayoutAnnotationRenderer::ANNOTATIONRENDERER_ID = "LayoutAnnotationRenderer";

  const std::string LayoutAnnotationRenderer::PROP_LAYOUT = "Layout";
  const std::string LayoutAnnotationRenderer::PROP_LAYOUT_PRIORITY = PROP_LAYOUT + ".priority";
  const std::string LayoutAnnotationRenderer::PROP_LAYOUT_ALIGNMENT = PROP_LAYOUT + ".alignment";
  const std::string LayoutAnnotationRenderer::PROP_LAYOUT_MARGIN = PROP_LAYOUT + ".margin";

  void LayoutAnnotationRenderer::SetMargin2D(Annotation *Annotation, const Point2D &OffsetVector)
  {
    mitk::Point2dProperty::Pointer OffsetVectorProperty = mitk::Point2dProperty::New(OffsetVector);
    Annotation->SetProperty(PROP_LAYOUT_MARGIN, OffsetVectorProperty.GetPointer());
  }

  Point2D LayoutAnnotationRenderer::GetMargin2D(Annotation *Annotation)
  {
    mitk::Point2D OffsetVector;
    OffsetVector.Fill(0);
    Annotation->GetPropertyValue<mitk::Point2D>(PROP_LAYOUT_MARGIN, OffsetVector);
    return OffsetVector;
  }

  LayoutAnnotationRenderer::LayoutAnnotationRenderer(const std::string &rendererId)
    : AbstractAnnotationRenderer(rendererId, LayoutAnnotationRenderer::ANNOTATIONRENDERER_ID)
  {
  }

  void LayoutAnnotationRenderer::AddAlignmentProperty(Annotation *Annotation,
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
    Annotation->AddProperty(PROP_LAYOUT_ALIGNMENT, alignmentProperty.GetPointer());
    Annotation->SetIntProperty(PROP_LAYOUT_PRIORITY, priority);
    SetMargin2D(Annotation, margin);
  }

  void LayoutAnnotationRenderer::OnAnnotationRenderersChanged()
  {
    if (!this->GetCurrentBaseRenderer())
      return;
    m_AnnotationContainerMap.clear();
    for (Annotation *annotation : this->GetServices())
    {
      if (!annotation)
        continue;
      BaseProperty *prop = annotation->GetProperty(PROP_LAYOUT_ALIGNMENT);
      auto *enumProb = dynamic_cast<EnumerationProperty *>(prop);
      Alignment currentAlignment = TopLeft;
      Point2D margin;
      margin.Fill(5);
      int priority = -1;
      annotation->GetIntProperty(PROP_LAYOUT_PRIORITY, priority);
      if (!enumProb)
      {
        AddAlignmentProperty(annotation, currentAlignment, margin, priority);
      }
      else
      { // TODO19786 insert
        currentAlignment = static_cast<Alignment>(enumProb->GetValueAsId());
      }
      AnnotationRankedMap &AnnotationVec = m_AnnotationContainerMap[currentAlignment];
      if (!AnnotationVec.empty() && priority < 0)
      {
        int max = AnnotationVec.rbegin()->first;
        if (max < 100)
          priority = 100;
        else
          priority = max + 1;
      }
      AnnotationVec.insert(std::pair<int, Annotation *>(priority, annotation));
    }
    this->PrepareLayout();
  }

  LayoutAnnotationRenderer::~LayoutAnnotationRenderer() {}
  const std::string LayoutAnnotationRenderer::GetID() const { return ANNOTATIONRENDERER_ID; }
  LayoutAnnotationRenderer *LayoutAnnotationRenderer::GetAnnotationRenderer(const std::string &rendererID)
  {
    LayoutAnnotationRenderer *result = nullptr;
    AbstractAnnotationRenderer *registeredService =
      AnnotationUtils::GetAnnotationRenderer(ANNOTATIONRENDERER_ID, rendererID);
    if (registeredService)
      result = dynamic_cast<LayoutAnnotationRenderer *>(registeredService);
    if (!result)
    {
      result = new LayoutAnnotationRenderer(rendererID);
      AnnotationUtils::RegisterAnnotationRenderer(result);
    }
    return result;
  }

  void LayoutAnnotationRenderer::OnRenderWindowModified() { PrepareLayout(); }
  void LayoutAnnotationRenderer::AddAnnotation(Annotation *Annotation,
                                               const std::string &rendererID,
                                               Alignment alignment,
                                               double marginX,
                                               double marginY,
                                               int priority)
  {
    GetAnnotationRenderer(rendererID);
    us::ServiceProperties props;
    props[Annotation::US_PROPKEY_AR_ID] = ANNOTATIONRENDERER_ID;
    props[Annotation::US_PROPKEY_RENDERER_ID] = rendererID;
    Annotation->RegisterAsMicroservice(props);
    Point2D margin;
    margin[0] = marginX;
    margin[1] = marginY;
    AddAlignmentProperty(Annotation, alignment, margin, priority);
  }

  void LayoutAnnotationRenderer::AddAnnotation(
    Annotation *Annotation, BaseRenderer *renderer, Alignment alignment, double marginX, double marginY, int priority)
  {
    AddAnnotation(Annotation, renderer->GetName(), alignment, marginX, marginY, priority);
  }

  void LayoutAnnotationRenderer::PrepareLayout()
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
  void LayoutAnnotationRenderer::PrepareTopLeftLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = displaySize[1];
    mitk::Annotation::Bounds bounds;
    AnnotationRankedMap &AnnotationMap = m_AnnotationContainerMap[TopLeft];
    for (auto it = AnnotationMap.cbegin(); it != AnnotationMap.cend(); ++it)
    {
      Annotation *Annotation = it->second;
      margin = GetMargin2D(Annotation);
      bounds = Annotation->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = posX + margin[0];
      bounds.Position[1] = posY;
      Annotation->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }
  void LayoutAnnotationRenderer::PrepareTopLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = displaySize[1];
    mitk::Annotation::Bounds bounds;
    AnnotationRankedMap &AnnotationMap = m_AnnotationContainerMap[Top];
    for (auto it = AnnotationMap.cbegin(); it != AnnotationMap.cend(); ++it)
    {
      Annotation *Annotation = it->second;
      margin = GetMargin2D(Annotation);
      bounds = Annotation->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] / 2 - bounds.Size[0] / 2;
      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      Annotation->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }
  void LayoutAnnotationRenderer::PrepareTopRightLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = displaySize[1];
    mitk::Annotation::Bounds bounds;
    AnnotationRankedMap &AnnotationMap = m_AnnotationContainerMap[TopRight];
    for (auto it = AnnotationMap.cbegin(); it != AnnotationMap.cend(); ++it)
    {
      Annotation *Annotation = it->second;
      margin = GetMargin2D(Annotation);
      bounds = Annotation->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] - (bounds.Size[0] + margin[0]);
      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      Annotation->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }

  void LayoutAnnotationRenderer::PrepareRightLayout(int *displaySize)
  {
    double posY;
    Point2D margin;
    double height = GetHeight(m_AnnotationContainerMap[Right], GetCurrentBaseRenderer());
    posY = (height / 2.0 + displaySize[1]) / 2.0;
    mitk::Annotation::Bounds bounds;
    AnnotationRankedMap &AnnotationMap = m_AnnotationContainerMap[Right];
    for (auto it = AnnotationMap.cbegin(); it != AnnotationMap.cend(); ++it)
    {
      Annotation *Annotation = it->second;
      margin = GetMargin2D(Annotation);
      bounds = Annotation->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = displaySize[0] - (bounds.Size[0] + margin[0]);
      bounds.Position[1] = posY + margin[1];
      Annotation->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }

  void LayoutAnnotationRenderer::PrepareLeftLayout(int *displaySize)
  {
    double posY;
    Point2D margin;
    double height = GetHeight(m_AnnotationContainerMap[Left], GetCurrentBaseRenderer());
    posY = (height / 2.0 + displaySize[1]) / 2.0;
    mitk::Annotation::Bounds bounds;
    AnnotationRankedMap &AnnotationMap = m_AnnotationContainerMap[Left];
    for (auto it = AnnotationMap.cbegin(); it != AnnotationMap.cend(); ++it)
    {
      Annotation *Annotation = it->second;
      margin = GetMargin2D(Annotation);
      bounds = Annotation->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posY -= bounds.Size[1] + margin[1];
      bounds.Position[0] = margin[0];
      bounds.Position[1] = posY;
      Annotation->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
    }
  }

  void LayoutAnnotationRenderer::PrepareBottomLeftLayout(int * /*displaySize*/)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = 0;
    mitk::Annotation::Bounds bounds;
    AnnotationRankedMap &AnnotationMap = m_AnnotationContainerMap[BottomLeft];
    for (auto it = AnnotationMap.cbegin(); it != AnnotationMap.cend(); ++it)
    {
      Annotation *Annotation = it->second;
      margin = GetMargin2D(Annotation);
      bounds = Annotation->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      bounds.Position[0] = posX + margin[0];
      bounds.Position[1] = posY + margin[1];
      Annotation->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
      posY += bounds.Size[1] + margin[1];
    }
  }
  void LayoutAnnotationRenderer::PrepareBottomLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = 0;
    mitk::Annotation::Bounds bounds;
    AnnotationRankedMap &AnnotationMap = m_AnnotationContainerMap[Bottom];
    for (auto it = AnnotationMap.cbegin(); it != AnnotationMap.cend(); ++it)
    {
      Annotation *Annotation = it->second;
      margin = GetMargin2D(Annotation);
      bounds = Annotation->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] / 2 - bounds.Size[0] / 2;
      bounds.Position[0] = posX;
      bounds.Position[1] = posY + margin[1];
      Annotation->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
      posY += bounds.Size[1] + margin[1];
    }
  }
  void LayoutAnnotationRenderer::PrepareBottomRightLayout(int *displaySize)
  {
    double posX, posY;
    Point2D margin;
    posX = 0;
    posY = 0;
    mitk::Annotation::Bounds bounds;
    AnnotationRankedMap &AnnotationMap = m_AnnotationContainerMap[BottomRight];
    for (auto it = AnnotationMap.cbegin(); it != AnnotationMap.cend(); ++it)
    {
      Annotation *Annotation = it->second;
      margin = GetMargin2D(Annotation);
      bounds = Annotation->GetBoundsOnDisplay(this->GetCurrentBaseRenderer());

      posX = displaySize[0] - (bounds.Size[0] + margin[0]);
      bounds.Position[0] = posX;
      bounds.Position[1] = posY + margin[1];
      Annotation->SetBoundsOnDisplay(this->GetCurrentBaseRenderer(), bounds);
      posY += bounds.Size[1] + margin[1];
    }
  }

  double LayoutAnnotationRenderer::GetHeight(AnnotationRankedMap &annotations, BaseRenderer *renderer)
  {
    double height = 0;
    for (auto it = annotations.cbegin(); it != annotations.cend(); ++it)
    {
      Annotation *annotation = it->second;
      Annotation::Bounds bounds = annotation->GetBoundsOnDisplay(renderer);
      height += bounds.Size[0];
      height += GetMargin2D(annotation)[0];
    }
    return height;
  }
}
