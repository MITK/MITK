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
#ifndef itkCurvilinearArraySpecialCoordinatesImage_h
#define itkCurvilinearArraySpecialCoordinatesImage_h

#include "itkSpecialCoordinatesImage.h"
#include "itkPoint.h"
#include "vnl/vnl_math.h"
#include "itkNeighborhoodAccessorFunctor.h"

namespace itk
{
/** \class CurvilinearArraySpecialCoordinatesImage
 *
 *  \brief Templated 2D nonrectilinear-coordinate image class for
 *  curvilinear/phased-array "range" images.
 *
 * \verbatim
 *
 *                             +---------------------> x-axis
 *                             |\
 *                          /  | \
 *                             |-~\
 *                       /     |   \
 *                             |    \
 *                    /        |     \
 *                             | lateral
 *                             |
 *                             v y-axis
 *
 * \endverbatim
 *
 * The equations form performing the conversion from Cartesian coordinates to
 * curvilinear/phased array coordinates are as follows:
 *
 * lateral = arctan(x/y)
 * radius = std::sqrt(x^2 + y^2)
 *
 * The reversed transforms are:
 *
 * x = radius * std::sin(lateral)
 * y = radius * std::cos(lateral)
 *
 * CurvilinearArraySpecialCoordinatesImages are templated over a pixel
 * type and follow the SpecialCoordinatesImage interface.  The data in
 * an image is  arranged in a 1D array as
 * [lateral-index][radius-index] with radius-index
 * varying most rapidly.  The Index type reverses the order so that
 * Index[0] = radius-index, Index[1] = lateral-index.
 *
 * Lateral is discretized into m_LateralAngularSeparation intervals
 * per angular voxel, the most negative lateral interval containing
 * data is then mapped to lateral-index=0, and the largest lateral
 * interval containing data is then mapped to lateral-index=( number
 * of samples along lateral axis - 1 ). Elevation is discretized in
 * the same manner.  This way, the mapping to Cartesian space is
 * symmetric about the x axis such that the line defined by
 * lateral/2 = x-axis.  Radius is discretized into
 * m_RadiusSampleSize units per angular voxel.  The smallest range
 * interval containing data is then mapped to radius-index=0, such
 * that radius = m_FirstSampleDistance + (radius-index *
 * m_RadiusSampleSize).
 *
 * \sa SpecialCoordinatesImage
 * \sa PhasedArray3DSpecialCoordinatesImage
 *
 * \ingroup Ultrasound
 *
 * \ingroup ImageObjects
 * \ingroup ITKCommon
 */
template< typename TPixel, unsigned int VDimension >
class CurvilinearArraySpecialCoordinatesImage:
  public SpecialCoordinatesImage< TPixel, VDimension >
{
public:
  /** Standard class typedefs */
  typedef CurvilinearArraySpecialCoordinatesImage       Self;
  typedef SpecialCoordinatesImage< TPixel, VDimension > Superclass;
  typedef SmartPointer< Self >                          Pointer;
  typedef SmartPointer< const Self >                    ConstPointer;
  typedef WeakPointer< const Self >                     ConstWeakPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(CurvilinearArraySpecialCoordinatesImage, SpecialCoordinatesImage);

  /** Pixel typedef support. Used to declare pixel type in filters
   * or other operations. */
  typedef TPixel PixelType;

  /** Typedef alias for PixelType */
  typedef TPixel ValueType;

  /** Internal Pixel representation. Used to maintain a uniform API
   * with Image Adaptors and allow to keep a particular internal
   * representation of data while showing a different external
   * representation. */
  typedef TPixel InternalPixelType;

  typedef typename Superclass::IOPixelType IOPixelType;

  /** Accessor type that convert data between internal and external
   *  representations.  */
  typedef DefaultPixelAccessor< PixelType > AccessorType;

  /** Accessor functor to choose between accessors: DefaultPixelAccessor for
   * the Image, and DefaultVectorPixelAccessor for the vector image. The
   * functor provides a generic API between the two accessors. */
  typedef DefaultPixelAccessorFunctor< Self > AccessorFunctorType;

  /** Typedef for the functor used to access a neighborhood of pixel
   * pointers. */
  typedef NeighborhoodAccessorFunctor< Self > NeighborhoodAccessorFunctorType;

  /** Dimension of the image.  This constant is used by functions that are
   * templated over image type (as opposed to being templated over pixel type
   * and dimension) when they need compile time access to the dimension of
   * the image. */
  itkStaticConstMacro(ImageDimension, unsigned int, VDimension);

  /** Index typedef support. An index is used to access pixel values. */
  typedef typename Superclass::IndexType      IndexType;
  typedef typename Superclass::IndexValueType IndexValueType;

  /** Offset typedef support. An offset is used to access pixel values. */
  typedef typename Superclass::OffsetType    OffsetType;

  /** Size typedef support. A size is used to define region bounds. */
  typedef typename Superclass::SizeType      SizeType;
  typedef typename Superclass::SizeValueType SizeValueType;

  /** Container used to store pixels in the image. */
  typedef ImportImageContainer< SizeValueType, PixelType > PixelContainer;

  /** Region typedef support. A region is used to specify a subset of
   *  an image.
   */
  typedef typename Superclass::RegionType RegionType;

  /** Spacing typedef support.  Spacing holds the "fake" size of a
   *  pixel, making each pixel look like a 1 unit hyper-cube to filters
   *  that were designed for normal images and that therefore use
   *  m_Spacing.  The spacing is the geometric distance between image
   *  samples.
   */
  typedef typename Superclass::SpacingType SpacingType;

  /** Origin typedef support.  The origin is the "fake" geometric
   *  coordinates of the index (0,0).  Also for use w/ filters designed
   *  for normal images.
   */
  typedef typename Superclass::PointType PointType;

  /** A pointer to the pixel container. */
  typedef typename PixelContainer::Pointer      PixelContainerPointer;
  typedef typename PixelContainer::ConstPointer PixelContainerConstPointer;

  /** Graft the data and information from one image to another. This
   * is a convenience method to setup a second image with all the meta
   * information of another image and use the same pixel
   * container. Note that this method is different than just using two
   * SmartPointers to the same image since separate DataObjects are
   * still maintained. This method is similar to
   * ImageSource::GraftOutput(). The implementation in ImageBase
   * simply calls CopyInformation() and copies the region ivars.
   * The implementation here refers to the superclass' implementation
   * and then copies over the pixel container. */
  virtual void Graft(const DataObject *data) ITK_OVERRIDE;

  /** \brief Get the continuous index from a physical point
   *
   * Returns true if the resulting index is within the image, false otherwise.
   * \sa Transform */
  template< typename TCoordRep >
  bool TransformPhysicalPointToContinuousIndex(
    const Point< TCoordRep, VDimension > & point,
    ContinuousIndex< TCoordRep, VDimension > & index) const
  {
    const RegionType & region = this->GetLargestPossibleRegion();
    const double maxLateral = region.GetSize(1) - 1;

    // Convert Cartesian coordinates into angular coordinates
    const TCoordRep lateral = std::atan(point[0] / point[1]);
    const TCoordRep radius  = std::sqrt(point[0] * point[0] + point[1] * point[1] );

    // Convert the "proper" angular coordinates into index format
    index[0] = static_cast< TCoordRep >( ( ( radius - m_FirstSampleDistance )
                                           / m_RadiusSampleSize ) );
    index[1] = static_cast< TCoordRep >( ( lateral / m_LateralAngularSeparation )
                                         + ( maxLateral / 2.0 ) );

    // Now, check to see if the index is within allowed bounds
    const bool isInside = region.IsInside(index);

    return isInside;
  }

  /** Get the index (discrete) from a physical point.
   * Floating point index results are truncated to integers.
   * Returns true if the resulting index is within the image, false otherwise
   * \sa Transform */
  template< typename TCoordRep >
  bool TransformPhysicalPointToIndex(
    const Point< TCoordRep, VDimension > & point,
    IndexType & index) const
  {
    const RegionType & region = this->GetLargestPossibleRegion();
    const double maxLateral = region.GetSize(1) - 1;

    // Convert Cartesian coordinates into angular coordinates
    const TCoordRep lateral = std::atan(point[0] / point[1]);
    const TCoordRep radius  = std::sqrt(point[0] * point[0] + point[1] * point[1] );

    // Convert the "proper" angular coordinates into index format
    index[0] = static_cast< IndexValueType >( ( ( radius - m_FirstSampleDistance )
                                           / m_RadiusSampleSize ) );
    index[1] = static_cast< IndexValueType >( ( lateral / m_LateralAngularSeparation )
                                         + ( maxLateral / 2.0 ) );

    // Now, check to see if the index is within allowed bounds
    const bool isInside = region.IsInside(index);

    return isInside;
  }

  /** Get a physical point (in the space which
   * the origin and spacing information comes from)
   * from a continuous index (in the index space)
   * \sa Transform */
  template< typename TCoordRep >
  void TransformContinuousIndexToPhysicalPoint(
    const ContinuousIndex< TCoordRep, VDimension > & index,
    Point< TCoordRep, VDimension > & point) const
  {
    const RegionType & region = this->GetLargestPossibleRegion();
    const double maxLateral = region.GetSize(1) - 1;

    // Convert the index into proper angular coordinates
    const TCoordRep radius  = ( index[0] * m_RadiusSampleSize ) + m_FirstSampleDistance;
    const TCoordRep lateral = ( index[1] - ( maxLateral / 2.0 ) ) * m_LateralAngularSeparation;

    // Convert the angular coordinates into Cartesian coordinates
    point[0] = static_cast< TCoordRep >( radius * std::sin(lateral) );
    point[1] = static_cast< TCoordRep >( radius * std::cos(lateral) );
  }

  /** Get a physical point (in the space which
   * the origin and spacing information comes from)
   * from a discrete index (in the index space)
   *
   * \sa Transform */
  template< typename TCoordRep >
  void TransformIndexToPhysicalPoint(
    const IndexType & index,
    Point< TCoordRep, VDimension > & point) const
  {
    const RegionType & region = this->GetLargestPossibleRegion();
    const double maxLateral = region.GetSize(1) - 1;

    // Convert the index into proper angular coordinates
    const TCoordRep radius  = ( index[0] * m_RadiusSampleSize ) + m_FirstSampleDistance;
    const TCoordRep lateral = ( index[1] - ( maxLateral / 2.0 ) ) * m_LateralAngularSeparation;

    // Convert the angular coordinates into Cartesian coordinates
    point[0] = static_cast< TCoordRep >( radius * std::sin(lateral) );
    point[1] = static_cast< TCoordRep >( radius * std::cos(lateral) );
  }

  /** Set/Get the number of radians between each lateral unit.   */
  itkSetMacro(LateralAngularSeparation, double);
  itkGetConstMacro(LateralAngularSeparation, double);

  /** Set/Get the number of cartesian units between each unit along the R .  */
  itkSetMacro(RadiusSampleSize, double);
  itkGetConstMacro(RadiusSampleSize, double);

  /** Set the distance to add to the radius. */
  itkSetMacro(FirstSampleDistance, double);
  itkGetConstMacro(FirstSampleDistance, double);

  template< typename TCoordRep >
  void TransformLocalVectorToPhysicalVector(
    FixedArray< TCoordRep, VDimension > & ) const
    {}

  template< typename TCoordRep >
  void TransformPhysicalVectorToLocalVector(
    const FixedArray< TCoordRep, VDimension > & ,
    FixedArray< TCoordRep, VDimension > & ) const
    {}

protected:
  CurvilinearArraySpecialCoordinatesImage()
    {
    m_RadiusSampleSize = 1;
    m_LateralAngularSeparation   =  1 * ( 2.0 * vnl_math::pi / 360.0 ); // 1
                                                                        // degree
    m_FirstSampleDistance = 0;
    }

  virtual ~CurvilinearArraySpecialCoordinatesImage() {}
  virtual void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

private:
  CurvilinearArraySpecialCoordinatesImage(const Self &); // purposely not implemented
  void operator=(const Self &);                          // purposely not implemented

  double m_LateralAngularSeparation;    // in radians
  double m_RadiusSampleSize;
  double m_FirstSampleDistance;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkCurvilinearArraySpecialCoordinatesImage.hxx"
#endif

#endif
