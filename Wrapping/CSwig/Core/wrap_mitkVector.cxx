#include "mitkVector.h"
#include <itkPoint.h>
#include <float.h>
#include <itkIndex.h>
#include <itkVector.h>
#include <itkMatrix.h>
#include <itkTransform.h>
#include <vnl/vnl_quaternion.h>
#include "mitkCommon.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="Vector";
     namespace wrappers
     {
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
        typedef vnl_quaternion<ScalarType> Quaternion;

        typedef mitk::VectorTraits<VnlVector> VectorTraitsVnl;
        typedef mitk::VectorTraits<double[4]> VectorTraitsD4;
        typedef mitk::VectorTraits<itk::Index<5>> VectorTraitsIndex5;
        typedef mitk::VectorTraits<itk::Index<3>> VectorTraitsIndex3;
        typedef mitk::VectorTraits<long int [3]> VectorTraitsLI3;
        typedef mitk::VectorTraits<float [3]> VectorTraitsF3;
        typedef mitk::VectorTraits<double [3]> VectorTraitsD3;
        typedef mitk::VectorTraits<vnl_VectorTraits_fixed<ScalarType, 3>> VectorTraitsVnlFixed;
        typedef mitk::VectorTraits<long unsigned int[3]> VectorTraitsLUI3;
        typedef mitk::VectorTraits<unsigned int *> VectorTraitsUI;
        typedef mitk::VectorTraits<ScalarType[4]> VectorTraitsScalarType4;
        typedef mitk::VectorTraits<itk::VectorTraits<float,3>> VectorTraitsVectorTraitsF3;
        typedef mitk::VectorTraits<itk::Point<float,3>> VectorTraitsPointF3;
        typedef mitk::VectorTraits<itk::Point<float,4>> VectorTraitsPointF4;
        typedef mitk::VectorTraits<itk::VectorTraits<double,3>> VectorTraitsVectorTraitsD3;
        typedef mitk::VectorTraits<itk::Point<double,3>> VectorTraitsPointD3;
        typedef mitk::VectorTraits<itk::VectorTraits<int,3>> VectorTraitsVectorTraitsI3;
        typedef mitk::VectorTraits<itk::Point<int,3>> VectorTraitsPointI3;
     }
}

#endif

