#include "mitkLineMapper2D.h"
#include "mitkPointSet.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkFloatProperty.h"
#include "mitkStringProperty.h"
#include <vtkTransform.h>


#ifdef WIN32
	#include <glut.h>
#else
	#include <GL/glut.h>
#endif
	
mitk::LineMapper2D::LineMapper2D()
: mitk::PointSetMapper2D()
{
 
}

mitk::LineMapper2D::~LineMapper2D()
{
}

const mitk::PointSet *mitk::LineMapper2D::GetInput(void)
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}
  return static_cast<const mitk::PointSet * > ( GetData() );
}

void mitk::LineMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  if(IsVisible(renderer)==false) 
    return;

  bool updateNeccesary = true;//!!!! @TODO !??!!

  if (updateNeccesary) 
  {
    mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());
    mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
    assert(displayGeometry.IsNotNull());

    //apply color and opacity read from the PropertyList
    ApplyProperties(renderer);

    vtkTransform* transform = GetDataTreeNode()->GetVtkTransform();

    //List of the Points
    PointSet::DataType::PointsContainerConstIterator it, end;
    it = input->GetPointSet()->GetPoints()->Begin();
    end = --input->GetPointSet()->GetPoints()->End();//the last before end, because lines from n to n+1

    //bool list for the selection of the points
    PointSet::DataType::PointDataContainerIterator selIt, selItNext;
    selIt=input->GetPointSet()->GetPointData()->Begin();
    
    int j=0;
    while(it!=end)
    {
      mitk::Point3D p, q, projected_p, projected_q;
      float vtkp[3];
      p.x= it->Value()[0];
      p.y= it->Value()[1];
      p.z= it->Value()[2];
      
      //next point n+1
      it++;
      q.x= it->Value()[0];
      q.y= it->Value()[1];
      q.z= it->Value()[2];
      it--;

      vec2vtk(p, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2vec(vtkp,p);

      vec2vtk(q, vtkp);
      transform->TransformPoint(vtkp, vtkp);
      vtk2vec(vtkp,q);


      displayGeometry->Project(p, projected_p);
      displayGeometry->Project(q, projected_q);

      if((Vector3D(p-projected_p).length()<2.0) &&
        (Vector3D(q-projected_q).length()<2.0))
      {
        Point2D p2d, q2d, tmp, horz(5,0), vert(0,5);
        
        displayGeometry->Map(projected_p, p2d);
        displayGeometry->MMToDisplay(p2d, p2d);
        displayGeometry->Map(projected_q, q2d);
        displayGeometry->MMToDisplay(q2d, q2d);

        //outlined, cause iterators behave strange and multiple points can't be selected anyway by now!
        //selItNext = selIt++;//Iterator to the next Element, cause only when the two points ar selected, then the Line is Selected
        //if (selIt->Value() && selItNext->Value())//selected
        //{
        //  float colorSel[]={1.0,0.0,0.6}; //for selected!

        //  //current color for changing to a diferent color if selected
        //  float currCol[4];
        //  glGetFloatv(GL_CURRENT_COLOR,currCol);

        //  glColor3f(colorSel[0],colorSel[1],colorSel[2]);//red


        //  glBegin (GL_LINES);
        //    glVertex2f(p2d.x, p2d.y);
        //    glVertex2f(q2d.x, q2d.y);
        //  glEnd ();

        //  glColor3f(currCol[0],currCol[1],currCol[2]);//the color before changing to select!

        //}
        //else
        //{
					glBegin (GL_LINES);
            glVertex2f(p2d.x, p2d.y);
            glVertex2f(q2d.x, q2d.y);
          glEnd ();
        //}
      }
      ++it;
      ++selIt;
 			j++;
    }
    //drawing the points
    mitk::PointSetMapper2D::Paint(renderer);
  }
}

void mitk::LineMapper2D::Update()
{
}

void mitk::LineMapper2D::GenerateOutputInformation()
{
}
