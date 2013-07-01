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

const std::string mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT = "STANDARD_2D_TOPLEFT";
const std::string mitk::Overlay2DLayouter::STANDARD_2D_TOP = "STANDARD_2D_TOP";
const std::string mitk::Overlay2DLayouter::STANDARD_2D_TOPRIGHT = "STANDARD_2D_TOPRIGHT";
const std::string mitk::Overlay2DLayouter::STANDARD_2D_LEFT = "STANDARD_2D_LEFT";
const std::string mitk::Overlay2DLayouter::STANDARD_2D_MIDDLE = "STANDARD_2D_MIDDLE";
const std::string mitk::Overlay2DLayouter::STANDARD_2D_RIGHT = "STANDARD_2D_RIGHT";
const std::string mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMLEFT = "STANDARD_2D_BOTTOMLEFT";
const std::string mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM = "STANDARD_2D_BOTTOM";
const std::string mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMRIGHT = "STANDARD_2D_BOTTOMRIGHT";

mitk::Overlay2DLayouter::Overlay2DLayouter()
{
  m_Margin = 5;
}

mitk::Overlay2DLayouter::~Overlay2DLayouter()
{
}

void mitk::Overlay2DLayouter::PrepareLayout()
{
  std::list<mitk::Overlay*> managedOverlays = GetManagedOverlays();
  std::list<mitk::Overlay*>::iterator it;
  mitk::Overlay::Bounds bounds;
  double posX,posY;
  int* size = GetBaseRenderer()->GetVtkRenderer()->GetSize();

  switch (m_Alignment) {
  case TopLeft:
    posX = m_Margin;
    posY = size[1]-m_Margin;
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
      posY -= 25;
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
      posY -= m_Margin;
    }
    break;
  default: break;
/*  case Top:
    posX = 0;
    posY = size[1]-m_Margin;
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
      bounds.Position[0] = (size[0]/2.0) - (bounds.Size[1]/2.0);
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
      posY += (bounds.Size[1]+m_Margin);
    }
    break;
  case TopRight:
    posX = size[0]-m_Margin;
    posY = size[1]-m_Margin;
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
      posY += (bounds.Size[1]+m_Margin);
    }
    break;
  case Left:
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
//      bounds.Position[0] = i++;
//      bounds.Position[1] = i++;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
    }
    break;
  case Middle:
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
//      bounds.Position[0] = i++;
//      bounds.Position[1] = i++;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
    }
    break;
  case Right:
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
//      bounds.Position[0] = i++;
//      bounds.Position[1] = i++;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
    }
    break;
  case BottomLeft:
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
//      bounds.Position[0] = i++;
//      bounds.Position[1] = i++;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
    }
    break;
  case Bottom:
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
//      bounds.Position[0] = i++;
//      bounds.Position[1] = i++;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
    }
    break;
  case BottomRight:
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
//      bounds.Position[0] = i++;
//      bounds.Position[1] = i++;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
    }
    break;*/
  }
}
