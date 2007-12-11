/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlaneCutFilter.h"
#include "ipPic/ipPicTypeMultiplex.h"
#include "mitkVector.h"

template <class T>
void _planesSet(ipPicDescriptor *dest, const mitk::Geometry3D* srcgeometry, const mitk::PlaneCutFilter::PlanesContainerType* planes, float _outside_value)
{
  T outside_value = static_cast<T>(_outside_value);
  T *d=(T*)dest->data;

  mitk::PlaneCutFilter::PlanesContainerType::ConstIterator pit, pend;
  pend = planes->End();

  int x_max(dest->n[0]),y_max(dest->n[1]), z_max(dest->n[2]);
  mitk::Point3D p;
  mitk::Point3D pt_mm;
  for(p[2]=0;p[2]<z_max;++p[2])
    for(p[1]=0;p[1]<y_max;++p[1])
      for(p[0]=0;p[0]<x_max;++p[0],++d)
        for(pit=planes->Begin();pit!=pend;++pit)
        {
          srcgeometry->IndexToWorld(p, pt_mm);
          if(pit.Value()->SignedDistance(pt_mm) < 0)
            *d=outside_value;
        }
}

void mitk::PlaneCutFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  ipPicDescriptor *pic_result;

  pic_result=ipPicClone(const_cast<mitk::Image*>(input.GetPointer())->GetPic());
  pic_result->dim=3;

  if(m_Planes.IsNotNull())
  {
    ipPicTypeMultiplex3(_planesSet, pic_result, input->GetGeometry(), m_Planes, m_BackgroundLevel);
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


