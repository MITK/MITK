#include "mitkPointSetMapper2D.h"
#include "mitkPointSet.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include <vtkTransform.h>
#include "mitkStringProperty.h"

#include <GL/glut.h>
	
//##ModelId=3F0189F00378
mitk::PointSetMapper2D::PointSetMapper2D()
{
}

//##ModelId=3F0189F00382
mitk::PointSetMapper2D::~PointSetMapper2D()
{
}

//##ModelId=3F0189F00366
const mitk::PointSet *mitk::PointSetMapper2D::GetInput(void)
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}

    return static_cast<const mitk::PointSet * > ( GetData() );
}


//##ModelId=3F0189F00373
void mitk::PointSetMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  if(IsVisible(renderer)==false) return;

  //	@FIXME: Logik fuer update
  bool updateNeccesary=true;

  if (updateNeccesary) {
      // ok, das ist aus GenerateData kopiert

      mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());

      mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();

      assert(displayGeometry.IsNotNull());

      //apply color and opacity read from the PropertyList
      ApplyProperties(renderer);

      vtkTransform* transform = GetDataTreeNode()->GetVtkTransform();

      //List of the Points
      PointSet::DataType::PointsContainerConstIterator it, end;
      it=input->GetPointSet()->GetPoints()->Begin();
      end=input->GetPointSet()->GetPoints()->End();

      //bool list for the selection of the points
      PointSet::DataType::PointDataContainerIterator selIt, selEnd;
      selIt=input->GetPointSet()->GetPointData()->Begin();
      selEnd=input->GetPointSet()->GetPointData()->End();
      
      int j=0;
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
              horz[0]=8; horz[1]=0;
              vert[0]=0; vert[1]=8;
                              
							// now paint text if available
              glRasterPos2f ( pt2d[0] + 5, pt2d[1] + 5);
							if (dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer()) == NULL)
							{}
							else {

									const char * pointLabel =dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer())->GetValue();
									char buffer[20];
						 	    std::string l = pointLabel;
									if (input->GetSize()>1)
									{
											sprintf(buffer,"%d",j+1);
											l.append(buffer);
									}
	
    		          for (unsigned int i = 0; i < l.size(); i++)
		    		          glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, l[i]);
		        	}

              if (selIt->Value())//selected
              {
                float colorSel[]={1.0,0.0,0.6}; //for selected!

                //current color for changing to a diferent color if selected
                float currCol[4];
                glGetFloatv(GL_CURRENT_COLOR,currCol);

                glColor3f(colorSel[0],colorSel[1],colorSel[2]);//red

                //a diamond around the point
                glBegin (GL_LINE_LOOP);
                    tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
                    tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
                    tmp=pt2d+horz;			glVertex2fv(&tmp[0]);
                    tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
                glEnd ();

                //the actual point
                glBegin (GL_POINTS);
                  tmp=pt2d;             glVertex2fv(&tmp[0]);
                glEnd ();
                
                
                glColor3f(currCol[0],currCol[1],currCol[2]);//the color before changing to select!

              }
              else
              {
								glBegin (GL_LINES);
                    tmp=pt2d-horz;      glVertex2fv(&tmp[0]);
                    tmp=pt2d+horz;      glVertex2fv(&tmp[0]);
                    tmp=pt2d-vert;      glVertex2fv(&tmp[0]);
                    tmp=pt2d+vert;      glVertex2fv(&tmp[0]);
                glEnd ();
              }
          }
          ++it;
          ++selIt;
 					j++;
      }
  }
}

//##ModelId=3F0189F00376
void mitk::PointSetMapper2D::Update()
{
}

//##ModelId=3F0189F00384
void mitk::PointSetMapper2D::GenerateOutputInformation()
{
}



