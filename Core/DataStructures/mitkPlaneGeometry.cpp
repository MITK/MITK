#include "PlaneGeometry.h"

//##ModelId=3E395F22035A
mitk::PlaneGeometry::PlaneGeometry()
{
}


//##ModelId=3E395F220382
mitk::PlaneGeometry::~PlaneGeometry()
{
}

//##ModelId=3E395E3E0077
const mitk::PlaneView& mitk::PlaneGeometry::GetPlaneView()
{
    return m_PlaneView;
}

//##ModelId=3E396ABE0385
void mitk::PlaneGeometry::SetPlaneView(const mitk::PlaneView& aPlaneView)
{
    m_PlaneView=aPlaneView;
}

