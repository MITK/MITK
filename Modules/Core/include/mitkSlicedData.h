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


#ifndef SLICEDDATA_H_HEADER_INCLUDED
#define SLICEDDATA_H_HEADER_INCLUDED

#include <MitkCoreExports.h>
#include "mitkBaseData.h"
#include "mitkSlicedGeometry3D.h"
#include "itkIndex.h"
#include "itkOffset.h"
#include "itkSize.h"
#include "itkImageRegion.h"

namespace mitk {

class SlicedGeometry3D;

//##Documentation
//## @brief Super class of data objects consisting of slices
//##
//## Super class of data objects consisting of slices, e.g., images or a stack
//## of contours. (GetGeometry will return a BaseGeometry containing PlaneGeometry
//## objects).
//##
//## SlicedData-objects have geometries of type SlicedGeometry3D or sub-classes.
//## @ingroup Data
class MITKCORE_EXPORT SlicedData : public BaseData
{
public:
  mitkClassMacro(SlicedData, BaseData);

  itkStaticConstMacro(RegionDimension, unsigned int, 5);

  /** Region typedef support. A region is used to specify a subset of a @a SlicedData. */
  typedef itk::ImageRegion<RegionDimension>  RegionType;

  /** Index typedef support. An index is used to access pixel values. */
  typedef itk::Index<RegionDimension>  IndexType;
  typedef IndexType::IndexValueType  IndexValueType;

  /** Offset typedef support. An offset represent relative position
  * between indices. */
  typedef itk::Offset<RegionDimension>  OffsetType;
  typedef OffsetType::OffsetValueType OffsetValueType;

  /** Size typedef support. A size is used to define region bounds. */
  typedef itk::Size<RegionDimension>  SizeType;
  typedef SizeType::SizeValueType SizeValueType;

  //##Documentation
  //## Update the information for this DataObject so that it can be used as
  //## an output of a ProcessObject.  This method is used in the pipeline
  //## mechanism to propagate information and initialize the meta data
  //## associated with a itk::DataObject.  Any implementation of this method
  //## in a derived class of itk::DataObject is assumed to call its source's
  //## ProcessObject::UpdateOutputInformation() which determines modified
  //## times, LargestPossibleRegions, and any extra meta data like spacing,
  //## origin, etc.
  virtual void UpdateOutputInformation() override;

  virtual void PrepareForNewData() override;

  //##Documentation
  //## Set the RequestedRegion to the LargestPossibleRegion.  This forces a
  //## filter to produce all of the output in one execution (i.e. not
  //## streaming) on the next call to Update().
  virtual void SetRequestedRegionToLargestPossibleRegion() override;

  //##Documentation
  //## Determine whether the RequestedRegion is outside of the
  //## BufferedRegion. This method returns true if the RequestedRegion is
  //## outside the BufferedRegion (true if at least one pixel is outside).
  //## This is used by the pipeline mechanism to determine whether a filter
  //## needs to re-execute in order to satisfy the current request.  If the
  //## current RequestedRegion is already inside the BufferedRegion from the
  //## previous execution (and the current filter is up to date), then a
  //## given filter does not need to re-execute
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

  //##Documentation
  //## @brief Verify that the RequestedRegion is within the
  //## LargestPossibleRegion.
  //##
  //## Verify that the RequestedRegion is within the LargestPossibleRegion.
  //## If the RequestedRegion is not within the LargestPossibleRegion,
  //## then the filter cannot possibly satisfy the request. This method
  //## returns true if the request can be satisfied (even if it will be
  //## necessary to process the entire LargestPossibleRegion) and
  //## returns false otherwise.  This method is used by
  //## PropagateRequestedRegion().  PropagateRequestedRegion() throws a
  //## InvalidRequestedRegionError exception if the requested region is
  //## not within the LargestPossibleRegion.
  virtual bool VerifyRequestedRegion() override;

  //##Documentation
  //## Set the requested region from this data object to match the requested
  //## region of the data object passed in as a parameter.  This method is
  //## implemented in the concrete subclasses of DataObject.
  virtual void SetRequestedRegion( const itk::DataObject *data) override;

  //##Documentation
  //## Set the requested region from this data object to match the requested
  //## region of the data object passed in as a parameter.  This method is
  //## implemented in the concrete subclasses of DataObject.
  virtual void SetRequestedRegion(SlicedData::RegionType *region);

  /*! Documentation
  \brief Sets the largest possible region.
  The largest possible region is the entire region occupied by the data object.
  Note that the largest possible region should always be bigger then the requested region
  of a certain operation.*/
  void SetLargestPossibleRegion(SlicedData::RegionType *region);

  const RegionType& GetLargestPossibleRegion() const
  {
    return m_LargestPossibleRegion;
  }

  //##Documentation
  //## Get the region object that defines the size and starting index
  //## for the region of the image requested (i.e., the region of the
  //## image to be operated on by a filter).
  virtual const RegionType& GetRequestedRegion() const
  {
    return m_RequestedRegion;
  }

  virtual bool IsSliceSet(int s = 0, int t = 0, int n = 0) const = 0;
  virtual bool IsVolumeSet(int t = 0, int n = 0) const = 0;
  virtual bool IsChannelSet(int n = 0) const = 0;
  virtual void CopyInformation(const itk::DataObject *data) override;

  //##Documentation
  //## @brief Get the number of channels
  unsigned int GetNumberOfChannels() const
  {
    return m_LargestPossibleRegion.GetSize(4);
  }

  ////##Documentation
  ////## @brief Return the PlaneGeometry of the slice (@a s, @a t).
  ////##
  ////## The method does not simply call GetGeometry()->GetPlaneGeometry(). Before doing this, it
  ////## makes sure that the PlaneGeometry is up-to-date before returning it (by
  ////## setting the update extent appropriately and calling
  ////## UpdateOutputInformation).
  ////##
  ////## @warning GetPlaneGeometry not yet completely implemented.
  ////## @todo Appropriate setting of the update extent is missing.
  //virtual const mitk::PlaneGeometry* GetPlaneGeometry(int s, int t=0) const;

  //##Documentation
  //## @brief Convenience access method for the geometry, which is of type SlicedGeometry3D (or a sub-class of it).
  //##
  //## @em No update will be called. Normally used in GenerateOutputInformation of
  //## subclasses of BaseProcess.
  SlicedGeometry3D* GetSlicedGeometry(unsigned int t=0) const;

  //##Documentation
  //## @brief Convenience access method for the geometry, which is of type SlicedGeometry3D (or a sub-class of it).
  //##
  //## The method does not simply return the value of the m_Geometry3D member.
  //## Before doing this, it makes sure that the BaseGeometry is up-to-date before
  //## returning it (by setting the update extent appropriately and calling
  //## UpdateOutputInformation).
  //##
  //## @warning GetGeometry not yet completely implemented.
  //## @todo Appropriate setting of the update extent is missing.
  const SlicedGeometry3D* GetUpdatedSlicedGeometry(unsigned int t=0);

  //##Documentation
  //## @brief Set the BaseGeometry of the data, which will be referenced (not copied!). It
  //## has to be a sub-class of SlicedGeometry3D.
  //##
  //## @warning This method will normally be called internally by the sub-class of SlicedData
  //## during initialization.
  virtual void SetGeometry(BaseGeometry* aGeometry3D) override;

  //##Documentation
  //## @brief Convenience method for setting the origin of
  //## the SlicedGeometry3D instances of all time steps
  //##
  //## In case the SlicedGeometry3D is evenly spaced,
  //## the origin of the first slice is set to \a origin.
  //## \sa mitk::BaseData::SetOrigin
  virtual void SetOrigin(const Point3D& origin) override;

  //##Documentation
  //## @brief Convenience method for setting the spacing of
  //## the SlicedGeometry3D instances of all time steps
  virtual void SetSpacing(const ScalarType aSpacing[]);

  //##Documentation
  //## @brief Convenience method for setting the spacing of
  //## the SlicedGeometry3D instances of all time steps
  virtual void SetSpacing(mitk::Vector3D aSpacing);

protected:

  SlicedData();
  SlicedData(const SlicedData &other);
  virtual ~SlicedData();
  RegionType m_LargestPossibleRegion;

  mutable RegionType  m_RequestedRegion;
  mutable bool        m_RequestedRegionInitialized;

  RegionType          m_BufferedRegion;
  bool                m_UseLargestPossibleRegion;
};

} // namespace mitk

#endif /* SLICEDDATA_H_HEADER_INCLUDED */
