#include "mitkImageMapper2D.h"
//#include "mitkRenderWindow.h"
#include "widget.h"
#include "picimage.h"
#include "pic2vtk.h"
#include "PlaneGeometry.h"
#include "BaseRenderer.h"
#include "DataTreeNode.h"

#include "mitkLookupTableProperty.h"
#include "mitkBoolProperty.h"
#include "mitkLevelWindowProperty.h"

#include "mitkRenderWindow.h"
#include "mitkAbstractTransformGeometry.h"

#include <vtkImageReslice.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>

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
  //@FIXME: durch die folgende Zeile sollte doch wohl der desctructor von RendererInfo aufgerufen werden. Das passiert aber nie. Deshalb wird bei der Programm-Beendung auch das iilImage und damit die textur nicht rechtzeitig freigegeben und das Programm crashed.
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
  iilPicImage*& image = renderinfo.m_iilImage;

  Update(renderer);

  const mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();

  Vector2D oldtopLeft=displayGeometry->GetOriginInUnits();
  Vector2D oldbottomRight=displayGeometry->GetOriginInUnits()+displayGeometry->GetSizeInUnits();

  Vector2D topLeft;
  Vector2D bottomRight;
  topLeft=displayGeometry->GetOriginInDisplayUnits();
  bottomRight=topLeft+displayGeometry->GetSizeInDisplayUnits();

  displayGeometry->DisplayToMM(topLeft, topLeft); topLeft.x*=renderinfo.m_PixelsPerMM.x;  topLeft.y*=renderinfo.m_PixelsPerMM.y;
  displayGeometry->DisplayToMM(bottomRight, bottomRight); bottomRight.x*=renderinfo.m_PixelsPerMM.x;  bottomRight.y*=renderinfo.m_PixelsPerMM.y;

  //test - small differences noticed for unisotropic datasets.
  if((Vector2D(oldtopLeft-topLeft).length()>0.1) || (Vector2D(oldbottomRight-bottomRight).length()>0.1))
  {
    bottomRight*=1.0;
  }

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
    vtkImageData* inputData = input->GetVtkImageData();
    const PlaneView* planeview=NULL;

    const Geometry2D* worldgeometry = renderer->GetWorldGeometry();

    if(dynamic_cast<const PlaneGeometry *>(worldgeometry)!=NULL)
    {
      planeview=&dynamic_cast<const PlaneGeometry *>(worldgeometry)->GetPlaneView();
      m_Reslicer->SetResliceTransform(NULL);
    }
    else
      if(dynamic_cast<const AbstractTransformGeometry *>(worldgeometry)!=NULL)
      {
        const AbstractTransformGeometry *abstractGeometry=dynamic_cast<const AbstractTransformGeometry *>(worldgeometry);
        planeview=&abstractGeometry->GetPlaneView();
        m_Reslicer->SetResliceTransform(abstractGeometry->GetVtkAbstractTransform());
        //m_Reslicer->DebugOn();
      }
      else
        return;

    assert(planeview!=NULL);
    assert(planeview->normal.length()>0.1);

    vtkMatrix4x4* geometry = vtkMatrix4x4::New();
    geometry->Identity();

    m_Reslicer->SetInput(inputData);
    m_Reslicer->SetOutputDimensionality(2);
    m_Reslicer->SetOutputOrigin(0,0,0);

    m_Reslicer->SetBackgroundLevel(-1024);

    int width, height;
    width=worldgeometry->GetWidthInUnits();
    height=worldgeometry->GetHeightInUnits();
    renderinfo.m_PixelsPerMM.set(planeview->getLengthOfOrientation1()/width, planeview->getLengthOfOrientation2()/height);

    m_Reslicer->SetOutputSpacing(renderinfo.m_PixelsPerMM.x, renderinfo.m_PixelsPerMM.y, 1.0);
    m_Reslicer->SetOutputExtent(0.0, width-1, 0.0, height-1, 0.0, 1.0);

    double origin[3];
    origin[0]=planeview->point.x;
    origin[1]=planeview->point.y;
    origin[2]=planeview->point.z;

    m_Reslicer->SetResliceAxes(geometry);
    m_Reslicer->SetResliceAxesOrigin(origin);
    //        m_Reslicer->SetInterpolationModeToLinear();
    double cosines[9];
    Vector3f orient1 = planeview->getOrientation1();
    orient1.normalize();

    Vector3f orient2 = planeview->getOrientation2();
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
    cosines[6]= planeview->normal.x;
    cosines[7]= planeview->normal.y;
    cosines[8]= planeview->normal.z;
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
    assert(pic->dim == 2);

    renderinfo.m_Pic = pic;

    //std::cout << "Pic dimensions:" << pic->dim << std::endl;

    image = new iilPicImage(NULL, "ll", 512);

    ApplyProperties(renderer);
//   image->setImage(pic, iilImage::INTENSITY_ALPHA);
	  image->setImage(pic, m_iilMode);
    image->setInterpolation(true);
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
    Update(it->first);
}

void mitk::ImageMapper2D::ApplyProperties(mitk::BaseRenderer* renderer)
{
  RendererInfo& renderinfo=m_RendererInfo[renderer];
  iilPicImage*& image = renderinfo.m_iilImage;

  assert(image != NULL);

  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  // check for color prop and use it for rendering if it exists
  GetColor(rgba, renderer);
  // check for opacity prop and use it for rendering if it exists
  GetOpacity(rgba[3], renderer);

  mitk::LevelWindow levelWindow;
  // check for level window prop and use it for display if it exists
  GetLevelWindow(levelWindow, renderer);


  mitk::LookupTableProperty::Pointer LookupTable;
  LookupTable = dynamic_cast<mitk::LookupTableProperty*>(this->GetDataTreeNode()->GetProperty("LookupTable").GetPointer());
	if (LookupTable.IsNull() )
	{
		m_iilMode = iilImage::INTENSITY_ALPHA;
	  image->setColor(rgba[0], rgba[1], rgba[2], rgba[3]);
	}
	else {
		m_iilMode = iilImage::COLOR_ALPHA;
		image->setColors(LookupTable->GetLookupTable().GetRawLookupTable());
	}

  mitk::BoolProperty::Pointer binary;
  binary = dynamic_cast<mitk::BoolProperty*>(this->GetDataTreeNode()->GetProperty("binary").GetPointer());

  mitk::LevelWindowProperty::Pointer overwriteLevelWindow;
  overwriteLevelWindow = dynamic_cast<mitk::LevelWindowProperty*>(this->GetDataTreeNode()->GetProperty("levelWindow").GetPointer());

  if (binary.IsNotNull() )
  {
    image->setExtrema(0, 1);
  }
  else if (overwriteLevelWindow.IsNotNull() )
  {
    image->setExtrema(overwriteLevelWindow->GetLevelWindow().GetMin(), overwriteLevelWindow->GetLevelWindow().GetMax());
  }
  else
  {
  // set the properties
    image->setExtrema(levelWindow.GetMin(), levelWindow.GetMax());
  }
//  image->setColor(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void mitk::ImageMapper2D::Update(mitk::BaseRenderer* renderer)
{

  RendererInfo& renderinfo=m_RendererInfo[renderer];
  DataTreeNode* node=GetDataTreeNode();
  iilPicImage*& image = renderinfo.m_iilImage;

  if(
      (image == NULL) ||
      (renderinfo.m_RendererId < 0) ||
      (renderinfo.m_LastUpdateTime < node->GetMTime())
    )
    GenerateData(renderer);
  else
  if(
      (renderinfo.m_LastUpdateTime < renderer->GetWorldGeometry()->GetMTime())
      //&&
      //(renderinfo.m_LastUpdateTime < renderer->GetMTime())
    )
    GenerateData(renderer);
  else
  if(
      (renderinfo.m_LastUpdateTime < node->GetPropertyList()->GetMTime()) ||
      (renderinfo.m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime())
    )
  {
    ApplyProperties(renderer);
    // since we have checked that nothing important has changed, we can set m_LastUpdateTime
    // to the current time
    mitk::Image::Pointer output = this->GetOutput();
    output->Modified();
    renderinfo.m_LastUpdateTime=output->GetMTime();
  }
}
