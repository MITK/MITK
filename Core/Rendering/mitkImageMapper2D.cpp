#include "mitkImageMapper2D.h"
//#include "mitkRenderWindow.h"
#include "widget.h"
#include "picimage.h"
#include "pic2vtk.h"
#include "PlaneGeometry.h"
#include "BaseRenderer.h"
#include "DataTreeNode.h"
#include "mitkRenderWindow.h"

#include <vtkImageReslice.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

int mitk::ImageMapper2D::numRenderer = 0;

mitk::ImageMapper2D::ImageMapper2D() : m_SliceSelector(NULL)
{
	// Modify superclass default values, can be overridden by subclasses
	this->SetNumberOfRequiredInputs(1);
	
    m_SliceSelector = ImageSliceSelector::New();
    m_Reslicer = vtkImageReslice::New();
	
}

//##ModelId=3E32DCF60043
mitk::ImageMapper2D::~ImageMapper2D()
{
    //FIXME: durch die folgende Zeile sollte doch wohl der desctructor von RendererInfo aufgerufen werden. Das passiert aber nie. Deshalb wird bei der Programm-Beendung auch das iilImage und damit die textur nicht rechtzeitig freigegeben und das Programm crashed.
    m_RendererInfo.clear(); 
}

//##ModelId=3E3D834B003A
void mitk::ImageMapper2D::GenerateData()
{

}

void mitk::ImageMapper2D::Paint(mitk::BaseRenderer * renderer)
{
    if(IsVisible(renderer)==false) return;

	RendererInfo& renderinfo=m_RendererInfo[renderer];
	DataTreeNode* node=GetDataTreeNode();

    if( (renderinfo.m_LastUpdateTime < renderer->GetMTime()) || 
		(renderinfo.m_LastUpdateTime < node->GetMTime()) ||
		(renderinfo.m_LastUpdateTime < node->GetPropertyList()->GetMTime())
	  )
		GenerateData(renderer);
	
	iilPicImage*& image = renderinfo.m_iilImage;

	const mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();
	
	Point2D topLeft=displayGeometry->GetOriginInUnits();
	Point2D bottomRight=displayGeometry->GetOriginInUnits()+displayGeometry->GetSizeInUnits();
	
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
    gluOrtho2D(topLeft.x, bottomRight.x, topLeft.y, bottomRight.y );
    glMatrixMode( GL_MODELVIEW );
	
	GLdouble eqn0[4] = {0.0, 1.0, 0.0, 0.0};	
	GLdouble eqn1[4] = {1.0, 0.0, 0.0, 0.0};
	GLdouble eqn2[4] = {-1.0, 0.0 , 0.0, image->width()};	
	GLdouble eqn3[4] = {0, -1.0, 0.0, image->height() };

	glClipPlane (GL_CLIP_PLANE0, eqn0);
	glEnable (GL_CLIP_PLANE0);
	glClipPlane (GL_CLIP_PLANE1, eqn1);
	glEnable (GL_CLIP_PLANE1);
	glClipPlane (GL_CLIP_PLANE2, eqn2);
	glEnable (GL_CLIP_PLANE2);
	glClipPlane (GL_CLIP_PLANE3, eqn3);
	glEnable (GL_CLIP_PLANE3);
 

	image->display(renderer->GetRenderWindow());


	glDisable (GL_CLIP_PLANE0);
	glDisable (GL_CLIP_PLANE1);
	glDisable (GL_CLIP_PLANE2);
	glDisable (GL_CLIP_PLANE3);
	
	glPushMatrix ();
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D(0, displayGeometry->GetDisplayWidth(), 0, displayGeometry->GetDisplayHeight() );
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix ();
}

//##ModelId=3E3D834B0008
const mitk::ImageMapper2D::InputImageType *mitk::ImageMapper2D::GetInput(void)
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}
	
	return static_cast<const mitk::ImageMapper2D::InputImageType * >
        ( GetData() );
}

//##ModelId=3E6E83B00343
int mitk::ImageMapper2D::GetAssociatedChannelNr(mitk::BaseRenderer *renderer)
{
	RendererInfo& renderinfo=m_RendererInfo[renderer];
    if(renderinfo.m_RendererId < 0)
        renderinfo.m_RendererId = ImageMapper2D::numRenderer++;
	
    return renderinfo.m_RendererId;
}

//##ModelId=3E8607D20380
void mitk::ImageMapper2D::GenerateOutputInformation()
{
   	mitk::Image::Pointer output = this->GetOutput();
	mitk::PixelType pt(typeid(int));
	unsigned int dim[]={256,256};
	output->Initialize(mitk::PixelType(typeid(short int)), 2, dim, 10);
}

//##ModelId=3ED932B00140
void mitk::ImageMapper2D::GenerateData(mitk::BaseRenderer *renderer)
{
	RendererInfo& renderinfo=m_RendererInfo[renderer];

    iilPicImage*& image = renderinfo.m_iilImage;

    mitk::Image::Pointer input  = const_cast<mitk::ImageMapper2D::InputImageType *>(this->GetInput());
	
    if(image!= NULL)
    {
        delete image;
        image = NULL;
    }
	
    if(renderinfo.m_Pic)
    {
        ipPicFree(renderinfo.m_Pic);
        renderinfo.m_Pic = NULL;
    }
	
    if(renderinfo.m_RendererId < 0)
        renderinfo.m_RendererId = ImageMapper2D::numRenderer++;
	
    if(input.IsNotNull())
    {
        // #define DEBUG
		
//        input->Update();
        vtkImageData* inputData = input->GetVtkImageData();
		
        vtkMatrix4x4* geometry = vtkMatrix4x4::New();
        geometry->Identity();
		
        vtkTransform* transform = vtkTransform::New();
        transform->Identity();
		
        m_Reslicer->SetInput(inputData);
        m_Reslicer->SetOutputDimensionality(2);
 	m_Reslicer->SetOutputOrigin(0,0,0);

        m_Reslicer->SetBackgroundLevel(-1024);
        PlaneGeometry* myPlaneGeom =
            dynamic_cast<PlaneGeometry *>((mitk::Geometry2D*)(renderer->GetWorldGeometry()));
        assert(myPlaneGeom);
        assert(myPlaneGeom->GetPlaneView().normal.length()>0.1);

    	m_Reslicer->SetOutputSpacing(myPlaneGeom->GetPlaneView().getLengthOfOrientation1()/myPlaneGeom->GetWidthInUnits(), myPlaneGeom->GetPlaneView().getLengthOfOrientation2()/myPlaneGeom->GetHeightInUnits(),1.0);
        m_Reslicer->SetOutputExtent(0.0, myPlaneGeom->GetWidthInUnits()-1, 0.0, myPlaneGeom->GetHeightInUnits()-1, 0.0, 1.0);

        double origin[3];
        origin[0]=myPlaneGeom->GetPlaneView().point.x;
        origin[1]=myPlaneGeom->GetPlaneView().point.y;
        origin[2]=myPlaneGeom->GetPlaneView().point.z;
		
        m_Reslicer->SetResliceAxes(geometry);
        m_Reslicer->SetResliceAxesOrigin(origin);
//        m_Reslicer->SetInterpolationModeToLinear();
        double cosines[9];
        Vector3f orient1 = myPlaneGeom->GetPlaneView().getOrientation1();
        orient1.normalize();
		
        Vector3f orient2 = myPlaneGeom->GetPlaneView().getOrientation2();
        orient2.normalize();
        
	// Richtung der X-Achse der Ergebnisschicht im Volumen,
        cosines[0]=orient1.x;
        cosines[1]=orient1.y;
        cosines[2]=orient1.z;
        
	//Richtung der Y-Achse der Ergebnisschicht im Volumen
        cosines[3]=orient2.x;
        cosines[4]=orient2.y;
        cosines[5]=orient2.z;
	
	// Schichtfolge/Projektionsrichtung
        cosines[6]= myPlaneGeom->GetPlaneView().normal.x;
        cosines[7]= myPlaneGeom->GetPlaneView().normal.y;
        cosines[8]= myPlaneGeom->GetPlaneView().normal.z;
        m_Reslicer->SetResliceAxesDirectionCosines(cosines);
		
        m_Reslicer->Update();
		
		vtkImageData* vtkoutput = m_Reslicer->GetOutput();
		
        assert(vtkoutput);
		
		//	std::cout << vtkoutput <<std::endl;
        ipPicDescriptor* pic = Pic2vtk::convert(vtkoutput);
        assert(pic);
        if(pic->dim==1)
        {
            pic->dim=2;
            pic->n[1]=1;
        }
		
        renderinfo.m_Pic = pic;
		
        //std::cout << "Pic dimensions:" << pic->dim << std::endl;
		
        image = new iilPicImage(NULL, "ll", 512);
		
		image->setImage(pic, iilImage::INTENSITY_ALPHA);
        image->setInterpolation(true);
        assert(pic->dim == 2);

        float rgba[4]={1.0f,1.0f,1.0f,1.0f};
        // check for color prop and use it for rendering if it exists
        GetColor(rgba, renderer);
        // check for opacity prop and use it for rendering if it exists
        GetOpacity(rgba[3], renderer);
        // check for level window prop and use it for display if it exists
        mitk::LevelWindow levelWindow;
        GetLevelWindow(levelWindow, renderer);
		
		image->setColor(rgba[0], rgba[1], rgba[2], rgba[3]);
        image->setExtrema(levelWindow.GetMin(),levelWindow.GetMax());
		image->setRegion(0,0,pic->n[0],pic->n[1]);
		
		mitk::Image::Pointer output = this->GetOutput();
		
		//if(renderinfo.m_RendererId < 10)
		//	output->SetPicSlice(pic,0,0,renderinfo.m_RendererId);
		output->Modified();
		renderinfo.m_LastUpdateTime=output->GetMTime();
    }
	return;
}

void mitk::ImageMapper2D::GenerateAllData()
{
	std::map<mitk::BaseRenderer*,RendererInfo>::iterator it=m_RendererInfo.begin();
	for(;it!=m_RendererInfo.end();++it)
	{
		RendererInfo& renderinfo=it->second;
		DataTreeNode* node=GetDataTreeNode();
		if( (renderinfo.m_LastUpdateTime < it->first->GetMTime()) || 
			(renderinfo.m_LastUpdateTime < node->GetMTime()) ||
			(renderinfo.m_LastUpdateTime < node->GetPropertyList()->GetMTime())
		  )
			GenerateData(it->first);
	}
}
