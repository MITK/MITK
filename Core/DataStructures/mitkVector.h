#ifndef MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD
#define MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD

#include "SpaceGeometry.h"
#include <itkPoint.h>
#include <float.h>



namespace mitk {
typedef float ScalarType;
typedef View<ScalarType>   PlaneView;
typedef Plane<ScalarType> Plane3D;
typedef Line<ScalarType> Line3D;
typedef Vector3<ScalarType> Vector3D;
typedef Point3<ScalarType>  Point3D;
typedef Vector2<ScalarType> Vector2D;
typedef Point2<ScalarType>  Point2D;
typedef Vector4<ScalarType>  Vector4D;
typedef Matrix4<ScalarType> Matrix4D;
typedef itk::Point<ScalarType,3> ITKPoint3D;
typedef itk::Vector<ScalarType,3> ITKVector3D;

typedef class itk::NumericTraits<mitk::ScalarType> ScalarTypeNumericTraits;

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
