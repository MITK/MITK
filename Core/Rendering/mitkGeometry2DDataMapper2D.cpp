#include "mitkGeometry2DDataMapper2D.h"
#include "BaseRenderer.h"
#include "PlaneGeometry.h"

//##ModelId=3E639E100243
mitk::Geometry2DDataMapper2D::Geometry2DDataMapper2D()
{
}

//##ModelId=3E639E100257
mitk::Geometry2DDataMapper2D::~Geometry2DDataMapper2D()
{
}

//##ModelId=3E6423D20337
void mitk::Geometry2DDataMapper2D::SetInput(const mitk::Geometry2DData *input)
{
	// Process object is not const-correct so the const_cast is required here
	this->ProcessObject::SetNthInput(0,
		const_cast< mitk::Geometry2DData * >( input ) );

}

//##ModelId=3E6423D20341
const mitk::Geometry2DData *mitk::Geometry2DDataMapper2D::GetInput(void)
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}

	return static_cast<const mitk::Geometry2DData * >
		(this->ProcessObject::GetInput(0) );
}

//##ModelId=3E6423D2034B
void mitk::Geometry2DDataMapper2D::SetInput(const mitk::BaseData* data)
{
	SetInput(dynamic_cast<const mitk::Geometry2DData *>(data));
}


//##ModelId=3E67D77A0109
void mitk::Geometry2DDataMapper2D::Paint(mitk::BaseRenderer * renderer)
{
    //	FIXME: Logik fuer update
    bool updateNeccesary=true;

    if (updateNeccesary) {
        // ok, das ist aus GenerateData kopiert

        mitk::Geometry2DData::Pointer input  = const_cast<mitk::Geometry2DData*>(this->GetInput());

        PlaneGeometry::ConstPointer planeGeometry = dynamic_cast<const PlaneGeometry *>(input->GetGeometry2D());
        PlaneGeometry::Pointer worldPlaneGeometry = dynamic_cast<PlaneGeometry*>(renderer->GetWorldGeometry());

        if((planeGeometry!=NULL) && (worldPlaneGeometry!=NULL))
        {
            mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
            assert(displayGeometry);

            //calculate intersection of the plane data with the border of the world geometry rectangle
	        Vector2D vlineFrom;
	        Vector2D vlineTo;
    		bool intersecting = worldPlaneGeometry->GetPlaneView().intersectPlane2D( planeGeometry->GetPlaneView(), vlineFrom, vlineTo );

	        Point2D lineFrom=vlineFrom;
	        Point2D lineTo=vlineTo;

            if(intersecting)
            {
                //convert intersection points (until now mm) to display coordinates (units )
                displayGeometry->MMToDisplay(lineFrom, lineFrom);
                displayGeometry->MMToDisplay(lineTo, lineTo);

                //convert display coordinates ( (0,0) is top-left ) in GL coordinates ( (0,0) is bottom-left )
                float toGL=displayGeometry->GetSizeInDisplayUnits().y;
                lineFrom.y=toGL-lineFrom.y;
                lineTo.y=toGL-lineTo.y;

                glBegin (GL_LINE_LOOP);
                    glVertex2fv(&lineFrom.x);
                    glVertex2fv(&lineTo.x);
                    glVertex2fv(&lineFrom.x);
                glEnd ();
            }
        }
    }
}

//##ModelId=3E67E1B90237
void mitk::Geometry2DDataMapper2D::Update()
{
}

//##ModelId=3E67E285024E
void mitk::Geometry2DDataMapper2D::GenerateOutputInformation()
{
}

