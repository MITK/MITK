/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#ifndef __itkMultiGaussianImageSource_h
#define __itkMultiGaussianImageSource_h

#include "itkImageSource.h"
#include "itkNumericTraits.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageFileWriter.h"
#include <itkMapContainer.h>


namespace itk
{
/** \class MultiGaussianImageSource

\brief Generate an 3-dimensional multigaussian image.

This class defines an 3-dimensional Image, in which the value at one voxel equals the value of a multigaussian function evaluated at the
voxel's coordinates. The multigaussian function is build as a sum of N gaussian function. This is defined by the following parameters:

1. CenterX, CenterY, CenterZ - vectors of the size of N determining the expectancy value at the x-, y- and the z-axis. That means: The i-th
gaussian bell curve takes its maximal value at the voxel with index [CenterX(i); CenterY(i); Centerz(i)].

2. SigmaX, SigmaY, SigmaZ - vectors of the size of N determining the deviation at the x-, y- and the z-axis. That means: The width of the i-the gaussian bell curve deviation in the x-axis is SigmaX(i), in the y-axis is SigmaY(i) and in the z-axis is SigmaZ(i).

3. Altitude - vector of the size of N determining the altitude: the i-th gaussian bell curve has a height of Altitude(i).
This class allows by the method CalculateMidpointAndMeanValue() to find a sphere with a specified radius that has a maximal mean value over all sphere with that radius with midpoint inside or at the boundary of the image. Furthermore it can calculate the maximal und minimal pixel intensities and whose indices in the founded sphere.

\section algorithm_description Algorithm description

\subsection gaus_generation Generation of a multigauss image

A multigauss function consists of the sum of \f$N\f$ gauss function. The \f$i\f$-th \linebreak (\f$1 \leq i \leq N\f$) gaussian is described with the following seven parameters:
  1. \f$x_0^{(i)}\f$ is the expectancy value in the \f$x\f$-Axis
  2. \f$y_0^{(i)}\f$ is the expectancy value in the \f$y\f$-Axis
  3. \f$z_0^{(i)}\f$ is the expectancy value in the \f$z\f$-Axis
  4. \f$\sigma_x^{(i)}\f$ is the deviation in the \f$x\f$-Axis
  5. \f$\sigma_y^{(i)}\f$ is the deviation in the \f$y\f$-Axis
  6. \f$\sigma_z^{(i)}\f$ is the deviation in the \f$z\f$-Axis
  7. \f$a^{(i)}\f$ is the  altitude of the gaussian.

A gauss function has the following form:

\f{eqnarray*}{
f^{(i)}(x,y,z) = a^{(i)}
exp \left[ - \left(
\frac{(x - x_0^{(i)})^2}{2 (\sigma_x^{(i)})^2} +
\frac{(y - y_0^{(i)})^2}{2 (\sigma_y^{(i)})^2} +
\frac{(z - z_0^{(i)})^2}{2 (\sigma_z^{(i)})^2}
\right) \right].
\f}

A multigauss function has then the form:

\f{align*}{
f_N(x,y,z) =& \sum_{i = 1}^{N}f^{(i)}(x,y,z)\\
=&\sum_{i=1}^{N} a^{(i)}
exp \left[ - \left(
\frac{(x - x_0^{(i)})^2}{2 (\sigma_x^{(i)})^2} +
\frac{(y - y_0^{(i)})^2}{2 (\sigma_y^{(i)})^2} +
\frac{(z - z_0^{(i)})^2}{2 (\sigma_z^{(i)})^2}
\right) \right].
\f}

The multigauss function \f$f_N\f$ will be evaluated at each voxel coordinate to become the voxel intensity.

\subsection calculating_statistics Algorithm for calculating statiscic in a sphere

This section explains how we can find a sphere region which has a maximal mean value over all sphere regions with a fixed radius. Furthermore we want to calculate the maximal and minimal value in the wanted sphere.

To calculate the mean value in a sphere we use the simple definition of a mean value of a function in a bounded discrete space: the sum of the values of the function divided by the number of the summand.

We discretize the spherical region of interest and use the spherical coordinate system to describe each point \linebreak [\f$x,y,z\f$] in this region:

\f{eqnarray*}{
x = r \sin(\phi) \cos(\psi)\hspace{20pt}
y = r \sin(\phi) \sin(\psi)\hspace{20pt}
z = r \cos(\phi),
\f}

where \f$r \in [0;R]\f$, \f$\phi \in [0; \pi]\f$, \f$ \psi \in [-\pi; \pi)\f$.
We decided to have only one discretizing parameter T, which is equal to the number of steps we make to traverse the radius \f$R\f$ of the sphere. So the higher the value of T is, the more accurate the mean value is. The two angle \f$\psi\f$ and \f$\phi\f$ have an equal step size, which is calculate as follows: Define a constant distance (\f$dist\f$) between the points on the sphere with the smallest radius. In our class we set \f$dist\f$ such a value, that on the equator of the smallest sphere only four points lie. Then we calculate for each radius the angle step size in such a way, that the points on the equator of the particular sphere have always the same distance and this should  be approximately  the same as \f$dist\f$ ( exactly the same distance as \f$dist\f$ is not always possible, because the length of the equator is generally not a multiple of a natural number). With such a discretization we almost achieve  a uniformly distribution of the points in the "hot spot" region. So the following term equals the mean value in the sphere with midpoint [\f$x_l, y_l, z_l\f$] for a multigauss function:

\f{align*}{
m_l = & \frac{1}{\kappa}
\sum_{i = 1}^{T} \sum_{j = 0}^{\eta(i)} \sum_{k = 0}^{2\eta(i)} \sum_{i = 1}^{N} a^{(i)} exp \Big[ - \Big(
 \frac{(r_i \sin(\phi_j) \cos(\psi_k) - x_0^{(i)} + x_l)^2}{2 (\sigma_x^{(i)})^2} \\
&+\frac{(r_i \sin(\phi_j) \sin(\psi_k) - y_0^{(i)} + y_l)^2}{2 (\sigma_y^{(i)})^2}
+\frac{(r_i \cos(\phi_j)              - z_0^{(i)} + z_l)^2}{2 (\sigma_z^{(i)})^2}\Big)\Big]
\f}

Here denotes \f$\kappa\f$ the number of summand; the radius of the \f$i\f$-th sphere equals its index multiplied by the radius step size and the angle \f$\phi_j\f$ equals its index multiplied by the angle step size (analogical \f$\psi_j\f$):
\f{align*}{
r_i = i \frac{R}{T}, \hspace{20pt} \phi_j = j \frac{\pi}{\eta(i)}, \hspace{20pt} \psi_k = \pi + k \frac{\pi}{\eta(k)},
\f}
where \f$\eta(i)\f$ is the number of angle steps: \f$\eta(i)= \left \lfloor \frac{\pi r_i}{dist} \right \rfloor\f$ and \f$dist\f$ the fixed distance between two points: \f$dist  = \frac{\pi R}{2T}\f$. We specify the volume of the "hot spot" to be \f$1\f$ cm\f$^3\f$. The radius of the sphere is there for:
\f{align*}{
R = \left(\frac{3}{4\pi}\right)^{1/3}.
\f}
Now we know how to calculate the mean value in a sphere for given midpoint and radius of the wanted region. So we assume each voxel in the given image to be the sphere midpoint and we calculate the mean value as described  above. If the new mean value is greater then the "maximum-until-now", we take the new value to be the "maximum-until-now". Then we go to the next voxel and make the same calculation and so on. At the same time we save the coordinates  of the midpoint voxel.

After we found the midpoint and the maximal mean value, we can calculate the maximum and the minimum in the sphere: we just traverse all the voxels in the region and take the maximum and minimum value and the respective coordinates.

We can also optimize the calculation of the mean value in the founded sphere just by increasing the number of  radius steps and calculate this value one more time. We set the new step number to be four time greater then T.
*/
template< typename TOutputImage >
class ITK_EXPORT MultiGaussianImageSource:public ImageSource< TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef MultiGaussianImageSource           Self;
  typedef ImageSource< TOutputImage > Superclass;
  typedef SmartPointer< Self >        Pointer;
  typedef SmartPointer< const Self >  ConstPointer;

  /** Typedef for the output image PixelType. */
  typedef typename TOutputImage::PixelType OutputImagePixelType;

  /** Typedef to describe the output image region type. */
  typedef typename TOutputImage::RegionType OutputImageRegionType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Basic types from the OutputImageType */
  typedef typename TOutputImage::SizeType                     SizeType;
  typedef typename TOutputImage::IndexType                    IndexType;
  typedef typename TOutputImage::SpacingType                  SpacingType;
  typedef typename TOutputImage::PointType                    PointType;
  typedef typename SizeType::SizeValueType                    SizeValueType;
  typedef SizeValueType                                       SizeValueArrayType[TOutputImage::ImageDimension];
  typedef typename TOutputImage::SpacingValueType             SpacingValueType;
  typedef SpacingValueType                                    SpacingValueArrayType[TOutputImage::ImageDimension];
  typedef typename TOutputImage::PointValueType               PointValueType;
  typedef PointValueType                                      PointValueArrayType[TOutputImage::ImageDimension];
  typedef typename itk::ImageRegion<3> ::SizeValueType        SizeRegionType;
  /** Typedef to describe the sphere radius type. */
  typedef double                                              RadiusType;
  /** Typedef to describe the standard vector type. */
  typedef std::vector<double>                                 VectorType;
  /** Typedef to describe the itk vector type. */
  typedef Vector<double, TOutputImage::ImageDimension>        ItkVectorType;
  /** Typedef to describe the ImageRegionIteratorWithIndex type. */
  typedef ImageRegionIteratorWithIndex<TOutputImage>          IteratorType;
  /** Typedef to describe the Poiner type at the output image. */
  typedef typename TOutputImage::Pointer                      ImageType;

  typedef  MapContainer<unsigned int, PointType>              MapContainerPoints;
   typedef  MapContainer<unsigned int, double>                MapContainerRadius;


  /** Set/Get size of the output image */
  itkSetMacro(Size, SizeType);
  virtual void SetSize(SizeValueArrayType sizeArray);
  virtual const SizeValueType * GetSize() const;
  /** Set/Get spacing of the output image */
  itkSetMacro(Spacing, SpacingType);
  virtual void SetSpacing(SpacingValueArrayType spacingArray);
  virtual const SpacingValueType * GetSpacing() const;
  /** Set/Get origin of the output image */
  itkSetMacro(Origin, PointType);
  virtual void SetOrigin(PointValueArrayType originArray);
  virtual const PointValueType * GetOrigin() const;
  /** Get the number of gaussian functions in the output image */
  virtual unsigned int GetNumberOfGaussians() const;
  /** Set the number of gaussian function*/
  virtual void SetNumberOfGausssians( unsigned int );
  /** Set/Get the radius of the sphere */
  virtual const RadiusType GetRadius() const;
  virtual void  SetRadius( RadiusType  radius );
  /** Set/Get the number of steps to traverse the radius of the sphere */
  virtual const  int GetRadiusStepNumber() const;
  /** Set the number of steps to traverse the radius */
  virtual void  SetRadiusStepNumber( unsigned int stepNumber );
  /** Get the maximal mean value in a sphere over all possible spheres with midpoint in the image */
  virtual const OutputImagePixelType GetMaxMeanValue() const;
  /** Get the index of the midpoint of a sphere with the maximal mean value */
  virtual const IndexType GetSphereMidpoint() const;
  /** Calculates the value of the multigaussian function at a Point given by its coordinates [x;y;z] */
  virtual const double MultiGaussianFunctionValueAtPoint(double , double, double);
  /** Adds a multigaussian defined by the parameter: CenterX, CenterY, CenterZ, SigmaX, SigmaY, SigmaZ, Altitude.
   All parameters should have the same size, which determinates the number of the gaussian added. */
  virtual void AddGaussian( VectorType, VectorType, VectorType, VectorType, VectorType, VectorType, VectorType);
  /** Calculates and set the index of the midpoint of the sphere with the maximal mean value as well as the mean value*/
  virtual void CalculateMidpointAndMeanValue();
  /** Calculates and set the index an the value of maximulm and minimum in the wanted sphere*/
  virtual void CalculateMaxAndMinInSphere();
  /** Get the index in the sphere with maximal value*/
  virtual const IndexType GetMaxValueIndexInSphere() const;
  /** Get the maximal value in the sphere*/
  virtual const OutputImagePixelType GetMaxValueInSphere() const;
  /** Get the index in the sphere with minimal value*/
  virtual const IndexType GetMinValueIndexInSphere() const;
  /** Get the minimal value in the sphere*/
  virtual const OutputImagePixelType GetMinValueInSphere() const;
  /** Set the region of interest */
  virtual void SetRegionOfInterest(ItkVectorType, ItkVectorType);
  /** Optimize the mean value in the wanted sphere*/
  virtual void OptimizeMeanValue();
  /** Write a .mps file to visualise the point in the sphere*/
  // virtual void WriteXMLToTest();
  // virtual void WriteXMLToTestTheCuboid();
  virtual void WriteXMLToTestTheCuboidInsideTheSphere();
  virtual void CalculateTheMidPointMeanValueWithOctree();
  virtual void CalculateEdgesInSphere( PointType globalCoordinateMidpointCuboid, PointType globalCoordinateMidpointSphere, double cuboidRadius, int level);
  virtual double MultiGaussianFunctionValueAtCuboid(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax);
  virtual void InsertPoints( PointType globalCoordinateMidpointCuboid, double cuboidRadius);
  virtual void GenerateCuboidSegmentationInSphere( PointType globalCoordinateMidpointSphere );
  virtual double FunctionPhi(double value);

  //virtual void CalculateMidpoint();

  virtual  unsigned int IntersectTheSphere( PointType globalCoordinateMidpointCuboid, PointType globalCoordinateMidpointSphere, double sideLength);
  void SetNormalDistributionValues();

  //double Quadtrees( PointType globalCoordinateMidpointCuboid, PointType globalCoordinateMidpointSphere,  double sideLength, double meanValueTemp);


  /** Set the minimum possible pixel value. By default, it is
   * NumericTraits<TOutputImage::PixelType>::min(). */
  itkSetClampMacro( Min, OutputImagePixelType,
                    NumericTraits< OutputImagePixelType >::NonpositiveMin(),
                    NumericTraits< OutputImagePixelType >::max() );
  /** Check if a index is inside the image*/
  bool IsInImage(IndexType index);

  /** Get the minimum possible pixel value. */
  itkGetConstMacro(Min, OutputImagePixelType);

  /** Set the maximum possible pixel value. By default, it is
   * NumericTraits<TOutputImage::PixelType>::max(). */
  itkSetClampMacro( Max, OutputImagePixelType,
                    NumericTraits< OutputImagePixelType >::NonpositiveMin(),
                    NumericTraits< OutputImagePixelType >::max() );

  /** Get the maximum possible pixel value. */
  itkGetConstMacro(Max, OutputImagePixelType);

protected:
  MultiGaussianImageSource();
  ~MultiGaussianImageSource();
  void PrintSelf(std::ostream & os, Indent indent) const;

  virtual void GenerateData();
  virtual void GenerateOutputInformation();

private:
  MultiGaussianImageSource(const MultiGaussianImageSource &); //purposely not implemented
  void operator=(const MultiGaussianImageSource &);    //purposely not implemented

  SizeType                                  m_Size;                  //size of the output image
  SpacingType                               m_Spacing;               //spacing
  PointType                                 m_Origin;                //origin
  OutputImagePixelType                      m_MaxValueInSphere;      //maximal value in the wanted sphere
  IndexType                                 m_MaxValueIndexInSphere; //index of the maximal value in the wanted sphere
  OutputImagePixelType                      m_MinValueInSphere;      //minimal value in the wanted sphere
  IndexType                                 m_MinValueIndexInSphere; //index of the minimal value in the wanted sphere
  unsigned int                              m_NumberOfGaussians;     //number of Gaussians
  RadiusType                                m_Radius;                //radius of the sphere
  unsigned int                              m_RadiusStepNumber;      //number of steps to traverse the sphere radius
  OutputImagePixelType                      m_MeanValue;             //mean value in the wanted sphere
  OutputImagePixelType                      m_ValueAtMidpoint;        //value at the midpoint of the wanted sphere
  IndexType                                 m_SphereMidpoint;        //midpoint of the wanted sphere
  VectorType                                m_SigmaX;                //deviation in the x-axis
  VectorType                                m_SigmaY;                //deviation in the y-axis
  VectorType                                m_SigmaZ;                //deviation in the z-axis
  VectorType                                m_CenterX;               //x-coordinate of the mean value of Gaussians
  VectorType                                m_CenterY;               //y-coordinate of the mean value of Gaussians
  VectorType                                m_CenterZ;               //z-coordinate of the mean value of Gaussians
  VectorType                                m_Altitude;              //amplitude
  ItkVectorType                             m_RegionOfInterestMax;   // maximal values for the coordinates in the region of interest
  ItkVectorType                             m_RegionOfInterestMin;   // minimal values for the coordinates in the region of interest
  typename TOutputImage::PixelType          m_Min;                   //minimum possible value
  typename TOutputImage::PixelType          m_Max;                   //maximum possible value
  PointType                                 m_GlobalCoordinate;      // physical coordiante of the sphere midpoint
  bool                                      m_dispVol;

  MapContainerPoints                        m_Midpoints;
  MapContainerRadius                        m_RadiusCuboid;
  double                                    m_Volume;
  VectorType                                m_XCoordToTest, m_YCoordToTest, m_ZCoordToTest;
  double                                    m_NormalDistValues [410];
  double                                    m_meanValueTemp;
  // The following variables are deprecated, and provided here just for
  // backward compatibility. It use is discouraged.
  mutable PointValueArrayType               m_OriginArray;
  mutable SpacingValueArrayType             m_SpacingArray;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiGaussianImageSource.hxx"
#endif

#endif
