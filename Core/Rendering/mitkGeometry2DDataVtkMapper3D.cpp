#include "mitkGeometry2DDataVtkMapper3D.h"

#include "PlaneGeometry.h"

#include "vtkActor.h";
#include "vtkTexture.h";
#include "vtkPlaneSource.h"
#include "vtkPolyDataMapper.h"
//#include "vtkImageMapToWindowLevelColors";

//##ModelId=3E32C9E5007A
vtkProp* mitk::Geometry2DDataVtkMapper3D::GetProp()
{
	return m_Actor;
}

//##ModelId=3E32CA6C01A1
mitk::Geometry2DDataVtkMapper3D::Geometry2DDataVtkMapper3D()
{
    m_Actor = vtkActor::New();
    m_VtkTexture = vtkTexture::New();
    m_VtkPlaneSource = vtkPlaneSource::New();
    m_VtkPolyDataMapper = vtkPolyDataMapper::New();

    m_VtkPolyDataMapper->SetInput(m_VtkPlaneSource->GetOutput());
    m_VtkPolyDataMapper->ImmediateModeRenderingOn();

    m_Actor->SetMapper(m_VtkPolyDataMapper);
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

//##ModelId=3E32CA6C01C9
mitk::Geometry2DDataVtkMapper3D::~Geometry2DDataVtkMapper3D()
{
}

//##ModelId=3E690E68028C
void mitk::Geometry2DDataVtkMapper3D::SetInput(const mitk::Geometry2DData *input)
{
	// Process object is not const-correct so the const_cast is required here
	this->ProcessObject::SetNthInput(0,
		const_cast< mitk::Geometry2DData * >( input ) );
}

//##ModelId=3E690E6A0163
const mitk::Geometry2DData *mitk::Geometry2DDataVtkMapper3D::GetInput(void)
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}

	return static_cast<const mitk::Geometry2DData * >
		(this->ProcessObject::GetInput(0) );
}

//##ModelId=3E690E6B0331
void mitk::Geometry2DDataVtkMapper3D::SetInput(const mitk::BaseData* data)
{
	SetInput(dynamic_cast<const mitk::Geometry2DData *>(data));
}

//##ModelId=3E690E6D03A2
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
        }
    }
}

//##ModelId=3E690E6F00D4
void mitk::Geometry2DDataVtkMapper3D::GenerateOutputInformation()
{
}

//##ModelId=3E690E7D021E
void mitk::Geometry2DDataVtkMapper3D::GenerateData()
{
}

