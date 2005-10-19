/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef SLICEDDATA_H_HEADER_INCLUDED_C1EBD53D
#define SLICEDDATA_H_HEADER_INCLUDED_C1EBD53D

#include "mitkCommon.h"
#include "mitkBaseData.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include "itkIndex.h"
#include "itkOffset.h"
#include "itkSize.h"
#include "itkImageRegion.h"

#if _MSC_VER < 1300
#define VmitkImageDimension 5
#endif

namespace mitk {

class SlicedGeometry3D;

//##ModelId=3E14379B00E8
//##Documentation
//## @brief Super class of data objects consisting of slices
//##
//## Super class of data objects consisting of slices, e.g., images or a stack
//## of contours. (GetGeometry will return a Geometry3D containing Geometry2D
//## objects).
//## 
//## SlicedData-objects have geometries of type SlicedGeometry3D or sub-classes.
//## @ingroup Data
class SlicedData : public BaseData
{
protected:
#if !(_MSC_VER < 1300)
  //##ModelId=3E144ED20061
  static const int VmitkImageDimension=5;
#endif
public:
  mitkClassMacro(SlicedData, BaseData);

  /** Region typedef support. A region is used to specify a subset of a @a SlicedData. */
  //##ModelId=3E143BF1002F
  typedef itk::ImageRegion<VmitkImageDimension>  RegionType;

  /** Index typedef support. An index is used to access pixel values. */
  //##ModelId=3E144ED10331
  typedef itk::Index<VmitkImageDimension>  IndexType;
  //##ModelId=3E144ED10346
  typedef IndexType::IndexValueType  IndexValueType;

  /** Offset typedef support. An offset represent relative position
  * between indices. */
  //##ModelId=3E144ED1035A
  typedef itk::Offset<VmitkImageDimension>  OffsetType;
  //##ModelId=3E144ED1036D
  typedef OffsetType::OffsetValueType OffsetValueType;

  /** Size typedef support. A size is used to define region bounds. */
  //##ModelId=3E144ED10377
  typedef itk::Size<VmitkImageDimension>  SizeType;
  //##ModelId=3E144ED10382
  typedef SizeType::SizeValueType SizeValueType;

  //##ModelId=3E141028018A
  //##Documentation
  //## Update the information for this DataObject so that it can be used as
  //## an output of a ProcessObject.  This method is used in the pipeline
  //## mechanism to propagate information and initialize the meta data
  //## associated with a itk::DataObject.  Any implementation of this method
  //## in a derived class of itk::DataObject is assumed to call its source's
  //## ProcessObject::UpdateOutputInformation() which determines modified
  //## times, LargestPossibleRegions, and any extra meta data like spacing,
  //## origin, etc.
  virtual void UpdateOutputInformation();

  virtual void PrepareForNewData();

  //##ModelId=3E14102C029E
  //##Documentation
  //## Set the RequestedRegion to the LargestPossibleRegion.  This forces a
  //## filter to produce all of the output in one execution (i.e. not
  //## streaming) on the next call to Update().
  virtual void SetRequestedRegionToLargestPossibleRegion();

  //##ModelId=3E14104300AC
  //##Documentation
  //## Determine whether the RequestedRegion is outside of the
  //## BufferedRegion. This method returns true if the RequestedRegion is
  //## outside the BufferedRegion (true if at least one pixel is outside).
  //## This is used by the pipeline mechanism to determine whether a filter
  //## needs to re-execute in order to satisfy the current request.  If the
  //## current RequestedRegion is already inside the BufferedRegion from the
  //## previous execution (and the current filter is up to date), then a
  //## given filter does not need to re-execute
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

  //##ModelId=3E14105B00F7
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
  virtual bool VerifyRequestedRegion();

  //##ModelId=3E1410760114
  //##Documentation
  //## Set the requested region from this data object to match the requested
  //## region of the data object passed in as a parameter.  This method is
  //## implemented in the concrete subclasses of DataObject.
  virtual void SetRequestedRegion(itk::DataObject *data);

  //##Documentation
  //## Set the requested region from this data object to match the requested
  //## region of the data object passed in as a parameter.  This method is
  //## implemented in the concrete subclasses of DataObject.
  virtual void SetRequestedRegion(SlicedData::RegionType *region);

  const RegionType& GetLargestPossibleRegion() const 
  {
    return m_LargestPossibleRegion;
  }

  //##ModelId=3E144ED20089
  //##Documentation
  //## Get the region object that defines the size and starting index
  //## for the region of the image requested (i.e., the region of the
  //## image to be operated on by a filter).
  virtual const RegionType& GetRequestedRegion() const
  {
    return m_RequestedRegion;
  }

  //##ModelId=3E14713503B7
  virtual bool IsSliceSet(int s = 0, int t = 0, int n = 0) const = 0;
  //##ModelId=3E147163027C
  virtual bool IsVolumeSet(int t = 0, int n = 0) const = 0;
  //##ModelId=3E1550BD0272
  virtual bool IsChannelSet(int n = 0) const = 0;
  //##ModelId=3E34513B016D
  virtual void CopyInformation(const itk::DataObject *data);

  //##Documentation
  //## @brief Get the number of channels
  unsigned int GetNumberOfChannels() const
  {
    return m_LargestPossibleRegion.GetSize(4);
  }

  ////##Documentation
  ////## @brief Return the Geometry2D of the slice (@a s, @a t). 
  ////## 
  ////## The method does not simply call GetGeometry()->GetGeometry2D(). Before doing this, it
  ////## makes sure that the Geometry2D is up-to-date before returning it (by
  ////## setting the update extent appropriately and calling
  ////## UpdateOutputInformation).
  ////## 
  ////## @warning GetGeometry2D not yet completely implemented.
  ////## @todo Appropriate setting of the update extent is missing.
  //virtual const mitk::Geometry2D* GetGeometry2D(int s, int t=0) const;

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
  //## Before doing this, it makes sure that the Geometry3D is up-to-date before 
  //## returning it (by setting the update extent appropriately and calling
  //## UpdateOutputInformation).
  //## 
  //## @warning GetGeometry not yet completely implemented. 
  //## @todo Appropriate setting of the update extent is missing.
  const SlicedGeometry3D* GetUpdatedSlicedGeometry(unsigned int t=0);

  //##Documentation
  //## @brief Set the Geometry3D of the data, which will be referenced (not copied!). It
  //## has to be a sub-class of SlicedGeometry3D.
  //## 
  //## @warning This method will normally be called internally by the sub-class of SlicedData 
  //## during initialization.
  virtual void SetGeometry(Geometry3D* aGeometry3D);

  //##Documentation
  //## @brief release memory
  virtual void ReleaseData();

protected:
  //##ModelId=3E19EA3300BA
  SlicedData();
  //##ModelId=3E19EA3300CE
  virtual ~SlicedData();

  RegionType m_LargestPossibleRegion;

  //##ModelId=3E143BF10135
  RegionType          m_RequestedRegion;
  //##ModelId=3E143BF10149
  RegionType          m_BufferedRegion;
  //##ModelId=3E144ED20075
  bool                m_UseLargestPossibleRegion;
};

} // namespace mitk

#endif /* SLICEDDATA_H_HEADER_INCLUDED_C1EBD53D */
