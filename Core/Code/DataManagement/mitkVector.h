/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD
#define MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD

// this is needed for memcopy in ITK
// can be removed when fixed in ITK
#include <cstring>

#include <itkPoint.h>
#include <float.h>
#include <itkIndex.h>
#include <itkContinuousIndex.h>
#include <itkVector.h>
#include <itkMatrix.h>
#include <itkTransform.h>
#include <vnl/vnl_quaternion.h>
#include <MitkExports.h>

#ifndef DOXYGEN_SKIP

namespace mitk {
typedef float ScalarType;

typedef itk::Matrix<ScalarType, 3, 3> Matrix3D;
typedef itk::Matrix<ScalarType,4,4> Matrix4D;
typedef vnl_matrix_fixed<ScalarType, 3, 3> VnlMatrix3D;
typedef itk::Transform<ScalarType, 3, 3> Transform3D;
typedef vnl_vector<ScalarType> VnlVector;
typedef vnl_vector_ref<ScalarType> VnlVectorRef;

typedef itk::Point<ScalarType,2> Point2D;
typedef itk::Point<ScalarType,3> Point3D;
typedef itk::Point<ScalarType,4> Point4D;
typedef itk::Point<int,2> Point2I;
typedef itk::Point<int,3> Point3I;
typedef itk::Point<int,4> Point4I;
typedef itk::Vector<ScalarType,2> Vector2D;
typedef itk::Vector<ScalarType,3> Vector3D;
typedef itk::Index<3> Index3D;
typedef itk::ContinuousIndex<ScalarType, 3> ContinuousIndex3D;
typedef vnl_quaternion<ScalarType> Quaternion;

//##Documentation
//##@brief enumeration of the type a point can be
enum PointSpecificationType
{
  PTUNDEFINED = 0,
  PTSTART,
  PTCORNER,
  PTEDGE,
  PTEND
};

typedef itk::NumericTraits<mitk::ScalarType> ScalarTypeNumericTraits;
MITK_CORE_EXPORT extern const ScalarType eps;
MITK_CORE_EXPORT extern const ScalarType sqrteps;
MITK_CORE_EXPORT extern const double large;

template <class T> class VectorTraits {
  public:
    typedef T ValueType;
};

template <> class VectorTraits<VnlVector> {
  public:
    typedef ScalarType ValueType;
};

template<> class VectorTraits<double[4]> {
  public:
    typedef double ValueType;
};
template<> class VectorTraits< itk::Index<5> > {
  public:
    typedef itk::Index<5>::IndexValueType ValueType;
};

template<> class VectorTraits< itk::Index<3> > {
  public:
    typedef itk::Index<3>::IndexValueType ValueType;
};

template<> class VectorTraits< long int [3]> {
  public:
    typedef long int ValueType;
};
template<> class VectorTraits< float [3]> {
  public:
    typedef float ValueType;
};
template<> class VectorTraits< double [3]> {
  public:
    typedef double ValueType;
};

template<> class VectorTraits< vnl_vector_fixed<ScalarType, 3> > {
  public:
    typedef ScalarType ValueType;
};

template<> class VectorTraits< long unsigned int[3]> {
  public:
    typedef long unsigned int ValueType;
};

template<> class VectorTraits< unsigned int *> {
  public:
    typedef unsigned int ValueType;
};

template<> class VectorTraits< ScalarType[4] > {
  public:
    typedef ScalarType ValueType;
};

template<> class VectorTraits< itk::Vector<float,3> > {
  public:
    typedef float ValueType;
};

template<> class VectorTraits< itk::Point<float,3> > {
  public:
    typedef float ValueType;
};

template<> class VectorTraits< itk::Point<float,4> > {
  public:
    typedef float ValueType;
};

template<> class VectorTraits< itk::Vector<double,3> > {
  public:
    typedef double ValueType;
};

template<> class VectorTraits< itk::Point<double,3> > {
  public:
    typedef double ValueType;
};

template<> class VectorTraits< itk::Vector<int,3> > {
  public:
    typedef int ValueType;
};

template<> class VectorTraits< itk::Point<int,3> > {
  public:
    typedef int ValueType;
};

template <class Tin, class Tout>
  inline void itk2vtk(const Tin& in, Tout& out)
{
  out[0]=(typename VectorTraits<Tout>::ValueType)(in[0]);
  out[1]=(typename VectorTraits<Tout>::ValueType)(in[1]);
  out[2]=(typename VectorTraits<Tout>::ValueType)(in[2]);
}

template <class Tin, class Tout>
  inline void vtk2itk(const Tin& in, Tout& out)
{
  out[0]=(typename VectorTraits<Tout>::ValueType)(in[0]);
  out[1]=(typename VectorTraits<Tout>::ValueType)(in[1]);
  out[2]=(typename VectorTraits<Tout>::ValueType)(in[2]);
}
template <class Tout>
  inline void FillVector3D(Tout& out, ScalarType x, ScalarType y, ScalarType z)
{
  out[0] = (typename VectorTraits<Tout>::ValueType)x;
  out[1] = (typename VectorTraits<Tout>::ValueType)y;
  out[2] = (typename VectorTraits<Tout>::ValueType)z;
}
template <class Tout>
  inline void FillVector4D(Tout& out, ScalarType x, ScalarType y, ScalarType z, ScalarType t)
{
  out[0] = (typename VectorTraits<Tout>::ValueType)x;
  out[1] = (typename VectorTraits<Tout>::ValueType)y;
  out[2] = (typename VectorTraits<Tout>::ValueType)z;
  out[3] = (typename VectorTraits<Tout>::ValueType)t;
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

/*!
\brief Check for matrix equality with a user defined accuracy. As an equality metric the root mean squared error (RMS) of all elements is calculated.
\param matrix1 first vnl matrix
\param matrix2 second vnl matrix
\epsilon user defined accuracy bounds
*/
template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
inline bool MatrixEqualRMS(const vnl_matrix_fixed<TCoordRep,NRows,NCols>& matrix1,const vnl_matrix_fixed<TCoordRep,NRows,NCols>& matrix2,mitk::ScalarType epsilon=mitk::eps)
{
  if ( (matrix1.rows() == matrix2.rows()) && (matrix1.cols() == matrix2.cols()) )
  {
    vnl_matrix_fixed<TCoordRep,NRows,NCols> differenceMatrix = matrix1-matrix2;
    if (differenceMatrix.rms()<epsilon)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

/*!
\brief Check for matrix equality with a user defined accuracy. As an equality metric the root mean squared error (RMS) of all elements is calculated.
\param matrix1 first itk matrix
\param matrix2 second itk matrix
\epsilon user defined accuracy bounds
*/
template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
inline bool MatrixEqualRMS(const itk::Matrix<TCoordRep, NRows, NCols>& matrix1,const itk::Matrix<TCoordRep, NRows, NCols>& matrix2,mitk::ScalarType epsilon=mitk::eps)
{
  return mitk::MatrixEqualRMS(matrix1.GetVnlMatrix(),matrix2.GetVnlMatrix(),epsilon);
}

/*!
\brief Check for element-wise matrix equality with a user defined accuracy.
\param matrix1 first vnl matrix
\param matrix2 second vnl matrix
\epsilon user defined accuracy bounds
*/
template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
inline bool MatrixEqualElementWise(const vnl_matrix_fixed<TCoordRep,NRows,NCols>& matrix1,const vnl_matrix_fixed<TCoordRep,NRows,NCols>& matrix2,mitk::ScalarType epsilon=mitk::eps)
{
  if ( (matrix1.rows() == matrix2.rows()) && (matrix1.cols() == matrix2.cols()) )
  {
    for( unsigned int r=0; r<NRows; r++)
    {
      for( unsigned int c=0; c<NCols; c++ )
      {
        TCoordRep difference =  fabs(matrix1(r,c)-matrix2(r,c));
        if (difference>epsilon)
        {
          return false;
        }
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}

/*!
\brief Check for element-wise matrix equality with a user defined accuracy.
\param matrix1 first itk matrix
\param matrix2 second itk matrix
\epsilon user defined accuracy bounds
*/
template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
inline bool MatrixEqualElementWise(const itk::Matrix<TCoordRep, NRows, NCols>& matrix1,const itk::Matrix<TCoordRep, NRows, NCols>& matrix2,mitk::ScalarType epsilon=mitk::eps)
{
  return mitk::MatrixEqualElementWise(matrix1.GetVnlMatrix(),matrix2.GetVnlMatrix(),epsilon);
}

template <typename TCoordRep, unsigned int NPointDimension>
inline bool Equal(const itk::Vector<TCoordRep, NPointDimension>& vector1, const itk::Vector<TCoordRep, NPointDimension>& vector2, TCoordRep eps=mitk::eps)
{
  typename itk::Vector<TCoordRep, NPointDimension>::VectorType diff = vector1-vector2;
  for (unsigned int i=0; i<NPointDimension; i++)
      if (diff[i]>eps || diff[i]<-eps)
      return false;
  return true;
}

template <typename TCoordRep, unsigned int NPointDimension>
  inline bool Equal(const itk::Point<TCoordRep, NPointDimension>& vector1, const itk::Point<TCoordRep, NPointDimension>& vector2, TCoordRep eps=mitk::eps)
{
  typename itk::Point<TCoordRep, NPointDimension>::VectorType diff = vector1-vector2;
  for (unsigned int i=0; i<NPointDimension; i++)
    if (diff[i]>eps || diff[i]<-eps)
      return false;
  return true;
}

inline bool Equal(const mitk::VnlVector& vector1, const mitk::VnlVector& vector2, ScalarType eps=mitk::eps)
{
  mitk::VnlVector diff = vector1-vector2;
  for (unsigned int i=0; i<diff.size(); i++)
    if (diff[i]>eps || diff[i]<-eps)
      return false;
  return true;
}

inline bool Equal(double scalar1, double scalar2, ScalarType eps=mitk::eps)
{
  return fabs(scalar1-scalar2) < eps;
}

template <typename TCoordRep, unsigned int NPointDimension>
  inline bool Equal(const vnl_vector_fixed<TCoordRep, NPointDimension> & vector1, const vnl_vector_fixed<TCoordRep, NPointDimension>& vector2, TCoordRep eps=mitk::eps)
{
  vnl_vector_fixed<TCoordRep, NPointDimension> diff = vector1-vector2;
  return diff.squared_magnitude() < mitk::eps;
}

template <typename U, typename V, unsigned int NRows, unsigned int NColumns>
inline void TransferMatrix(const itk::Matrix<U, NRows, NColumns>& in, itk::Matrix<V, NRows, NColumns>& out)
{
  for (unsigned int i = 0; i < in.RowDimensions; ++i)
    for (unsigned int j = 0; j < in.ColumnDimensions; ++j)
      out[i][j] = in[i][j];
}


} // namespace mitk

#endif //DOXYGEN_SKIP

/*
 * This part of the code has been shifted here to avoid compiler clashes
 * caused by including <itkAffineGeometryFrame.h> before the declaration of
 * the Equal() methods above. This problem occurs when using MSVC and is
 * probably related to a compiler bug.
 */

#include <itkAffineGeometryFrame.h>

namespace mitk
{
  typedef itk::AffineGeometryFrame<ScalarType, 3>::TransformType AffineTransform3D;
}


#define mitkSetConstReferenceMacro(name,type) \
  virtual void Set##name (const type & _arg) \
  { \
    itkDebugMacro("setting " << #name " to " << _arg ); \
    if (this->m_##name != _arg) \
      { \
      this->m_##name = _arg; \
      this->Modified(); \
      } \
  }

#define mitkSetVectorMacro(name,type) \
  mitkSetConstReferenceMacro(name,type)

#define mitkGetVectorMacro(name,type) \
  itkGetConstReferenceMacro(name,type)

#endif /* MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD */
