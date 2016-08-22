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

#include "mitkOverlay2DLayouter.h"

std::string mitk::Overlay2DLayouter::STANDARD_2D()
{
  static const std::string s = "Standard2D";
  return s;
}

std::string mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT()
{
  static const std::string s = STANDARD_2D() + ".TopLeft";
  return s;
}
std::string mitk::Overlay2DLayouter::STANDARD_2D_TOP()
{
  static const std::string s = STANDARD_2D() + ".Top";
  return s;
}
std::string mitk::Overlay2DLayouter::STANDARD_2D_TOPRIGHT()
{
  static const std::string s = STANDARD_2D() + ".TopRight";
  return s;
}
std::string mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMLEFT()
{
  static const std::string s = STANDARD_2D() + ".BottomLeft";
  return s;
}
std::string mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM()
{
  static const std::string s = STANDARD_2D() + ".Bottom";
  return s;
}
std::string mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMRIGHT()
{
  static const std::string s = STANDARD_2D() + ".BottomRight";
  return s;
}
std::string mitk::Overlay2DLayouter::STANDARD_2D_LEFT()
{
  static const std::string s = STANDARD_2D() + ".Left";
  return s;
}
std::string mitk::Overlay2DLayouter::STANDARD_2D_RIGHT()
{
  static const std::string s = STANDARD_2D() + ".Right";
  return s;
}

mitk::Overlay2DLayouter::Overlay2DLayouter() : m_Margin(5)
{
}

mitk::Overlay2DLayouter::~Overlay2DLayouter()
{
}

mitk::Overlay2DLayouter::Pointer mitk::Overlay2DLayouter::CreateLayouter(const std::string &identifier,
                                                                         mitk::BaseRenderer *renderer)
{
  if (renderer == NULL)
    return NULL;
  Alignment alignment;
  if (identifier.compare(0, 10, STANDARD_2D()) != 0)
    return NULL;
  if (identifier.compare(STANDARD_2D_TOPLEFT()) == 0)
    alignment = TopLeft;
  else if (identifier.compare(STANDARD_2D_TOP()) == 0)
    alignment = Top;
  else if (identifier.compare(STANDARD_2D_TOPRIGHT()) == 0)
    alignment = TopRight;
  else if (identifier.compare(STANDARD_2D_BOTTOMLEFT()) == 0)
    alignment = BottomLeft;
  else if (identifier.compare(STANDARD_2D_BOTTOM()) == 0)
    alignment = Bottom;
  else if (identifier.compare(STANDARD_2D_BOTTOMRIGHT()) == 0)
    alignment = BottomRight;
  else if (identifier.compare(STANDARD_2D_LEFT()) == 0)
    alignment = Left;
  else if (identifier.compare(STANDARD_2D_RIGHT()) == 0)
    alignment = Right;
  else
    return NULL;

  mitk::Overlay2DLayouter::Pointer layouter = mitk::Overlay2DLayouter::New();
  layouter->m_Alignment = alignment;
  layouter->m_Identifier = identifier;
  layouter->SetBaseRenderer(renderer);
  return layouter;
}

std::vector<mitk::Overlay2DLayouter::Pointer> mitk::Overlay2DLayouter::CreateLayouter(mitk::BaseRenderer *renderer)
{
  std::vector<mitk::Overlay2DLayouter::Pointer> layouterList;
  layouterList.push_back(CreateLayouter(TopLeft, renderer));
  layouterList.push_back(CreateLayouter(Top, renderer));
  layouterList.push_back(CreateLayouter(TopRight, renderer));
  layouterList.push_back(CreateLayouter(BottomLeft, renderer));
  layouterList.push_back(CreateLayouter(Bottom, renderer));
  layouterList.push_back(CreateLayouter(BottomRight, renderer));
  layouterList.push_back(CreateLayouter(Left, renderer));
  layouterList.push_back(CreateLayouter(Right, renderer));
  return layouterList;
}

mitk::Overlay2DLayouter::Pointer mitk::Overlay2DLayouter::CreateLayouter(mitk::Overlay2DLayouter::Alignment alignment,
                                                                         mitk::BaseRenderer *renderer)
{
  if (renderer == NULL)
    return NULL;
  std::string identifier;
  switch (alignment)
  {
    case TopLeft:
      identifier = STANDARD_2D_TOPLEFT();
      break;
    case Top:
      identifier = STANDARD_2D_TOP();
      break;
    case TopRight:
      identifier = STANDARD_2D_TOPRIGHT();
      break;
    case BottomLeft:
      identifier = STANDARD_2D_BOTTOMLEFT();
      break;
    case Bottom:
      identifier = STANDARD_2D_BOTTOM();
      break;
    case BottomRight:
      identifier = STANDARD_2D_BOTTOMRIGHT();
      break;
    case Left:
      identifier = STANDARD_2D_LEFT();
      break;
    case Right:
      identifier = STANDARD_2D_RIGHT();
      break;
    default:
      return NULL;
  }

  mitk::Overlay2DLayouter::Pointer layouter = mitk::Overlay2DLayouter::New();
  layouter->m_Alignment = alignment;
  layouter->m_Identifier = identifier;
  layouter->SetBaseRenderer(renderer);
  return layouter;
}

void mitk::Overlay2DLayouter::PrepareLayout()
{
  std::list<mitk::Overlay *> managedOverlays = GetManagedOverlays();
  std::list<mitk::Overlay *>::iterator it;
  mitk::Overlay::Bounds bounds;
  double posX, posY;
  int *size = GetBaseRenderer()->GetVtkRenderer()->GetSize();

  // The alignment enum defines the type of this layouter
  switch (m_Alignment)
  {
    case TopLeft:
      posX = m_Margin;
      posY = size[1] - m_Margin;
      for (it = managedOverlays.begin(); it != managedOverlays.end(); it++)
      {
        mitk::Overlay *overlay = *it;
        bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
        posY -= bounds.Size[1];
        bounds.Position[0] = posX;
        bounds.Position[1] = posY;
        overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
        posY -= m_Margin;
      }
      break;
    default:
      break;
    case Top:
      posX = 0;
      posY = size[1] - m_Margin;
      for (it = managedOverlays.begin(); it != managedOverlays.end(); it++)
      {
        mitk::Overlay *overlay = *it;
        bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
        posX = size[0] / 2 - bounds.Size[0] / 2;
        posY -= bounds.Size[1];
        bounds.Position[0] = posX;
        bounds.Position[1] = posY;
        overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
        posY -= m_Margin;
      }
      break;
    case TopRight:
      posX = 0;
      posY = size[1] - m_Margin;
      for (it = managedOverlays.begin(); it != managedOverlays.end(); it++)
      {
        mitk::Overlay *overlay = *it;
        bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
        posX = size[0] - (bounds.Size[0] + m_Margin);
        posY -= bounds.Size[1];
        bounds.Position[0] = posX;
        bounds.Position[1] = posY;
        overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
        posY -= m_Margin;
      }
      break;
    case BottomLeft:
      posX = m_Margin;
      posY = m_Margin;
      for (it = managedOverlays.begin(); it != managedOverlays.end(); it++)
      {
        mitk::Overlay *overlay = *it;
        bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
        bounds.Position[0] = posX;
        bounds.Position[1] = posY;
        overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
        posY += bounds.Size[1];
        posY += m_Margin;
      }
      break;
    case Bottom:
      posX = 0;
      posY = m_Margin;
      for (it = managedOverlays.begin(); it != managedOverlays.end(); it++)
      {
        mitk::Overlay *overlay = *it;
        bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
        posX = size[0] / 2 - bounds.Size[0] / 2;
        bounds.Position[0] = posX;
        bounds.Position[1] = posY;
        overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
        posY += bounds.Size[1];
        posY += m_Margin;
      }
      break;
    case BottomRight:
      posX = 0;
      posY = m_Margin;
      for (it = managedOverlays.begin(); it != managedOverlays.end(); it++)
      {
        mitk::Overlay *overlay = *it;
        bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
        posX = size[0] - (bounds.Size[0] + m_Margin);
        bounds.Position[0] = posX;
        bounds.Position[1] = posY;
        overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
        posY += bounds.Size[1];
        posY += m_Margin;
      }
      break;
    case Right:
      posX = m_Margin;
      posY = size[1] / 2.0;
      for (it = managedOverlays.begin(); it != managedOverlays.end(); it++)
      {
        mitk::Overlay *overlay = *it;
        bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
        posY = (size[1] - bounds.Size[1]) / 2.0;
        bounds.Position[0] = posX;
        bounds.Position[1] = posY;
        overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
        posY += bounds.Size[1];
        posY += m_Margin;
      }
      break;
    case Left:
      posX = size[0] - m_Margin;
      posY = size[1] / 2.0;
      for (it = managedOverlays.begin(); it != managedOverlays.end(); it++)
      {
        mitk::Overlay *overlay = *it;
        bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
        posY = (size[1] - bounds.Size[1]) / 2.0;
        posX = size[0] - (bounds.Size[0] + m_Margin);
        bounds.Position[0] = posX;
        bounds.Position[1] = posY;
        overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
        posY += bounds.Size[1];
        posY += m_Margin;
      }
      break;
  }
}
