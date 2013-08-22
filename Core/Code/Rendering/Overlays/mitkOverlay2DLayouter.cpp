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

const char* mitk::Overlay2DLayouter::STANDARD_2D_TOPLEFT = "STANDARD_2D_TOPLEFT";
const char* mitk::Overlay2DLayouter::STANDARD_2D_TOP = "STANDARD_2D_TOP";
const char* mitk::Overlay2DLayouter::STANDARD_2D_TOPRIGHT = "STANDARD_2D_TOPRIGHT";
const char* mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMLEFT = "STANDARD_2D_BOTTOMLEFT";
const char* mitk::Overlay2DLayouter::STANDARD_2D_BOTTOM = "STANDARD_2D_BOTTOM";
const char* mitk::Overlay2DLayouter::STANDARD_2D_BOTTOMRIGHT = "STANDARD_2D_BOTTOMRIGHT";

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

  //The alignment enum defines the type of this layouter
  switch (m_Alignment) {
  case TopLeft:
    posX = m_Margin;
    posY = size[1]-m_Margin;
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
      posY -= bounds.Size[1];
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
      posY -= m_Margin;
    }
    break;
  default: break;
  case Top:
    posX = 0;
    posY = size[1]-m_Margin;
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
      posX = size[0]/2 - bounds.Size[0]/2;
      posY -= bounds.Size[1];
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
      posY -= m_Margin;
    }
    break;
  case TopRight:
    posX = 0;
    posY = size[1]-m_Margin;
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
      posX = size[0] - (bounds.Size[0]+m_Margin);
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
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
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
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
      posX = size[0]/2 - bounds.Size[0]/2;
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
    for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
    {
      mitk::Overlay* overlay = *it;
      bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
      posX = size[0] - (bounds.Size[0]+m_Margin);
      bounds.Position[0] = posX;
      bounds.Position[1] = posY;
      overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
      posY += bounds.Size[1];
      posY += m_Margin;
    }
    break;
  }
}
