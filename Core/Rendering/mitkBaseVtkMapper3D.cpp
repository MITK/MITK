#include "BaseVtkMapper3D.h"
#include "DataTreeNode.h"
#include <vtkProp3D.h>
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
