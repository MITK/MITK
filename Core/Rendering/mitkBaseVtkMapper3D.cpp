#include "BaseVtkMapper3D.h"
#include "DataTreeNode.h"
#include <vtkProp3D.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkLinearTransform.h>

//##ModelId=3E3424950213
mitk::BaseVtkMapper3D::BaseVtkMapper3D() : m_SliceNr(0), m_TimeNr(0), m_ChannelNr(0), m_Prop3D(NULL)
{
}

//##ModelId=3E3424950231
mitk::BaseVtkMapper3D::~BaseVtkMapper3D()
{
	if(m_Prop3D)
		m_Prop3D->Delete();
}

vtkProp* mitk::BaseVtkMapper3D::GetProp()
{
	if(GetDataTreeNode()!=NULL)
		m_Prop3D->SetUserTransform(GetDataTreeNode()->GetVtkTransform());
	return m_Prop3D;
}

//##ModelId=3EF180C70065
void mitk::BaseVtkMapper3D::ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer)
{
    float rgba[4]={1.0f,1.0f,1.0f,1.0f};
    // check for color prop and use it for rendering if it exists
    GetColor(rgba, renderer);
    // check for opacity prop and use it for rendering if it exists
    GetOpacity(rgba[3], renderer);

    actor->GetProperty()->SetColor(rgba);
    actor->GetProperty()->SetOpacity(rgba[3]);
}



