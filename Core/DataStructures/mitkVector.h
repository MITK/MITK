#ifndef MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD
#define MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD

#include "SpaceGeometry.h"
#include <itkPoint.h>

namespace mitk {
typedef double ScalarType;
typedef View3f   PlaneView;
typedef Vector3f Vector3D;
typedef Point3f  Point3D;
typedef Vector2f Vector2D;
typedef Point2f  Point2D;
typedef Vector4f  Vector4D;
typedef Matrix4f  Matrix4D;
typedef itk::Point<ScalarType,3> ITKPoint3D;
typedef itk::Vector<ScalarType,3> ITKVector3D;

//itk vs. vecmath conversion
template <class T>
inline void vm2itk(const Point3<T>& p, ITKPoint3D& a)
{
	a[0]=p.x;
	a[1]=p.y;
	a[2]=p.z;
}
 
template <class T>
inline void itk2vm(const ITKPoint3D& a, Point3<T> & p)
{
	p.set(a[0], a[1], a[2]);
}

template <class T>
inline void vm2itk(const Vector3<T>& p, ITKVector3D& a)
{
	a[0]=p.x;
	a[1]=p.y;
	a[2]=p.z;
}
 
template <class T>
inline void itk2vm(const ITKVector3D& a, Vector3<T> & p)
{
	p.set(a[0], a[1], a[2]);
}


} // namespace mitk


#endif /* MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD */
