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

}


mitk::Overlay2DLayouter::~Overlay2DLayouter()
{
}

void mitk::Overlay2DLayouter::PrepareLayout()
{
  int i = 0;
  std::list<mitk::Overlay*> managedOverlays = GetManagedOverlays();
  std::list<mitk::Overlay*>::iterator it;
  for ( it=managedOverlays.begin() ; it != managedOverlays.end(); it++ )
  {
    mitk::Overlay* overlay = *it;
    mitk::Overlay::Bounds bounds = overlay->GetBoundsOnDisplay(this->GetBaseRenderer());
    bounds.Position[0] = i++;
    bounds.Position[1] = i++;
    i+=50;
    overlay->SetBoundsOnDisplay(this->GetBaseRenderer(), bounds);
  }
}
