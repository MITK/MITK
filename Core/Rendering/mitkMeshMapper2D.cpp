#include "mitkMeshMapper2D.h"
#include "mitkMesh.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include <vtkTransform.h>


#ifdef WIN32
	#include <glut.h>
#else
	#include <GL/glut.h>
#endif

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
//    dataEnd=itkMesh->GetPointData()->End();
      
    //for switching back to old color after using selected color
    float unselectedColor[4];
    glGetFloatv(GL_CURRENT_COLOR,unselectedColor);

    while(it!=end)
    {
      mitk::Point3D p, projected_p;
      float vtkp[3];
      p.x= it->Value()[0];
      p.y= it->Value()[1];
      p.z= it->Value()[2];
      vec2vtk(p, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2vec(vtkp,p);

      displayGeometry->Project(p, projected_p);
      if(Vector3D(p-projected_p).length()<2.0)
      {
        Point2D pt2d, tmp;
        displayGeometry->Map(projected_p, pt2d);
        displayGeometry->MMToDisplay(pt2d, pt2d);

        Point2D horz(5,0),vert(0,5);
                        
        //check if the point is to be marked as selected 
        if (dataIt->Value().selected)
        {
          horz.x=8;
          vert.y=8;
          glColor3f(selectedColor[0],selectedColor[1],selectedColor[2]);//red

          //a diamond around the point
          glBegin (GL_LINE_LOOP);
            tmp=pt2d-horz;      glVertex2fv(&tmp.x);
            tmp=pt2d+vert;      glVertex2fv(&tmp.x);
            tmp=pt2d+horz;			glVertex2fv(&tmp.x);
            tmp=pt2d-vert;      glVertex2fv(&tmp.x);
          glEnd ();

          //the actual point
          glBegin (GL_POINTS);
            tmp=pt2d;             glVertex2fv(&tmp.x);
          glEnd ();
        }
        else //if not selected
        {
          glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
					//drawing crosses
          glBegin (GL_LINES);
              tmp=pt2d-horz;      glVertex2fv(&tmp.x);
              tmp=pt2d+horz;      glVertex2fv(&tmp.x);
              tmp=pt2d-vert;      glVertex2fv(&tmp.x);
              tmp=pt2d+vert;      glVertex2fv(&tmp.x);
          glEnd ();
        }
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
          p.x = thisPoint[0];
          p.y = thisPoint[1];
          p.z = thisPoint[2];
          vec2vtk(p, vtkp);
          transform->TransformPoint(vtkp, vtkp);
          vtk2vec(vtkp,p);
          displayGeometry->Project(p, projected_p);

          if(Vector3D(p-projected_p).length()<2.0)
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
                  glVertex2fv(&(*lastPoint).x);
                  glVertex2fv(&pt2d.x);
                glEnd ();
              }
              else //if not selected
              {
                glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
						    //drawing crosses
                glBegin (GL_LINES);
                  glVertex2fv(&(*lastPoint).x);
                  glVertex2fv(&pt2d.x);
                glEnd ();
              }
              //to draw the line to the next in iteration step
              *lastPoint = pt2d;
              //and to search for the selection state of the line
              lastPointId = *cellIdIt;
            }//if..else
          }//if <2.0
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
              glVertex2fv(&(*lastPoint).x);
              glVertex2fv(&(*firstOfCell).x);
            glEnd ();
          }
          else 
          {
            glColor3f(unselectedColor[0],unselectedColor[1],unselectedColor[2]);
            glBegin (GL_LINES);
              glVertex2fv(&(*lastPoint).x);
              glVertex2fv(&(*firstOfCell).x);
            glEnd ();
          }
        }//if closed
        
        //Axis-aligned bounding boxe(AABB) around the cell if selected
        if (cellDataIt->Value().selected)
        {
          mitk::PointSet::DataType::PointsContainerPointer pointsContainer = mitk::PointSet::DataType::PointsContainer::New();
          Mesh::PointIdIterator bbIt = cellIt->Value()->PointIdsBegin();
          Mesh::PointIdIterator bbEnd = cellIt->Value()->PointIdsEnd();
          while(bbIt != bbEnd)
          {
            mitk::PointSet::PointType point;
            bool pointOk = itkMesh->GetPoint((*bbIt), &point);
            if (pointOk)
              pointsContainer->SetElement((*bbIt), point);
            ++bbIt;
          }

          mitk::PointSet::DataType::BoundingBoxPointer aABB = mitk::PointSet::DataType::BoundingBoxType::New();
          aABB->SetPoints(pointsContainer);
          bool bBOk = aABB->ComputeBoundingBox();
          if (bBOk)
          {
            mitk::PointSet::PointType min, max;
            min = aABB->GetMinimum();
            max = aABB->GetMaximum();

            //project to the displayed geometry
            Point2D min2D, max2D;
            Point3D p, projected_p;
            float vtkp[3];
            vtkp[0] = min[0];
            vtkp[1] = min[1];
            vtkp[2] = min[2];
            transform->TransformPoint(vtkp, vtkp);
            vtk2vec(vtkp,p);
            displayGeometry->Project(p, projected_p);
            displayGeometry->Map(projected_p, min2D);
            displayGeometry->MMToDisplay(min2D, min2D);

            vtkp[0] = max[0];
            vtkp[1] = max[1];
            vtkp[2] = max[2];
            transform->TransformPoint(vtkp, vtkp);
            vtk2vec(vtkp,p);
            displayGeometry->Project(p, projected_p);
            displayGeometry->Map(projected_p, max2D);
            displayGeometry->MMToDisplay(max2D, max2D);

            //draw the BoundingBox
            glColor3f(selectedColor[0],selectedColor[1],selectedColor[2]);//red
            //a line from lastPoint to firstPoint
            glBegin(GL_LINE_LOOP);
              glVertex2f(min2D.x, min2D.y);
              glVertex2f(min2D.x, max2D.y);
              glVertex2f(max2D.x, max2D.y);
              glVertex2f(max2D.x, min2D.y);
            glEnd();
          }
        }
      }//if numOfPointsInCell>1
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
