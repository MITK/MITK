#include "mitkPlaneCutFilter.h"
#include "ipPicTypeMultiplex.h"
#include "mitkVector.h"

template <class T>
void _planesSet(ipPicDescriptor *dest, const mitk::PlaneView *planes, T outside_value, T * dummy=0)
{
	T *d=(T*)dest->data;

    const mitk::PlaneView **pl=&planes;
//    Plane3dList::iterator pl = planes->begin();
//    Plane3dList::iterator end = planes->end();

	int x_max(dest->n[0]),y_max(dest->n[1]), z_max(dest->n[2]), xy3_size(dest->n[0]*dest->n[1]);
    mitk::Vector3D p;
	for(p.z=0;p.z<z_max;++p.z)
		for(p.y=0;p.y<y_max;++p.y)
			for(p.x=0;p.x<x_max;++p.x,++d)
//				for(pl=planes->begin();pl!=end;++pl)
				{
					if((*pl)->signedDistance(p)<0)
						*d=outside_value;
				}
}

void mitk::PlaneCutFilter::GenerateData()
{
	mitk::Image::ConstPointer input = this->GetInput();
    mitk::Image::Pointer output = this->GetOutput();

	ipPicDescriptor *pic_result;

    pic_result=ipPicClone(const_cast<mitk::Image*>(input.GetPointer())->GetPic());

	if(m_Planes!=NULL)
    {
		ipPicTypeMultiplex2(_planesSet, pic_result, m_Planes, m_BackgroundLevel);
    }

    output->Initialize(pic_result);
    output->SetPicVolume(pic_result);
}

mitk::PlaneCutFilter::PlaneCutFilter() : m_BackgroundLevel(0), m_Planes(NULL)
{
}

mitk::PlaneCutFilter::~PlaneCutFilter()
{
}


