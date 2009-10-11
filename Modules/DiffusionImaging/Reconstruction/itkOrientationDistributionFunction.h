/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkSymmetricSecondRankTensor.h,v $
  Language:  C++
  Date:      $Date: 2008-03-10 22:48:13 $
  Version:   $Revision: 1.25 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkOrientationDistributionFunction_h
#define __itkOrientationDistributionFunction_h

// Undefine an eventual OrientationDistributionFunction macro
#ifdef OrientationDistributionFunction
#undef OrientationDistributionFunction
#endif

#include "MitkDiffusionImagingExports.h"
#include "itkIndent.h"
#include "itkFixedArray.h"
#include "itkMatrix.h"
#include "itkSymmetricEigenAnalysis.h"
#include "itkPointShell.h"
#include "itkSimpleFastMutexLock.h"

#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkDelaunay2D.h"
#include "vtkCleanPolyData.h"
#include "vtkAppendPolyData.h"
#include "vtkPlane.h"

namespace itk
{

/** \class OrientationDistributionFunction
 * \brief Represents an ODF for Q-Ball imaging.
 *
 * Reference: David S. Tuch, Q-ball imaging, 
 * Magnetic Resonance in Medicine Volume 52 Issue 6, Pages 1358 - 1372
 *
 * \author Klaus Fritzsche, MBI
 *
 */

template < typename TComponent, unsigned int NOdfDirections >
class OrientationDistributionFunction: public 
        FixedArray<TComponent,NOdfDirections>
{
public:

  enum InterpolationMethods {
    ODF_NEAREST_NEIGHBOR_INTERP,
    ODF_TRILINEAR_BARYCENTRIC_INTERP,
    ODF_SPHERICAL_GAUSSIAN_BASIS_FUNCTIONS
  };

  /** Standard class typedefs. */
  typedef OrientationDistributionFunction  Self;
  typedef FixedArray<TComponent,NOdfDirections> Superclass;
  
  /** Dimension of the vector space. */
  itkStaticConstMacro(InternalDimension, unsigned int, NOdfDirections);

  /** Convenience typedefs. */
  typedef FixedArray<TComponent,
    itkGetStaticConstMacro(InternalDimension)> BaseArray;
  
  /**  Define the component type. */
  typedef TComponent ComponentType;
  typedef typename Superclass::ValueType ValueType;
  typedef typename NumericTraits<ValueType>::RealType AccumulateValueType;
  typedef typename NumericTraits<ValueType>::RealType RealValueType;
  
  typedef Matrix<TComponent, NOdfDirections, NOdfDirections> MatrixType;

  typedef vnl_matrix_fixed<double, 3, NOdfDirections> DirectionsType;

  /** Default constructor has nothing to do. */
  OrientationDistributionFunction() {this->Fill(0);}
  
  OrientationDistributionFunction (const ComponentType& r) { this->Fill(r); }
  
  typedef ComponentType ComponentArrayType[ itkGetStaticConstMacro(InternalDimension) ];

  /** Pass-through constructor for the Array base class. */
  OrientationDistributionFunction(const Self& r): BaseArray(r) {}
  OrientationDistributionFunction(const ComponentArrayType r): BaseArray(r) {}    
  
  /** Pass-through assignment operator for the Array base class. */
  Self& operator= (const Self& r);
  Self& operator= (const ComponentType& r);
  Self& operator= (const ComponentArrayType r);

  /** Aritmetic operations between pixels. Return a new OrientationDistributionFunction. */
  Self operator+(const Self &vec) const;
  Self operator-(const Self &vec) const;
  const Self & operator+=(const Self &vec);
  const Self & operator-=(const Self &vec);

  /** Arithmetic operations between ODFs and scalars */
  Self operator*(const RealValueType & scalar ) const;
  Self operator/(const RealValueType & scalar ) const;
  const Self & operator*=(const RealValueType & scalar );
  const Self & operator/=(const RealValueType & scalar );

  /** Return the number of components. */
  static DirectionsType* GetDirections() 
  { 
    return itkGetStaticConstMacro(m_Directions);
  }

  /** Return the number of components. */
  static unsigned int GetNumberOfComponents() 
  { 
    return itkGetStaticConstMacro(InternalDimension);
  }

  /** Return the value for the Nth component. */
  ComponentType GetNthComponent(int c) const
  { return this->operator[](c); }

  /** Return the value for the Nth component. */
  ComponentType GetInterpolatedComponent( vnl_vector_fixed<double,3> dir, InterpolationMethods method ) const;

  /** Set the Nth component to v. */
  void SetNthComponent(int c, const ComponentType& v)  
  {  this->operator[](c) = v; }

  /** Matrix notation, in const and non-const forms. */
  ValueType & operator()( unsigned int row, unsigned int col );
  const ValueType & operator()( unsigned int row, unsigned int col ) const;

  /** Set the distribution to isotropic.*/
  void SetIsotropic();

  /** Pre-Multiply by a Matrix as ResultingTensor = Matrix * ThisTensor. */
  Self PreMultiply( const MatrixType & m ) const;

  /** Post-Multiply by a Matrix as ResultingTensor = ThisTensor * Matrix. */
  Self PostMultiply( const MatrixType & m ) const;

  void Normalize();

  Self MinMaxNormalize() const;
  
  Self MaxNormalize() const;

  int GetPrincipleDiffusionDirection() const;

  int GetNthDiffusionDirection(int n, vnl_vector_fixed<double,3> rndVec) const;

  TComponent GetGeneralizedFractionalAnisotropy() const;

  TComponent GetGeneralizedGFA(int k, int p) const;

  TComponent GetNormalizedEntropy() const;

  TComponent GetNematicOrderParameter() const;
  
  TComponent GetStdDevByMaxValue() const;

  TComponent GetPrincipleCurvature(double alphaMinDegree, double alphaMaxDegree, int invert) const;

  static std::vector<int> GetNeighbors(int idx);

  static vtkPolyData* GetBaseMesh(){ComputeBaseMesh(); return m_BaseMesh;}

  static void ComputeBaseMesh();

  static double GetMaxChordLength();

  static vnl_vector_fixed<double,3> GetDirection(int i);

private:

  static vtkPolyData* m_BaseMesh;

  static double m_MaxChordLength;

  static DirectionsType* m_Directions;

  static std::vector< std::vector<int>* >* m_NeighborIdxs;

  static std::vector< std::vector<int>* >* m_AngularRangeIdxs;

  static std::vector<int>* m_HalfSphereIdxs;

  static itk::SimpleFastMutexLock m_MutexBaseMesh;
  static itk::SimpleFastMutexLock m_MutexHalfSphereIdxs;
  static itk::SimpleFastMutexLock m_MutexNeighbors;
  static itk::SimpleFastMutexLock m_MutexAngularRange;

};

/** This extra typedef is necessary for preventing an Internal Compiler Error in
 * Microsoft Visual C++ 6.0. This typedef is not needed for any other compiler. */
typedef std::ostream               OutputStreamType;
typedef std::istream               InputStreamType;

template< typename TComponent, unsigned int NOdfDirections  >  
MitkDiffusionImaging_EXPORT OutputStreamType& operator<<(OutputStreamType& os, 
              const OrientationDistributionFunction<TComponent,NOdfDirections> & c); 
template< typename TComponent, unsigned int NOdfDirections  >  
MitkDiffusionImaging_EXPORT InputStreamType& operator>>(InputStreamType& is, 
                    OrientationDistributionFunction<TComponent,NOdfDirections> & c); 

   

} // end namespace itk

//#include "itkNumericTraitsTensorPixel.h"


// Define instantiation macro for this template.
#define ITK_TEMPLATE_OrientationDistributionFunction(_, EXPORT, x, y) namespace itk { \
  _(2(class MitkDiffusionImaging_EXPORT EXPORT OrientationDistributionFunction< ITK_TEMPLATE_2 x >)) \
  namespace Templates { typedef OrientationDistributionFunction< ITK_TEMPLATE_2 x > \
                                         OrientationDistributionFunction##y; } \
  }

#if ITK_TEMPLATE_EXPLICIT
# include "Templates/itkOrientationDistributionFunction+-.h"
#endif

#if ITK_TEMPLATE_TXX
# include "itkOrientationDistributionFunction.cpp"
#endif


#endif //__itkOrientationDistributionFunction_h
