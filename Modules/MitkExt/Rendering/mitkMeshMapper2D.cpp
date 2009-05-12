/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkMeshMapper2D.h"
#include "mitkMesh.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkLine.h"
#include "mitkGL.h"

#include <vtkLinearTransform.h>

#include <algorithm>

const float selectedColor[]={1.0,0.0,0.6}; //for selected!

mitk::MeshMapper2D::MeshMapper2D()
{
}


mitk::MeshMapper2D::~MeshMapper2D()
{
}

const mitk::Mesh *mitk::MeshMapper2D::GetInput(void)
{
    return static_cast<const mitk::Mesh * > ( GetData() );
}

// Return whether a point is "smaller" than the second
static bool point3DSmaller( const mitk::Point3D& elem1, const mitk::Point3D& elem2 )
{
  if(elem1[0]!=elem2[0])
    return elem1[0] < elem2[0];
  if(elem1[1]!=elem2[1])
    return elem1[1] < elem2[1];
  return elem1[2] < elem2[2];
}

void mitk::MeshMapper2D::Paint( mitk::BaseRenderer *renderer )
{
  if ( !this->IsVisible(renderer) ) return;

  //  @FIXME: Logik fuer update
  bool updateNeccesary = true;

  if (updateNeccesary) 
  {
  //aus GenerateData    
    mitk::Mesh::Pointer input = const_cast<mitk::Mesh*>(this->GetInput());

    // Get the TimeSlicedGeometry of the input object
    const TimeSlicedGeometry* inputTimeGeometry = input->GetTimeSlicedGeometry();
    if (( inputTimeGeometry == NULL ) || ( inputTimeGeometry->GetTimeSteps() == 0 ) )
    {
      return;
    }

    //
    // get the world time
    //
    const Geometry2D* worldGeometry = renderer->GetCurrentWorldGeometry2D();
    assert( worldGeometry != NULL );
    ScalarType time = worldGeometry->GetTimeBounds()[ 0 ];

    //
    // convert the world time in time steps of the input object
    //
    int timeStep=0;
    if ( time > ScalarTypeNumericTraits::NonpositiveMin() )
      timeStep = inputTimeGeometry->MSToTimeStep( time );
    if ( inputTimeGeometry->IsValidTime( timeStep ) == false )
    {
      return;
    }


    mitk::Mesh::MeshType::Pointer itkMesh = input->GetMesh( timeStep );

    if ( itkMesh.GetPointer() == NULL) 
    {
      return;
    }


    mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
    assert(displayGeometry.IsNotNull());

    const PlaneGeometry* worldplanegeometry = dynamic_cast<const PlaneGeometry*>(renderer->GetCurrentWorldGeometry2D());

    //apply color and opacity read from the PropertyList
    ApplyProperties(renderer);

    vtkLinearTransform* transform = GetDataTreeNode()->GetVtkTransform();

    //List of the Points
    Mesh::DataType::PointsContainerConstIterator it, end;
    it=itkMesh->GetPoints()->Begin();
    end=itkMesh ->GetPoints()->End();

    //iterator on the additional data of each point
    Mesh::PointDataIterator dataIt;//, dataEnd;
    dataIt=itkMesh->GetPointData()->Begin();
      
    //for switching back to old color after using selected color
    float unselectedColor[4];
    glGetFloatv(GL_CURRENT_COLOR,unselectedColor);

    while(it!=end)
    {
      mitk::Point3D p, projected_p;
      float vtkp[3];

      itk2vtk(it->Value(), vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2itk(vtkp,p);

      displayGeometry->Project(p, projected_p);
      Vector3D diff=p-projected_p;
      if(diff.GetSquaredNorm()<4.0)
      {
        Point2D pt2d, tmp;
        displayGeometry->Map(projected_p, pt2d);
        displayGeometry->WorldToDisplay(pt2d, pt2d);

        Vector2D horz,vert;
        horz[0]=5; horz[1]=0;
        vert[0]=0; vert[1]=5;
                        
        //check if the point is to be marked as selected 
        if (dataIt->Value().selected)
        {
          horz[0]=8;
          vert[1]=8;
          glColor3f(selectedColor[0],selectedColor[1],selectedColor[2]);//red

          switch (dataIt->Value().pointSpec)
          {
          case PTSTART:
            {
              //a quad
              glBegin (GL_LINE_LOOP);
                tmp=pt2d-horz+vert;      glVertex2fv(&tmp[0]);
                tmp=pt2d+horz+vert;      glVertex2fv(&tmp[0]);
                tmp=pt2d+horz-vert;      glVertex2fv(&tmp[0]);
                tmp=pt2d-horz-vert;      glVertex2fv(&tmp[0]);
              glEnd ();
            }
            break;
          case PTUNDEFINED:
            {
              //a diamond around the point
              glBegin (GL_LINE_LOOP);
                tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
                tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
                tmp=pt2d+horz;      glVertex2fv(&tmp[0]);
                tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
              glEnd ();
            }
            break;
          default:
            break;
          }//switch

          //the actual point
          glBegin (GL_POINTS);
            tmp=pt2d;             glVertex2fv(&tmp[0]);
          glEnd ();
        }
        else //if not selected
        {
          glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
          switch (dataIt->Value().pointSpec)
          {
          case PTSTART:
            {
              //a quad
              glBegin (GL_LINE_LOOP);
                tmp=pt2d-horz+vert;      glVertex2fv(&tmp[0]);
                tmp=pt2d+horz+vert;      glVertex2fv(&tmp[0]);
                tmp=pt2d+horz-vert;      glVertex2fv(&tmp[0]);
                tmp=pt2d-horz-vert;      glVertex2fv(&tmp[0]);
              glEnd ();
            }
          case PTUNDEFINED:
            {
              //drawing crosses
              glBegin (GL_LINES);
                  tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
                  tmp=pt2d+horz;      glVertex2fv(&tmp[0]);
                  tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
                  tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
              glEnd ();
            }
    default:
            {
        break;
            }
          }//switch
        }//else
      }
      ++it;
      ++dataIt;
    }

    //now connect the lines inbetween
    mitk::Mesh::PointType thisPoint;
    Point2D *firstOfCell = NULL;
    Point2D *lastPoint = NULL;
    unsigned int lastPointId = 0;
    bool lineSelected = false;

    Point3D firstOfCell3D;
    Point3D lastPoint3D;
    bool first;
    mitk::Line<mitk::ScalarType> line;
    std::vector<mitk::Point3D> intersectionPoints;
    double t;

    //iterate through all cells and then iterate through all indexes of points in that cell
    Mesh::CellIterator cellIt, cellEnd;
    Mesh::CellDataIterator cellDataIt;//, cellDataEnd;
    Mesh::PointIdIterator cellIdIt, cellIdEnd;

    cellIt = itkMesh->GetCells()->Begin();
    cellEnd = itkMesh->GetCells()->End();
    cellDataIt = itkMesh->GetCellData()->Begin();

    while (cellIt != cellEnd)
    {
      unsigned int numOfPointsInCell = cellIt->Value()->GetNumberOfPoints();
      if (numOfPointsInCell>1)
      {
        //iterate through all id's in the cell
        cellIdIt = cellIt->Value()->PointIdsBegin();
        cellIdEnd = cellIt->Value()->PointIdsEnd();

        firstOfCell3D = input->GetPoint(*cellIdIt);

        intersectionPoints.clear();
        intersectionPoints.reserve(numOfPointsInCell);

        first = true;

        while(cellIdIt != cellIdEnd)
        {
          lastPoint3D = thisPoint;

          thisPoint = input->GetPoint(*cellIdIt);

          //search in data (vector<> selectedLines) if the index of the point is set. if so, then the line is selected.
          lineSelected = false;
          Mesh::SelectedLinesType selectedLines = cellDataIt->Value().selectedLines;
          
          //a line between 1(lastPoint) and 2(pt2d) has the Id 1, so look for the Id of lastPoint
          //since we only start, if we have more than one point in the cell, lastPointId is initiated with 0
          Mesh::SelectedLinesIter position = std::find(selectedLines.begin(), selectedLines.end(), lastPointId);
          if (position != selectedLines.end())
          {
            lineSelected = true;
          }

          mitk::Point3D p, projected_p;
          float vtkp[3];
          itk2vtk(thisPoint, vtkp);
          transform->TransformPoint(vtkp, vtkp);
          vtk2itk(vtkp,p);
          displayGeometry->Project(p, projected_p);
          Vector3D diff=p-projected_p;
          if(diff.GetSquaredNorm()<4.0)
          {
            Point2D pt2d, tmp;
            displayGeometry->Map(projected_p, pt2d);
            displayGeometry->WorldToDisplay(pt2d, pt2d);

            if (lastPoint == NULL)
            {
              //set the first point in the cell. This point in needed to close the polygon
              firstOfCell = new Point2D;
              *firstOfCell = pt2d;
              lastPoint = new Point2D;
              *lastPoint = pt2d;
              lastPointId = *cellIdIt;
            }
            else
            {
              if (lineSelected)
              {
                glColor3f(selectedColor[0],selectedColor[1],selectedColor[2]);//red
                //a line from lastPoint to thisPoint
                glBegin (GL_LINES);
                  glVertex2fv(&(*lastPoint)[0]);
                  glVertex2fv(&pt2d[0]);
                glEnd ();
              }
              else //if not selected
              {
                glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
                //drawing crosses
                glBegin (GL_LINES);
                  glVertex2fv(&(*lastPoint)[0]);
                  glVertex2fv(&pt2d[0]);
                glEnd ();
              }
              //to draw the line to the next in iteration step
              *lastPoint = pt2d;
              //and to search for the selection state of the line
              lastPointId = *cellIdIt;
            }//if..else
          }//if <4.0

          //fill off-plane polygon part 1
          if((!first) && (worldplanegeometry!=NULL))
          {
            line.SetPoints(lastPoint3D, thisPoint);
            if(worldplanegeometry->IntersectionPointParam(line, t) &&
              ((t>=0) && (t<=1))
              )
            {
              intersectionPoints.push_back(line.GetPoint(t));
            }
          }
          ++cellIdIt;
          first=false;
        }//while cellIdIter

        //closed polygon?
        if ( cellDataIt->Value().closed )
        {
          //close the polygon if needed
          if( firstOfCell != NULL )
          {
            lineSelected = false;
            Mesh::SelectedLinesType selectedLines = cellDataIt->Value().selectedLines;
            Mesh::SelectedLinesIter position = std::find(selectedLines.begin(), selectedLines.end(), lastPointId);
            if (position != selectedLines.end())//found the index
            {
              glColor3f(selectedColor[0],selectedColor[1],selectedColor[2]);//red
              //a line from lastPoint to firstPoint
              glBegin (GL_LINES);
                glVertex2fv(&(*lastPoint)[0]);
                glVertex2fv(&(*firstOfCell)[0]);
              glEnd ();
            }
            else 
            {
              glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
              glBegin (GL_LINES);
                glVertex2fv(&(*lastPoint)[0]);
                glVertex2fv(&(*firstOfCell)[0]);
              glEnd ();
            }
          }
        }//if closed

        //Axis-aligned bounding box(AABB) around the cell if selected and set in Property
        bool showBoundingBox;
        if (dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("showBoundingBox")) == NULL)
          showBoundingBox = false;
        else
          showBoundingBox = dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("showBoundingBox"))->GetValue();

        if(showBoundingBox) 
        {
          if (cellDataIt->Value().selected)
          {
            mitk::Mesh::DataType::BoundingBoxPointer aABB = input->GetBoundingBoxFromCell(cellIt->Index());
            if (aABB.IsNotNull())
            {
              mitk::Mesh::PointType min, max;
              min = aABB->GetMinimum();
              max = aABB->GetMaximum();

              //project to the displayed geometry
              Point2D min2D, max2D;
              Point3D p, projected_p;
              float vtkp[3];
              itk2vtk(min, vtkp);
              transform->TransformPoint(vtkp, vtkp);
              vtk2itk(vtkp,p);
              displayGeometry->Project(p, projected_p);
              displayGeometry->Map(projected_p, min2D);
              displayGeometry->WorldToDisplay(min2D, min2D);

              itk2vtk(max, vtkp);
              transform->TransformPoint(vtkp, vtkp);
              vtk2itk(vtkp,p);
              displayGeometry->Project(p, projected_p);
              Vector3D diff=p-projected_p;
              if(diff.GetSquaredNorm()<4.0)
              {
                displayGeometry->Map(projected_p, max2D);
                displayGeometry->WorldToDisplay(max2D, max2D);

                //draw the BoundingBox
                glColor3f(selectedColor[0],selectedColor[1],selectedColor[2]);//red
                //a line from lastPoint to firstPoint
                glBegin(GL_LINE_LOOP);
                  glVertex2f(min2D[0], min2D[1]);
                  glVertex2f(min2D[0], max2D[1]);
                  glVertex2f(max2D[0], max2D[1]);
                  glVertex2f(max2D[0], min2D[1]);
                glEnd();
              }//draw bounding-box
            }//bounding-box exists
          }//cell selected
        }//show bounding-box

        //fill off-plane polygon part 2
        if(worldplanegeometry!=NULL)
        {
          //consider line from last to first
          line.SetPoints(thisPoint, firstOfCell3D);
          if(worldplanegeometry->IntersectionPointParam(line, t) &&
            ((t>=0) && (t<=1))
            )
          {
            intersectionPoints.push_back(line.GetPoint(t));
          }
          std::sort(intersectionPoints.begin(), intersectionPoints.end(), point3DSmaller);
          std::vector<mitk::Point3D>::iterator it, end;
          end=intersectionPoints.end();
          if((intersectionPoints.size()%2)!=0)
          {
            --end; //ensure even number of intersection-points 
          }
          float p[2];
          Point3D pt3d;
          Point2D pt2d;
          for ( it = intersectionPoints.begin( ); it != end; ++it )
          {
            glBegin (GL_LINES);
              displayGeometry->Map(*it, pt2d); displayGeometry->WorldToDisplay(pt2d, pt2d); 
              p[0] = pt2d[0]; p[1] = pt2d[1]; glVertex2fv(p); 
              ++it;
              displayGeometry->Map(*it, pt2d); displayGeometry->WorldToDisplay(pt2d, pt2d);
              p[0] = pt2d[0]; p[1] = pt2d[1]; glVertex2fv(p); 
            glEnd ();
          }
          if(it!=intersectionPoints.end())
          {
            glBegin (GL_LINES);
              displayGeometry->Map(*it, pt2d); displayGeometry->WorldToDisplay(pt2d, pt2d); 
              p[0] = pt2d[0]; p[1] = pt2d[1]; glVertex2fv(p); 
              p[0] = pt2d[0]; p[1] = pt2d[1]; glVertex2fv(p); 
            glEnd ();
          }
        }//fill off-plane polygon part 2
      }//if numOfPointsInCell>1
      delete firstOfCell;
      delete lastPoint;
      lastPoint = NULL;
      firstOfCell = NULL;
      lastPointId = 0;
      ++cellIt;
      ++cellDataIt;
    }
  }
}
