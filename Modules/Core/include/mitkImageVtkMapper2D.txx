#pragma once

#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkCellArray.h>

namespace mitk
{

template <typename PixelType>
vtkSmartPointer<vtkPolyData> mitk::ImageVtkMapper2D::CreateOutlinePolyData(mitk::BaseRenderer* renderer ){
  LocalStorage* localStorage = this->GetLocalStorage(renderer);

  //get the min and max index values of each direction
  int* extent = localStorage->m_ReslicedImage->GetExtent();
  int xMin = extent[0];
  int xMax = extent[1];
  int yMin = extent[2];
  int yMax = extent[3];

  int* dims = localStorage->m_ReslicedImage->GetDimensions(); //dimensions of the image
  int line = dims[0]; //how many pixels per line?
  int x = xMin; //pixel index x
  int y = yMin; //pixel index y

  //get the depth for each contour
  float depth = CalculateLayerDepth(renderer);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New(); //the points to draw
  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New(); //the lines to connect the points

  // We take the pointer to the first pixel of the image
  auto currentPixel = static_cast<PixelType*>(localStorage->m_ReslicedImage->GetScalarPointer());

  while (y <= yMax)
  {
    //if the current pixel value is set to something
    if ((currentPixel) && (*currentPixel != 0))
    {
      //check in which direction a line is necessary
      //a line is added if the neighbor of the current pixel has the value 0
      //and if the pixel is located at the edge of the image

      //if   vvvvv  not the first line vvvvv
      if (y > yMin && *(currentPixel-line) == 0)
      { //x direction - bottom edge of the pixel
        //add the 2 points
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        //add the line between both points
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  not the last line vvvvv
      if (y < yMax && *(currentPixel+line) == 0)
      { //x direction - top edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  not the first pixel vvvvv
      if ( (x > xMin || y > yMin) && *(currentPixel-1) == 0)
      { //y direction - left edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  not the last pixel vvvvv
      if ( (y < yMax || (x < xMax) ) && *(currentPixel+1) == 0)
      { //y direction - right edge of the pixel
        vtkIdType p1 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      /*  now consider pixels at the edge of the image  */

      //if   vvvvv  left edge of image vvvvv
      if (x == xMin)
      { //draw left edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  right edge of image vvvvv
      if (x == xMax)
      { //draw right edge of the pixel
        vtkIdType p1 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  bottom edge of image vvvvv
      if (y == yMin)
      { //draw bottom edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  top edge of image vvvvv
      if (y == yMax)
      { //draw top edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }
    }//end if currentpixel is set

    x++;

    if (x > xMax)
    { //reached end of line
      x = xMin;
      y++;
    }

    // Increase the pointer-position to the next pixel.
    // This is safe, as the while-loop and the x-reset logic above makes
    // sure we do not exceed the bounds of the image
    currentPixel++;
  }//end of while

  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  // Add the points to the dataset
  polyData->SetPoints(points);
  // Add the lines to the dataset
  polyData->SetLines(lines);
  return polyData;
}

}
