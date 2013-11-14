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

namespace itk
{
/** \class MultiGaussianImageSource
 * \brief Generate an 3-dimensional multigaussian image.
 * This class defines an 3-dimensional Image, in which the value at one voxel equals the value of a multigaussian function evaluated at the voxel's   * coordinates. The multigaussian function is build as a sum of N gaussian function. This is defined by the following parameters:
 *    1. CenterX, CenterY, CenterZ - vectors of the size of N determining the expectancy value at the x-, y- and the z-axis. That means: The i-th
 * gaussian bell curve takes its maximal value at the voxel with index [CenterX(i); CenterY(i); Centerz(i)].
 *    2. SigmaX, SigmaY, SigmaZ - vectors of the size of N determining the deviation at the x-, y- and the z-axis. That means: The width of the i-th
 * gaussian bell curve in the x-axis is SigmaX(i), in the y-axis is SigmaY(i) and in the z-axis is SigmaZ(i).
 *    3. Altitude - vector of the size of N determining the altitude: the i-th gaussian bell curve has a height of Altitude(i).
 * This class allows by the method CalculateMidpointAndMeanValue() to find a sphere with a specified radius that has a maximal mean value over all    * sphere with that radius with midpoint inside or at the boundary of the image.
 *
 * \ingroup DataSources
 * \ingroup ITKTestKernel
 *
 * \wiki
 * \wikiexample{}
 * \endwiki
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
  /** Typedef to describe the sphere radius type. */
  typedef double                                              RadiusType;
  /** Typedef to describe the standard vector type. */
  typedef std::vector<double>                                 VectorType;
  /** Typedef to describe the itk vector type. */
  typedef itk::Vector<double, TOutputImage::ImageDimension>   ItkVectorType;
  /** Typedef to describe the ImageRegionIteratorWithIndex type. */
  typedef ImageRegionIteratorWithIndex<TOutputImage>          IteratorType;
  /** Typedef to describe the Poiner type at the output image. */
  typedef typename TOutputImage::Pointer                      ImageType;

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
  virtual const double GetMaxMeanValue() const;
  /** Get the index of the midpoint of a sphere with the maximal mean value */
  virtual const ItkVectorType GetSphereMidpoint() const;
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
  virtual const ItkVectorType GetMaxValueIndexInSphere() const;
  /** Get the maximal value in the sphere*/
  virtual const double GetMaxValueInSphere() const;
  /** Get the index in the sphere with minimal value*/
  virtual const ItkVectorType GetMinValueIndexInSphere() const;
  /** Get the minimal value in the sphere*/
  virtual const double GetMinValueInSphere() const;
  /** Set the region of interest */
  virtual void SetRegionOfInterest(ItkVectorType, ItkVectorType);
  /** Optimize the mean value in the wanted sphere*/
  virtual void OptimizeMeanValue();
  /** Check whether the point is in the region of interest */
  virtual const bool IsInRegionOfInterest(unsigned int, unsigned int, unsigned int);
  /** Set the minimum possible pixel value. By default, it is
   * NumericTraits<TOutputImage::PixelType>::min(). */
  itkSetClampMacro( Min, OutputImagePixelType,
                    NumericTraits< OutputImagePixelType >::NonpositiveMin(),
                    NumericTraits< OutputImagePixelType >::max() );

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

  SizeType              m_Size;                  //size of the output image
  SpacingType           m_Spacing;               //spacing
  PointType             m_Origin;                //origin
  OutputImagePixelType  m_MaxValueInSphere;      //maximal value in the wanted sphere
  ItkVectorType         m_MaxValueIndexInSphere; //index of the maximal value in the wanted sphere
  OutputImagePixelType  m_MinValueInSphere;      //minimal value in the wanted sphere
  ItkVectorType         m_MinValueIndexInSphere; //index of the minimal value in the wanted sphere
  unsigned int          m_NumberOfGaussians;     //number of Gaussians
  RadiusType            m_Radius;                //radius of the sphere
  unsigned int          m_RadiusStepNumber;      //number of steps to traverse the sphere radius
  OutputImagePixelType  m_MeanValue;             //mean value in the wanted sphere
  ItkVectorType         m_SphereMidpoint;        //midpoint of the wanted sphere
  VectorType            m_SigmaX;                //deviation in the x-axis
  VectorType            m_SigmaY;                //deviation in the y-axis
  VectorType            m_SigmaZ;                //deviation in the z-axis
  VectorType            m_CenterX;               //x-coordinate of the mean value of Gaussians
  VectorType            m_CenterY;               //y-coordinate of the mean value of Gaussians
  VectorType            m_CenterZ;               //z-coordinate of the mean value of Gaussians
  VectorType            m_Altitude;              //amplitude
  ItkVectorType         m_RegionOfInterestMax;   // maximal values for the coordinates in the region of interest
  ItkVectorType         m_RegionOfInterestMin;   // minimal values for the coordinates in the region of interest
  typename TOutputImage::PixelType m_Min;        //minimum possible value
  typename TOutputImage::PixelType m_Max;        //maximum possible value

  // The following variables are deprecated, and provided here just for
  // backward compatibility. It use is discouraged.
  mutable PointValueArrayType   m_OriginArray;
  mutable SpacingValueArrayType m_SpacingArray;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiGaussianImageSource.hxx"
#endif

#endif
