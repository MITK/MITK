#include "mitkGeometry2DDataVtkMapper3D.h"

#include "PlaneGeometry.h"

#include "DataTree.h"
#include "ImageMapper2D.h"

#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkTexture.h"
#include "vtkPlaneSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkLookupTable.h"
//#include "vtkImageMapToWindowLevelColors";
#include "mitkColorProperty.h"
#include "mitkFloatProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkSmartPointerProperty.h"
#include <vtkActor.h>
#include <vtkImageData.h>

#include "pic2vtk.h"

//##ModelId=3E691E09038E
mitk::Geometry2DDataVtkMapper3D::Geometry2DDataVtkMapper3D() : m_DataTreeIterator(NULL), m_LastTextureUpdateTime(0)
{
    m_VtkPlaneSource = vtkPlaneSource::New();

    m_VtkPolyDataMapper = vtkPolyDataMapper::New();
        m_VtkPolyDataMapper->SetInput(m_VtkPlaneSource->GetOutput());
        m_VtkPolyDataMapper->ImmediateModeRenderingOn();

    m_Actor = vtkActor::New();
        m_Actor->SetMapper(m_VtkPolyDataMapper);

	m_Prop3D = m_Actor;
		m_Prop3D->Register(NULL);

    m_VtkLookupTable = vtkLookupTable::New();
        m_VtkLookupTable->SetTableRange (-1024, 4096);
        m_VtkLookupTable->SetSaturationRange (0, 0);
        m_VtkLookupTable->SetHueRange (0, 0);
        m_VtkLookupTable->SetValueRange (0, 1);
        m_VtkLookupTable->Build ();

    m_VtkTexture = vtkTexture::New();
        m_VtkTexture->InterpolateOn();
        m_VtkTexture->SetLookupTable(m_VtkLookupTable);
        m_VtkTexture->MapColorScalarsThroughLookupTableOn();

    m_SliceSelector = mitk::ImageSliceSelector::New();

    //    m_Actor->SetTexture(axialTexture);

    //  axialTexture->SetInput(axialSection->GetOutput());
    //  axialTexture->InterpolateOn();
    //  axialTexture->SetLookupTable (lut);
    //  axialTexture->MapColorScalarsThroughLookupTableOn();
    //m_VtkPlaneSource = vtkPlaneSource::New();
    //  m_VtkPlaneSource->SetXResolution(1);
    //  m_VtkPlaneSource->SetYResolution(1);
    //  m_VtkPlaneSource->SetOrigin(0.0, 0.0, slice);
    //  m_VtkPlaneSource->SetPoint1(xDim, 0.0, slice);
    //  m_VtkPlaneSource->SetPoint2(0.0, yDim, slice);
    //m_VtkPolyDataMapper = vtkPolyDataMapper::New();
    //  m_VtkPolyDataMapper->SetInput(m_VtkPlaneSource->GetOutput());
    //  m_VtkPolyDataMapper->ImmediateModeRenderingOn();
    //axial = vtkActor::New();
    //  axial->SetMapper(m_VtkPolyDataMapper);
    //  axial->SetTexture(axialTexture);

}

//##ModelId=3E691E090394
mitk::Geometry2DDataVtkMapper3D::~Geometry2DDataVtkMapper3D()
{
    delete m_DataTreeIterator;

    m_VtkPlaneSource->Delete();
    m_VtkPolyDataMapper->Delete();
    m_Actor->Delete();
    m_VtkLookupTable->Delete();
    m_VtkTexture->Delete();
}



//##ModelId=3E691E090380
const mitk::Geometry2DData *mitk::Geometry2DDataVtkMapper3D::GetInput()
{
    if (this->GetNumberOfInputs() < 1)
    {
        return 0;
    }

    return static_cast<const mitk::Geometry2DData * > ( GetData() );
}

//##ModelId=3E691E09038A
void mitk::Geometry2DDataVtkMapper3D::Update()
{
    mitk::Geometry2DData::Pointer input  = const_cast<mitk::Geometry2DData*>(this->GetInput());

    if(input!=NULL)
    {
        mitk::PlaneGeometry::ConstPointer planeGeometry = dynamic_cast<const PlaneGeometry *>(input->GetGeometry2D());

        if(planeGeometry!=NULL)
        {
            const PlaneView &plane=planeGeometry->GetPlaneView();
            Vector3D right, bottom;
            right=plane.point+plane.getOrientation1();
            bottom=plane.point+plane.getOrientation2();

            m_VtkPlaneSource->SetXResolution(1);
            m_VtkPlaneSource->SetYResolution(1);
            m_VtkPlaneSource->SetOrigin(plane.point.x, plane.point.y, plane.point.z);
            m_VtkPlaneSource->SetPoint1(right.x, right.y, right.z);
            m_VtkPlaneSource->SetPoint2(bottom.x, bottom.y, bottom.z);

            if(m_DataTreeIterator)
            {
                mitk::DataTreeIterator* it=m_DataTreeIterator->clone();
                while(it->hasNext())
                {
                    it->next();
                    mitk::Mapper::Pointer mapper = it->get()->GetMapper(1);
                    mitk::ImageMapper2D* imagemapper = dynamic_cast<ImageMapper2D*>(mapper.GetPointer());

                    if(imagemapper)
                    {
						mitk::SmartPointerProperty::Pointer rendererProp = dynamic_cast<mitk::SmartPointerProperty*>(GetDataTreeNode()->GetPropertyList()->GetProperty("renderer").GetPointer());
						if(rendererProp!=NULL)
						{
							mitk::BaseRenderer::Pointer renderer = dynamic_cast<mitk::BaseRenderer*>(rendererProp->GetSmartPointer().GetPointer());
							if(renderer!=NULL)
							{
								// check for level window prop and use it for display if it exists
								mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(it->get()->GetPropertyList()->GetProperty("levelwindow").GetPointer());
								if(levWinProp!=NULL)
								{
									m_VtkLookupTable->SetTableRange(levWinProp->GetLevelWindow().GetMin(),levWinProp->GetLevelWindow().GetMax());
								}

								imagemapper->GenerateAllData();
								const ImageMapper2D::RendererInfo* ri=imagemapper->GetRendererInfo(renderer);
								if((ri!=NULL) && (m_LastTextureUpdateTime<ri->m_LastUpdateTime))
								{
									ipPicDescriptor *p=ri->m_Pic;
									vtkImageData* vtkimage=Pic2vtk::convert(p);
									m_VtkTexture->SetInput(vtkimage); vtkimage->Delete(); vtkimage=NULL;
									m_Actor->SetTexture(m_VtkTexture);
									m_LastTextureUpdateTime=ri->m_LastUpdateTime;
								}
								break;
							}
						}
                    }
                }
            }
        }
        //query and set color
        const mitk::DataTreeNode* node=GetDataTreeNode();
        float rgba[4]={1.0f,1.0f,1.0f,1.0f};
        if(node!=NULL)
        {
            mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(node->GetPropertyList()->GetProperty("color").GetPointer());
            if(colorprop!=NULL)
                memcpy(rgba, colorprop->GetColor().GetDataPointer(), 3*sizeof(float));
            mitk::FloatProperty::Pointer opacityprop = dynamic_cast<mitk::FloatProperty*>(node->GetPropertyList()->GetProperty("opacity").GetPointer());
            if(opacityprop!=NULL)
                rgba[3]=opacityprop->GetValue();
        }
        m_Actor->GetProperty()->SetColor(rgba);
        m_Actor->GetProperty()->SetOpacity(rgba[3]);
    }
}

//##ModelId=3E691E090396
void mitk::Geometry2DDataVtkMapper3D::GenerateOutputInformation()
{
}

//##ModelId=3E691E09038C
void mitk::Geometry2DDataVtkMapper3D::GenerateData()
{
}

//##ModelId=3E6E874F0007
void mitk::Geometry2DDataVtkMapper3D::SetDataIteratorForTexture(mitk::DataTreeIterator* iterator)
{
    delete m_DataTreeIterator;
    m_DataTreeIterator = iterator->clone();
}

