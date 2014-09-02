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

This class defines an 3-dimensional Image, in which the value at one voxel equals the value of a multigaussian function evaluated at the voxel's coordinates. The multigaussian function is built as a sum of N gaussian function and is defined by the following parameters (\ref Generation-of-a-multigauss-image):

1. CenterX, CenterY, CenterZ - vectors of the size of N determining the expectancy value at the x-, y- and the z-axis. That means: The i-th gaussian bell curve takes its maximal value at the voxel with index [CenterX(i); CenterY(i); Centerz(i)].

2. SigmaX, SigmaY, SigmaZ - vectors of the size of N determining the deviation at the x-, y- and the z-axis. That means: The width of the i-th gaussian bell curve is determined by the deviation in the x-axis, which is SigmaX(i), in the y-axis is SigmaY(i) and in the z-axis is SigmaZ(i).

3. Altitude - vector of the size of N determining the altitude: the i-th gaussian bell curve has a height of Altitude(i).
This class allows by the method CalculateMidpointAndMeanValue() to find a sphere with a specified radius that has a maximal mean value over all sphere with that radius with midpoint inside or on the boundary of the image. Furthermore it can calculate the maximal und minimal pixel intensities and whose indices in the founded sphere.

To serve as a test tool for ImageStatisticsCalculator, esp. the "hotspot search" feature of this class, MultiGaussianImageSource is also able to calculate the position of a sphere that maximizes the mean value of the voxels within the sphere (\ref Algorithm-for-calculating-statistic-in-a-sphere).

\section Generation-of-a-multigauss-image Generation of a multigauss image

A multigauss function consists of the sum of \f$ N \f$ gauss function. The \f$ i \f$-th \linebreak (\f$0 \leq i \leq N \f$) gaussian is described with the following seven parameters (see above):
- \f$ x_0^{(i)} \f$ is the expectancy value in the \f$ x \f$-Axis
- \f$ y_0^{(i)} \f$ is the expectancy value in the \f$ y \f$-Axis
- \f$ z_0^{(i)} \f$ is the expectancy value in the \f$ z \f$-Axis
- \f$ \sigma_x^{(i)} \f$ is the deviation in the \f$ x \f$-Axis
- \f$ \sigma_y^{(i)} \f$ is the deviation in the \f$ y \f$-Axis
- \f$ \sigma_z^{(i)} \f$ is the deviation in the \f$ z \f$-Axis
- \f$ a^{(i)} \f$ is the  altitude of the gaussian.

A gauss function has the following form:

\f{eqnarray}{
\nonumber
f^{(i)}(x,y,z) = a^{(i)}
exp \left[ - \left(
\frac{(x - x_0^{(i)})^2}{2 (\sigma_x^{(i)})^2} +
\frac{(y - y_0^{(i)})^2}{2 (\sigma_y^{(i)})^2} +
\frac{(z - z_0^{(i)})^2}{2 (\sigma_z^{(i)})^2}
\right) \right].
\f}

A multigauss function has then the form:

\f{align*}{
f_N(x,y,z) =& \sum_{i=0}^{N}f^{(i)}(x,y,z)\\
=&\sum_{0}^{N} a^{(i)}
exp \left[ - \left(
\frac{(x - x_0^{(i)})^2}{2 (\sigma_x^{(i)})^2} +
\frac{(y - y_0^{(i)})^2}{2 (\sigma_y^{(i)})^2} +
\frac{(z - z_0^{(i)})^2}{2 (\sigma_z^{(i)})^2}
\right) \right].
\f}

The multigauss function \f$f_N\f$ will be evaluated at each voxel coordinate to become the voxel intensity.

\section Algorithm-for-calculating-statistic-in-a-sphere Algorithm for calculating statistic in a sphere

This section explains how we can find a sphere region which has a maximal mean value over all sphere regions with a fixed radius. Furthermore we want to calculate the maximal and minimal value in the wanted sphere.

To calculate the mean value in a sphere we integrate the gaussians over the whole sphere. The antiderivative is unknown as an explicit function, but we have a value table for the distribution function of the normal distribution \f$ \Phi(x) \f$ for \f$ x \f$ between \f$ -3.99 \f$ and \f$ 3.99 \f$ with step size \f$ 0.01 \f$. The only problem is that we cannot integrate over a spherical region, because we have an 3-dim integral and therefore are the integral limits dependent from each other and we cannot evaluate \f$ \Phi \f$. So we approximate the sphere with cuboids inside the sphere and prisms on the boundary of the sphere. We calculate these cuboids with the octree recursive method: We start by subdividing the wrapping box of the sphere in eight cuboids. Further we subdivide each cuboid in eight cuboids and check for each of them, whether it is inside or outside the sphere or whether it intersects the sphere surface. We save those of them, which are inside the sphere and continue to subdivide the cuboids that intersect the sphere until the recursion breaks. In the last step we take the half of the cuboids on the boundary and this are the prisms. Now we can calculate and sum the integrals over the cuboids and divide through the volume of the body to obtain the mean value.

For each cuboid \f$ Q = [a_1, b_1]\times[a_2, b_2]\times[a_3, b_3] \f$ we apply  Fubini's theorem for integrable functions and become for the integral the following:

\f{align*}{
m_k =& \sum_{i=0}^{N} \int_{Q} f^{(i)}(x,y,z)dx\\
=&\sum_{i=0}^{N} a^{(i)} \int_{Q}
exp \left[ - \left(
\frac{(x - x_0^{(i)})^2}{2 (\sigma_x^{(i)})^2} +
\frac{(y - y_0^{(i)})^2}{2 (\sigma_y^{(i)})^2} +
\frac{(z - z_0^{(i)})^2}{2 (\sigma_z^{(i)})^2}
\right) \right] dx \\
=& \sum_{i=0}^{N} a^{(i)} \int_{a_1}^{b_1} exp \left[ - \frac{(x - x_0^{(i)})^2}{2 (\sigma_x^{(i)})^2} \right] dx
 \int_{a_2}^{b_2}exp \left[ -\frac{(y - y_0^{(i)})^2}{2 (\sigma_y^{(i)})^2} \right]dx
 \int_{a_3}^{b_3}exp \left[ -\frac{(z - z_0^{(i)})^2}{2 (\sigma_z^{(i)})^2} \right]dx.
\f}

So we calculate three one dimensional integrals:
\f{align*}{
\int_{a}^{b} & exp \left[ - \frac{(x - x_0^{(i)})^2}{2 (\sigma_x^{(i)})^2} \right] dx \\
=&\int_{-\infty}^{b} exp \left[ - \frac{(x - x_0^{(i)})^2}{2 (\sigma_x^{(i)})^2} \right] dx - \int_{-\infty}^{a} exp \left[ - \frac{(x - x_0^{(i)})^2}{2 (\sigma_x^{(i)})^2} \right] dx \\
=& \sigma_x^{(i)} \left[\int_{-\infty}^{(a - x_0^{(i)})/ \sigma_x^{(i)}} e^{-\frac{t^2}{2}} dt
 - \int_{-\infty}^{(b - x_0^{(i)})/ \sigma_x^{(i)}} e^{-\frac{t^2}{2}}dt \right] \\
=&\sigma_x^{(i)} \sqrt{(\pi)} \left[ \Phi \left( \frac{(a - x_0^{(i)})^2}{\sigma_x^{(i)}} \right) - \Phi \left ( \frac{(b - x_0^{(i)})^2}{\sigma_x^{(i)}} \right) \right].
\f}

and become for the integral over \f$ Q \f$:

\f{align*}{
m_k =& \sum_{i=0}^{N} \sigma_x^{(i)} \sigma_y^{(i)} \sigma_z^{(i)} \pi^{1.5}
 \left[ \Phi \left( \frac{(a_1 - x_0^{(i)})^2}{\sigma_x^{(i)}} \right) - \Phi \left ( \frac{(b_1 - x_0^{(i)})^2}{\sigma_x^{(i)}} \right) \right]\times \\
  &\left[ \Phi \left( \frac{(a_2 - y_0^{(i)})^2}{\sigma_y^{(i)}} \right) - \Phi \left ( \frac{(b_2 - y_0^{(i)})^2}{\sigma_y^{(i)}} \right) \right]\times
   \left[ \Phi \left( \frac{(a_3 - z_0^{(i)})^2}{\sigma_z^{(i)}} \right) - \Phi \left ( \frac{(b_3 - z_0^{(i)})^2}{\sigma_z^{(i)}} \right) \right].
\f}

For the integral over the prism we take the half of the integral over the corresponding cuboid.

Altogether we find the mean value in the sphere as:
\f{align*}{
\left( \sum_{Q_k \text{ Cuboid}} m_k + \sum_{P_l \text{ Prism}} 0.5 m_l \right )/Volume(B),
\f}

where Volume(B) is the volume of the body that approximate the sphere.

Now we know how to calculate the mean value in a sphere for given midpoint and radius. So we assume each voxel in the given image to be the sphere midpoint and we calculate the mean value as described above. If the new mean value is greater than the "maximum-until-now", we take the new value to be the "maximum-until-now". Then we go to the next voxel and make the same calculation and so on. At the same time we save the coordinates  of the midpoint voxel.

After we found the midpoint and the maximal mean value, we can calculate the maximum and the minimum in the sphere: we just traverse all the voxels in the region and take the maximum and minimum value and the respective coordinates.

\section  Input-and-output Input and output

An example for an input in the command-line is:
\verbatim
  mitkMultiGaussianTest C:/temp/outputFile C:/temp/inputFile.xml
\endverbatim

Here is outputFile the name of the gaussian image with extension .nrrd and at the same time the name of the output file with extension .xml, which is the same as the inputFile, only added the calculated mean value, max and min and the corresponding indexes in the statistic tag. Here we see an example for the input and output .xml file:

\verbatim
INPUT:

<testcase>
  <testimage image-rows="20" image-columns="20" image-slices="20" numberOfGaussians="2" spacingX="1" spacingY="1" spacingZ="1" entireHotSpotInImage="1">
    <gaussian centerIndexX="4" centerIndexY="16" centerIndexZ="10" deviationX="7" deviationY="7" deviationZ="7" altitude="200"/>
    <gaussian centerIndexX="18" centerIndexY="2" centerIndexZ="10" deviationX="1" deviationY="1" deviationZ="1" altitude="210"/>
  </testimage>
  <segmentation numberOfLabels="1" hotspotRadiusInMM="6.2035">
    <roi label="1" maximumSizeX="20" minimumSizeX="0" maximumSizeY="20" minimumSizeY="0" maximumSizeZ="20" minimumSizeZ="0"/>
  </segmentation>
</testcase>
\endverbatim

\verbatim
OUTPUT:

<testcase>
  <testimage image-rows="20" image-columns="20" image-slices="20" numberOfGaussians="2" spacingX="1" spacingY="1" spacingZ="1" entireHotSpotInImage="1">
    <gaussian centerIndexX="4" centerIndexY="16" centerIndexZ="10" deviationX="7" deviationY="7" deviationZ="7" altitude="200"/>
    <gaussian centerIndexX="18" centerIndexY="2" centerIndexZ="10" deviationX="1" deviationY="1" deviationZ="1" altitude="210"/>
  </testimage>
  <segmentation numberOfLabels="1" hotspotRadiusInMM="6.2035">
    <roi label="1" maximumSizeX="20" minimumSizeX="0" maximumSizeY="20" minimumSizeY="0" maximumSizeZ="20" minimumSizeZ="0"/>
  </segmentation>
  <statistic hotspotIndexX="6" hotspotIndexY="13" hotspotIndexZ="10" peak="141.544" mean="141.544" maximumIndexX="4" maximumIndexY="16" maximumIndexZ="10" maximum="200" minimumIndexX="9" minimumIndexY="8" minimumIndexZ="8"  minimum="77.4272"/>
</testcase>
\endverbatim

\subsection  Parameter-for-the-input Parameter for the input

In the tag \a testimage we describe the image that we generate. Image rows/columns/slices gives the number of rows/columns/slices of the image; \a numberOfGaussians is  the number of gauss functions (\f$ N \f$); spacing defines the extend of one voxel for each direction. The parameter \a entireHotSpotInImage determines whether the whole sphere is in the image included (\f$ = 1 \f$) or only the midpoint of the sphere is inside the image.

NOTE: When the \a entireHotSpotInImage \f$ = 0 \f$ it is possible that we find the midpoint of the sphere on the border of the image. In this case we cut the approximation of the sphere, so that we become a body, which is completely inside the image, but not a "sphere" anymore. To that effect is the volume of the body decreased and that could lead to unexpected results.

In the subtag \a gaussian we describe each gauss function as mentioned in the second section.

In the tag \a segmentation we define the radius of the wanted sphere in mm (\a hotspotRadiusInMM ). We can also set the number of labels (\a numberOfLabels ) to be an positive number and this determines the number of regions of interest (ROI). In each ROI we find the sphere with the wanted properties and midpoint inside the ROI, but not necessarily the whole sphere. In the subtag \a roi we set label number and the index coordinates for the borders of the roi.

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


  /** Set/Get size of the output image. */
  itkSetMacro(Size, SizeType);
  virtual void SetSize(SizeValueArrayType sizeArray);
  virtual const SizeValueType * GetSize() const;
  /** Set/Get spacing of the output image. */
  itkSetMacro(Spacing, SpacingType);
  virtual void SetSpacing(SpacingValueArrayType spacingArray);
  virtual const SpacingValueType * GetSpacing() const;
  /** Set/Get origin of the output image. This programm works proper only with origin [0.0, 0.0, 0.0] */
  itkSetMacro(Origin, PointType);
  virtual void SetOrigin(PointValueArrayType originArray);
  virtual const PointValueType * GetOrigin() const;
  /** Get the number of gaussian functions in the output image. */
  virtual unsigned int GetNumberOfGaussians() const;
  /** Set the number of gaussian function. */
  virtual void SetNumberOfGausssians( unsigned int );
  /** Set/Get the radius of the sphere. */
  virtual RadiusType GetRadius() const;
  virtual void  SetRadius( RadiusType  radius );
  /** Get the maximal mean value in a sphere over all possible spheres with midpoint in the image. */
  virtual const OutputImagePixelType GetMaxMeanValue() const;
  /** Get the index of the midpoint of a sphere with the maximal mean value.*/
  virtual const IndexType GetSphereMidpoint() const;
  /** Calculates the value of the multigaussian function at a Point given by its coordinates [x, y, z]. */
  virtual double MultiGaussianFunctionValueAtPoint(double , double, double);
  /** Adds a multigaussian defined by the parameter: CenterX, CenterY, CenterZ, SigmaX, SigmaY, SigmaZ, Altitude.
   All parameters should have the same size, which determinates the number of the gaussian added. */
  virtual void AddGaussian( VectorType centerX, VectorType centerY, VectorType centerZ, VectorType sigmaX, VectorType sigmaY, VectorType sigmaZ, VectorType altitude);
  /** Calculates and set the index of the midpoint of the sphere with the maximal mean value as well as the mean value. */
  virtual void CalculateTheMidpointAndTheMeanValueWithOctree();
  /** Calculates and set the index an the value of maximulm and minimum in the wanted sphere. */
  virtual void CalculateMaxAndMinInSphere();
  /** Get the index in the sphere with maximal value. */
  virtual const IndexType GetMaxValueIndexInSphere() const;
  /** Get the maximal value in the sphere. */
  virtual const OutputImagePixelType GetMaxValueInSphere() const;
  /** Get the index in the sphere with minimal value. */
  virtual const IndexType GetMinValueIndexInSphere() const;
  /** Get the minimal value in the sphere. */
  virtual const OutputImagePixelType GetMinValueInSphere() const;
  /** Set the region of interest. */
  virtual void SetRegionOfInterest(ItkVectorType, ItkVectorType);
  /** Write a .mps file to visualise the point in the sphere. */
  virtual void WriteXMLToTestTheCuboidInsideTheSphere();
  /**This recursive method realise the octree method. It subdivide a cuboid in eight cuboids, when this cuboid crosses the boundary of sphere. If the cuboid is inside the sphere, it calculates the integral. */
  virtual void CalculateEdgesInSphere( PointType globalCoordinateMidpointCuboid, PointType globalCoordinateMidpointSphere, double cuboidRadius, int level);
  /**Calculate and return value of the integral of the gaussian in a cuboid region with the dimension 3: in the x-axis between xMin and xMax and in the y-axis between yMin and yMax and in the z-axis also between zMin and zMax. */
  virtual double MultiGaussianFunctionValueAtCuboid(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax);
  /** Inseret the midpoints of cuboid in a vector m_Midpoints, so that we can visualise it. */
  virtual void InsertPoints( PointType globalCoordinateMidpointCuboid, double cuboidRadius);
  /** Start the octree recursion in eigth directions for the sphere with midpoint globalCoordinateMidpointSphere. */
  virtual void GenerateCuboidSegmentationInSphere( PointType globalCoordinateMidpointSphere );
  /** Get the  the values of the cumulative distribution function of the normal distribution. */
  virtual double FunctionPhi(double value);
  /** Check if a cuboid with midpoint globalCoordinateMidpointCuboid and side length sideLength intersect the sphere with midpoint globalCoordinateMidpointSphere boundary. */
  virtual  unsigned int IntersectTheSphere( PointType globalCoordinateMidpointCuboid, PointType globalCoordinateMidpointSphere, double sideLength);
  /** Set the tabel values of the distribution function of the normal distribution. */
  void SetNormalDistributionValues();


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
  OutputImagePixelType                      m_ValueAtMidpoint;       //value at the midpoint of the wanted sphere
  IndexType                                 m_SphereMidpoint;        //midpoint of the wanted sphere
  VectorType                                m_SigmaX;                //deviation in the x-axis
  VectorType                                m_SigmaY;                //deviation in the y-axis
  VectorType                                m_SigmaZ;                //deviation in the z-axis
  VectorType                                m_CenterX;               //x-coordinate of the mean value of Gaussians
  VectorType                                m_CenterY;               //y-coordinate of the mean value of Gaussians
  VectorType                                m_CenterZ;               //z-coordinate of the mean value of Gaussians
  VectorType                                m_Altitude;              //amplitude
  ItkVectorType                             m_RegionOfInterestMax;   //maximal values for the coordinates in the region of interest
  ItkVectorType                             m_RegionOfInterestMin;   //minimal values for the coordinates in the region of interest
  typename TOutputImage::PixelType          m_Min;                   //minimum possible value
  typename TOutputImage::PixelType          m_Max;                   //maximum possible value
  PointType                                 m_GlobalCoordinate;      //physical coordiante of the sphere midpoint
  bool                                      m_WriteMPS;              //1 = write a MPS File to visualise the cuboid midpoints of one approximation of the sphere
  MapContainerPoints                        m_Midpoints;             //the midpoints of the cuboids
  MapContainerRadius                        m_RadiusCuboid;          //the radius ( = 0.5 * side length) of the cuboids (in the same order as the midpoints in m_Midpoints)
  double                                    m_Volume;                //the volume of the body, that approximize the sphere
  double                                    m_NormalDistValues [410];//normal distribution values
  double                                    m_meanValueTemp;         //= m_Volume * meanValue in each sphere
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
