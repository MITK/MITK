#include "mitkMeshMapper2D.h"
#include "mitkMesh.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include <vtkTransform.h>
#include <GL/glut.h>
#include <mitkProperties.h>


//##Documentation
//##@brief color to mark a selected point
const float selectedColor[]={1.0,0.0,0.6}; //for selected!

mitk::MeshMapper2D::MeshMapper2D()
{
}


mitk::MeshMapper2D::~MeshMapper2D()
{
}

const mitk::Mesh *mitk::MeshMapper2D::GetInput(void)
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}

    return static_cast<const mitk::Mesh * > ( GetData() );
}



void mitk::MeshMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  if(IsVisible(renderer)==false) return;

  //	@FIXME: Logik fuer update
  bool updateNeccesary=true;

  if (updateNeccesary) 
  {
  //aus GenerateData    
    mitk::Mesh::Pointer input  = const_cast<mitk::Mesh*>(this->GetInput());
    mitk::Mesh::MeshType::Pointer itkMesh = input->GetMesh();
    mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
    assert(displayGeometry.IsNotNull());

    //apply color and opacity read from the PropertyList
    ApplyProperties(renderer);

    vtkTransform* transform = GetDataTreeNode()->GetVtkTransform();

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
        displayGeometry->MMToDisplay(pt2d, pt2d);

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
                tmp=pt2d+horz;			glVertex2fv(&tmp[0]);
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
    //iterate through all cells and then iterate through all indexes of points in that cell
    Mesh::CellIterator cellIt, cellEnd;
    Mesh::CellDataIterator cellDataIt;//, cellDataEnd;
    Mesh::PointIdIterator cellIdIt, cellIdEnd;

    cellIt = itkMesh->GetCells()->Begin();
    cellEnd = itkMesh->GetCells()->End();
    cellDataIt = itkMesh->GetCellData()->Begin();

    mitk::Mesh::PointType thisPoint;
    Point2D *firstOfCell = NULL;
    Point2D *lastPoint = NULL;
    unsigned int lastPointId = 0;
    bool lineSelected = false;

    while (cellIt != cellEnd)
    {
      unsigned int numOfPointsInCell = cellIt->Value()->GetNumberOfPoints();
      if (numOfPointsInCell>1)
      {
        //iterate through all id's in the cell
        cellIdIt = cellIt->Value()->PointIdsBegin();
        cellIdEnd = cellIt->Value()->PointIdsEnd();

        while(cellIdIt != cellIdEnd)
        {
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
            displayGeometry->MMToDisplay(pt2d, pt2d);

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
          ++cellIdIt;
        }//while cellIdIter

        //close the polygon if needed
        if ( cellDataIt->Value().closed && 
          firstOfCell != NULL )
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
        }//if closed
        
        //Axis-aligned bounding box(AABB) around the cell if selected and set in Property
        bool showBoundingBox;
        if (dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("showBoundingBox").GetPointer()) == NULL)
		  		showBoundingBox = false;
			  else
				  showBoundingBox = dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("showBoundingBox").GetPointer())->GetValue();

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
              displayGeometry->MMToDisplay(min2D, min2D);

              itk2vtk(max, vtkp);
              transform->TransformPoint(vtkp, vtkp);
              vtk2itk(vtkp,p);
              displayGeometry->Project(p, projected_p);
              displayGeometry->Map(projected_p, max2D);
              displayGeometry->MMToDisplay(max2D, max2D);

              //draw the BoundingBox
              glColor3f(selectedColor[0],selectedColor[1],selectedColor[2]);//red
              //a line from lastPoint to firstPoint
              glBegin(GL_LINE_LOOP);
                glVertex2f(min2D[0], min2D[1]);
                glVertex2f(min2D[0], max2D[1]);
                glVertex2f(max2D[0], max2D[1]);
                glVertex2f(max2D[0], min2D[1]);
              glEnd();
            }
          }
        }
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

void mitk::MeshMapper2D::Update()
{
}

void mitk::MeshMapper2D::GenerateOutputInformation()
{
}
