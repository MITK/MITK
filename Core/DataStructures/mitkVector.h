#ifndef MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD
#define MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD

#include <itkPoint.h>
#include <itkFixedCenterOfRotationAffineTransform.h>
#include <float.h>

namespace mitk {
typedef float ScalarType;

typedef itk::Matrix<ScalarType, 3, 3> Matrix3D;
typedef vnl_matrix_fixed<ScalarType, 3, 3> VnlMatrix3D;
typedef itk::Transform<ScalarType, 3, 3> Transform3D;
typedef itk::FixedCenterOfRotationAffineTransform<ScalarType, 3> AffineTransform3D;
typedef vnl_vector<ScalarType> VnlVector;
typedef vnl_vector_ref<ScalarType> VnlVectorRef;

typedef itk::Point<ScalarType,2> Point2D;
typedef itk::Point<ScalarType,3> Point3D;
typedef itk::Vector<ScalarType,2> Vector2D;
typedef itk::Vector<ScalarType,3> Vector3D;

//##Documentation
//##@brief enumeration of the type a point can be
typedef enum PointSpecificationType
{
  PTUNDEFINED = 0,
  PTSTART,
  PTCORNER,
  PTEDGE
};


typedef class itk::NumericTraits<mitk::ScalarType> ScalarTypeNumericTraits;
extern const ScalarType eps;
extern const ScalarType epsSquared;

template <class Tout>
  inline void FillVector3D(Tout& out, ScalarType x, ScalarType y, ScalarType z)
{
  out[0] = x;
  out[1] = y;
  out[2] = z;
}


template <class Tin, class Tout>
  inline void itk2vtk(const Tin& in, Tout& out)
{
  out[0]=in[0];
  out[1]=in[1];
  out[2]=in[2];
}

template <class Tin, class Tout>
  inline void vtk2itk(const Tin& in, Tout& out)
{
  out[0]=in[0];
  out[1]=in[1];
  out[2]=in[2];
}

template <class Tin, class Tout>
  inline void vnl2vtk(const vnl_vector<Tin>& in, Tout *out)
{
  unsigned int i;
  for(i=0; i<in.size();++i)     
    out[i]=(Tout) (in[i]);
}

template <class Tin, class Tout>
  inline void vtk2vnl(const Tin *in, vnl_vector<Tout>& out)
{
  unsigned int i;
  for(i=0; i<out.size();++i)
    out[i]=(Tout) (in[i]);
}

template <class Tin, class Tout>
  inline void vtk2vnlref(const Tin *in, vnl_vector_ref<Tout>& out)
{
  unsigned int i;
  for(i=0; i<out.size();++i)
    out[i]=(Tout) (in[i]);
}

template <class Tin, class Tout, unsigned int n>
  inline void vnl2vtk(const vnl_vector_fixed<Tin, n>& in, Tout *out)
{
  unsigned int i;
  for(i=0; i<in.size();++i)
    out[i]=(Tout) (in[i]);
}

template <class Tin, class Tout, unsigned int n>
  inline void vtk2vnl(const Tin *in, vnl_vector_fixed<Tout, n>& out)
{
  unsigned int i;
  for(i=0; i<out.size();++i)
    out[i]=(Tout) (in[i]);
}

template <class T, unsigned int NVectorDimension>
  itk::Vector<T, NVectorDimension> operator+(const itk::Vector<T, NVectorDimension> &vector, const itk::Point<T, NVectorDimension> &point)
{
  itk::Vector<T, NVectorDimension> sub;
  for( unsigned int i=0; i<NVectorDimension; i++) 
  {
    sub[i] = vector[i]+point[i];
  }
  return sub;
}

template <class T, unsigned int NVectorDimension>
  inline itk::Vector<T, NVectorDimension>& operator+=(itk::Vector<T, NVectorDimension> &vector, const itk::Point<T, NVectorDimension> &point)
{
  for( unsigned int i=0; i<NVectorDimension; i++) 
  {
    vector[i] += point[i];
  }
  return vector;
}

template <class T, unsigned int NVectorDimension>
  itk::Vector<T, NVectorDimension> operator-(const itk::Vector<T, NVectorDimension> &vector, const itk::Point<T, NVectorDimension> &point)
{
  itk::Vector<T, NVectorDimension> sub;
  for( unsigned int i=0; i<NVectorDimension; i++) 
  {
    sub[i] = vector[i]-point[i];
  }
  return sub;
}

template <class T, unsigned int NVectorDimension>
  inline itk::Vector<T, NVectorDimension>& operator-=(itk::Vector<T, NVectorDimension> &vector, const itk::Point<T, NVectorDimension> &point)
{
  for( unsigned int i=0; i<NVectorDimension; i++) 
  {
    vector[i] -= point[i];
  }
  return vector;
}

template <typename TCoordRep, unsigned int NPointDimension>
  inline bool Equal(const itk::Vector<TCoordRep, NPointDimension>& vector1, const itk::Vector<TCoordRep, NPointDimension>& vector2) 
{
  typename itk::Vector<TCoordRep, NPointDimension>::VectorType diff = vector1-vector2;
  return diff.GetSquaredNorm() < mitk::epsSquared; 
}

template <typename TCoordRep, unsigned int NPointDimension>
  inline bool Equal(const itk::Point<TCoordRep, NPointDimension>& vector1, const itk::Point<TCoordRep, NPointDimension>& vector2) 
{
  typename itk::Point<TCoordRep, NPointDimension>::VectorType diff = vector1-vector2;
  return diff.GetSquaredNorm() < mitk::epsSquared; 
}

inline bool Equal(const mitk::VnlVector& vector1, const mitk::VnlVector& vector2) 
{
  mitk::VnlVector diff = vector1-vector2;
  return diff.squared_magnitude() < mitk::epsSquared; 
}

inline bool Equal(double scalar1, double scalar2) 
{
  return fabs(scalar1-scalar2) < mitk::eps;
}

inline bool Equal(float scalar1, float scalar2) 
{
  return fabs(scalar1-scalar2) < mitk::eps;
}

} // namespace mitk


#endif /* MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD */
