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

#include "mitkVtkLogoRepresentation.h"
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkActor2D.h"
#include "vtkPolyData.h"
#include "vtkTexture.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkPropCollection.h"
#include "vtkRenderer.h"
#include "vtkWindow.h"


vtkStandardNewMacro(mitkVtkLogoRepresentation);

//----------------------------------------------------------------------
inline void mitkVtkLogoRepresentation::AdjustImageSize(double o[2],
                                                   double borderSize[2],
                                                   double imageSize[2])
{
  // Scale the image to fit with in the border.
  // Also update the origin so the image is centered.
  double r0 = borderSize[0]/imageSize[0];
  double r1 = borderSize[1]/imageSize[1];
  if ( r0 > r1 )
    {
    imageSize[0] *= r1;
    imageSize[1] *= r1;
    }
  else
    {
    imageSize[0] *= r0;
    imageSize[1] *= r0;
    }
}

//-------------------------------------------------------------------------
void mitkVtkLogoRepresentation::BuildRepresentation()
{
  if ( this->GetMTime() > this->BuildTime ||
       (this->Renderer && this->Renderer->GetVTKWindow() &&
        this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime) )
    {

    // Determine and adjust the size of the image
    if ( this->Image )
      {
      double imageSize[2], borderSize[2], o[2];
      imageSize[0] = 0.0;
      imageSize[1] = 0.0;
      //this->Image->Update();
      if ( this->Image->GetDataDimension() == 2 )
        {
        int dims[3];
        this->Image->GetDimensions(dims);
        imageSize[0] = static_cast<double>(dims[0]);
        imageSize[1] = static_cast<double>(dims[1]);
        }
      int *p1 = this->PositionCoordinate->
        GetComputedDisplayValue(this->Renderer);
      int *p2 = this->Position2Coordinate->
        GetComputedDisplayValue(this->Renderer);
      borderSize[0] = p2[0];
      borderSize[1] = p2[1];
      o[0] = static_cast<double>(p1[0]);
      o[1] = static_cast<double>(p1[1]);

      // this preserves the image aspect ratio. The image is
      // centered around the center of the bordered ragion.
      this->AdjustImageSize(o,borderSize,imageSize);

      // Update the points
      this->Texture->SetInputData(this->Image);
      int* size = this->Renderer->GetSize();

      switch(this->cornerPosition)
      {
      case 0:
      {
        this->TexturePoints->SetPoint(0, o[0],o[1],0.0);
        this->TexturePoints->SetPoint(1, o[0]+imageSize[0],o[1],0.0);
        this->TexturePoints->SetPoint(2, o[0]+imageSize[0],o[1]+imageSize[1],0.0);
        this->TexturePoints->SetPoint(3, o[0],o[1]+imageSize[1],0.0);
        break;
      }
      case 1:
      {
        o[0] = size[0]-o[0];
        this->TexturePoints->SetPoint(0, o[0]-imageSize[0],o[1],0.0);
        this->TexturePoints->SetPoint(1, o[0],o[1],0.0);
        this->TexturePoints->SetPoint(2, o[0],o[1]+imageSize[1],0.0);
        this->TexturePoints->SetPoint(3, o[0]-imageSize[0],o[1]+imageSize[1],0.0);
        break;
      }
      case 2:
      {
        o[0] = size[0]-o[0];
        o[1] = size[1]-o[1];
        this->TexturePoints->SetPoint(0, o[0]-imageSize[0],o[1]-imageSize[1],0.0);
        this->TexturePoints->SetPoint(1, o[0],o[1]-imageSize[1],0.0);
        this->TexturePoints->SetPoint(2, o[0],o[1],0.0);
        this->TexturePoints->SetPoint(3, o[0]-imageSize[0],o[1],0.0);
        break;
      }
      case 3:
      {
        o[1] = size[1]-o[1];
        this->TexturePoints->SetPoint(0, o[0],o[1]-imageSize[1],0.0);
        this->TexturePoints->SetPoint(1, o[0]+imageSize[0],o[1]-imageSize[1],0.0);
        this->TexturePoints->SetPoint(2, o[0]+imageSize[0],o[1],0.0);
        this->TexturePoints->SetPoint(3, o[0],o[1],0.0);
        break;
      }
      default:
      {
        this->TexturePoints->SetPoint(0, o[0],o[1],0.0);
        this->TexturePoints->SetPoint(1, o[0]+imageSize[0],o[1],0.0);
        this->TexturePoints->SetPoint(2, o[0]+imageSize[0],o[1]+imageSize[1],0.0);
        this->TexturePoints->SetPoint(3, o[0],o[1]+imageSize[1],0.0);
      }
      }


      }
    }
}

mitkVtkLogoRepresentation::mitkVtkLogoRepresentation() : cornerPosition(0)
{

}

mitkVtkLogoRepresentation::~mitkVtkLogoRepresentation()
{

}

//-------------------------------------------------------------------------
void mitkVtkLogoRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->Image )
    {
    os << indent << "Image:\n";
    this->Image->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Image: (none)\n";
    }

  if ( this->ImageProperty )
    {
    os << indent << "Image Property:\n";
    this->ImageProperty->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Image Property: (none)\n";
    }
}
