#include "mitkPointSetMapper2D.h"
#include "mitkPointSet.h"
#include "BaseRenderer.h"
#include "PlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkFloatProperty.h"
#include "mitkStringProperty.h"
#include <vtkTransform.h>


#ifdef WIN32
	#include <glut.h>
#else
	#include <GL/glut.h>
#endif
	

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
        PointSet::PointSetType::PointsContainer::ConstIterator it, end;
        //bool list for the selection of the points
        PointSet::BoolListConstIter selIt, selEnd;

        it=input->GetPointList()->GetPoints()->Begin();
        selIt=input->GetSelectList().begin();
        end=input->GetPointList()->GetPoints()->End();
        selEnd=input->GetSelectList().end();

        int j=0;
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
                                
                if (*selIt)//selected
                {
                    horz.x=10;
                    vert.y=10;
                }

								// now paint text if available
                glRasterPos2f ( pt2d.x + 5, pt2d.y + 5);
								if (dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer()) == NULL)
								{}
								else {

										const char * pointLabel =dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer())->GetString();
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

		          
								glBegin (GL_LINE_LOOP);
                    tmp=pt2d-horz;      glVertex2fv(&tmp.x);
                    tmp=pt2d+horz;      glVertex2fv(&tmp.x);
                    tmp=pt2d;			glVertex2fv(&tmp.x);
                    tmp=pt2d-vert;      glVertex2fv(&tmp.x);
                    tmp=pt2d+vert;      glVertex2fv(&tmp.x);
                    tmp=pt2d;			glVertex2fv(&tmp.x);
                glEnd ();

                
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



