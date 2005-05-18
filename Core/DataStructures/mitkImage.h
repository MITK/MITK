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


#ifndef MITKIMAGE_H_HEADER_INCLUDED_C1C2FCD2
#define MITKIMAGE_H_HEADER_INCLUDED_C1C2FCD2

#include "mitkCommon.h"
#include "mitkSlicedData.h"
#include "mitkImageDataItem.h"
#include "mitkPixelType.h"
#include "mitkBaseData.h"
#include "mitkLevelWindow.h"
#include "mitkPlaneGeometry.h"

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif 

namespace mitk {

class SubImageSelector;
//template <class TPixel, unsigned int VImageDimension=2> class ImageToItk;

//##ModelId=3DCBC1E300FE
//##Documentation
//## @brief Image class for storing images
//## @ingroup Data
//## Can be asked for header information, the
//## data vector, ipPicDescriptors or vtkImageData objects. If not the complete
//## data is required, the appropriate SubImageSelector class should be used
//## for access.
//## Image organizes sets of slices (s x 2D), volumes (t x 3D) and channels (n
//## x ND). Channels are for different kind of data, e.g., morphology in 
//## channel 0, velocities in channel 1. All channels must have the same Geometry! In 
//## particular, the dimensions of all channels are the same, only the pixel-type
//## may differ between channels
class Image : public SlicedData
{
  friend class SubImageSelector;
  //    template <typename, unsigned int> friend class ImageToItk;

public:
  mitkClassMacro(Image, SlicedData);    

  //##ModelId=3E18748C0045
  itkNewMacro(Self);

  /** Smart Pointer type to a ImageDataItem. */
  //##ModelId=3E0B7B3C00F0
  typedef ImageDataItem::Pointer ImageDataItemPointer;

  //##ModelId=3E0B7B3C0172
  //##Documentation
  //## @brief Type for STL Array of SmartPointers to ImageDataItems */
  typedef std::vector<ImageDataItemPointer> ImageDataItemPointerArray;

  typedef itk::Statistics::Histogram<double> HistogramType;
public:

  //##ModelId=3DCBC2B50345
  //##Documentation
  //## @brief Returns the PixelType of channel @a n.
  const mitk::PixelType& GetPixelType(int n = 0) const;

  //##ModelId=3DCBC5AA0112
  //##Documentation
  //## @brief Get dimension of the image
  //##
  unsigned int GetDimension() const;

  //##ModelId=3DCBC6040068
  //##Documentation
  //## @brief Get the size of dimension @a i (e.g., i=0 results in the number of pixels in x-direction).
  //##
  //## @sa GetDimensions()
  unsigned int GetDimension(int i) const;

  //##ModelId=3E0B494802D6
  //## @brief Get the data vector of the complete image, i.e., of all channels linked together.
  //##
  //## If you only want to access a slice, volume at a specific time or single channel
  //## use one of the SubImageSelector classes.
  virtual void* GetData();

  //##ModelId=3DCBEF2902C6
  //##Documentation
  //## @brief Get a volume at a specific time @a t of channel @a n as a vtkImageData.
  virtual vtkImageData* GetVtkImageData(int t = 0, int n = 0);

  //##ModelId=3DCBE2B802E4
  //##Documentation
  //## @brief Get the complete image, i.e., all channels linked together, as a @a ipPicDescriptor.
  //##
  //## If you only want to access a slice, volume at a specific time or single channel
  //## use one of the SubImageSelector classes.
  virtual ipPicDescriptor* GetPic();

  //##ModelId=3E1012990305
  //##Documentation
  //## @brief Check whether slice @a s at time @a t in channel @a n is set
  virtual bool IsSliceSet(int s = 0, int t = 0, int n = 0) const;

  //##ModelId=3E10139001BF
  //##Documentation
  //## @brief Check whether volume at time @a t in channel @a n is set
  virtual bool IsVolumeSet(int t = 0, int n = 0) const;

  //##ModelId=3E1550E700E2
  //##Documentation
  //## @brief Check whether the channel @a n is set
  virtual bool IsChannelSet(int n = 0) const;

  //##ModelId=3E10148003D7
  //##Documentation
  //## @brief Set @a data as slice @a s at time @a t in channel @a n. It is in
  //## the responsibility of the caller to ensure that the data vector @a data 
  //## is really a slice (at least is not smaller than a slice), since there is 
  //## no chance to check this.
  //## 
  //## @sa SetPicSlice
  virtual bool SetSlice(const void *data, int s = 0, int t = 0, int n = 0);

  //##ModelId=3E1014A00211
  //##Documentation
  //## @brief Set @a data as volume at time @a t in channel @a n. It is in
  //## the responsibility of the caller to ensure that the data vector @a data 
  //## is really a volume (at least is not smaller than a volume), since there is 
  //## no chance to check this.
  //## 
  //## @sa SetPicVolume
  virtual bool SetVolume(const void *data, int t = 0, int n = 0);

  //##Documentation
  //## @brief Set @a data in channel @a n. It is in
  //## the responsibility of the caller to ensure that the data vector @a data 
  //## is really a channel (at least is not smaller than a channel), since there is 
  //## no chance to check this.
  //## 
  //## @sa SetPicChannel
  virtual bool SetChannel(const void *data, int n = 0);

  //##ModelId=3E1027F8023D
  //##Documentation
  //## @brief Set @a pic as slice @a s at time @a t in channel @a n. 
  //## @todo The corresponding @a Geomety3D and depending @a Geometry2D entries 
  //## are updated according to the information provided in the tags of @a pic.
  //## @return @a false : dimensions and/or data-type of @a pic does not
  //## comply with image 
  //## @a true success
  virtual bool SetPicSlice(const ipPicDescriptor *pic, int s = 0, int t = 0, int n = 0);


  //##ModelId=3E102818024D
  //##Documentation
  //## @brief Set @a pic as volume at time @a t in channel @a n.
  //## @todo The corresponding @a Geomety3D and depending @a Geometry2D entries 
  //## are updated according to the information provided in the tags of @a pic.
  //## @return @a false : dimensions and/or data-type of @a pic does not
  //## comply with image 
  //## @a true success
  virtual bool SetPicVolume(const ipPicDescriptor *pic, int t = 0, int n = 0);

  //##Documentation
  //## @brief Set @a pic in channel @a n. 
  //## @todo The corresponding @a Geomety3D and depending @a Geometry2D entries 
  //## are updated according to the information provided in the tags of @a pic.
  //## @return @a false : dimensions and/or data-type of @a pic does not
  //## comply with image 
  //## @a true success
  virtual bool SetPicChannel(const ipPicDescriptor *pic, int n = 0);

  //##ModelId=3E102AE9004B
  //##Documentation
  //## initialize new (or re-initialize) image information
  //## @warning Initialize() by pic assumes a plane, evenly spaced geometry starting at (0,0,0).
  virtual void Initialize(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions, unsigned int channels = 1);

  //##Documentation
  //## initialize new (or re-initialize) image information
  //##
  //## @param shiftBoundingBoxMinimumToZero The bounding-box (in units) 
  //## of @a geometry does not necessarily has its minimum of (0,0,0). 
  //## If @a shiftBoundingBoxMinimumToZero is @a true it is shifted there
  //## without changing the world coordinate in mm of the minimum position
  //## (the translation in mm is changed accordingly).
  virtual void Initialize(const mitk::PixelType& type, const mitk::Geometry3D& geometry, bool shiftBoundingBoxMinimumToZero = true);

  //##Documentation
  //## initialize new (or re-initialize) image information by another
  //## mitk-image.
  //## Only the header is used, not the data vector!
  //##
  virtual void Initialize(const mitk::Image* image);

  //##ModelId=3E102D2601DF
  //##Documentation
  //## initialize new (or re-initialize) image information by @a pic. 
  //## Dimensions and @a Geometry3D /@a Geometry2D are set according 
  //## to the tags in @a pic.
  //## Only the header is used, not the data vector! Use SetPicVolume(pic)
  //## to set the data vector.
  //##
  //## @param tDim override time dimension (@a n[3]) in @a pic (if >0 and <)
  //## @param sDim override z-space dimension (@a n[2]) in @a pic (if >0 and <)
  //## @warning Initialize() by pic assumes a plane, evenly spaced geometry starting at (0,0,0).
  virtual void Initialize(const ipPicDescriptor* pic, int channels = 1, int tDim = -1, int sDim = -1);

  //##Documentation
  //## initialize new (or re-initialize) image information by @a vtkimagedata,
  //## a vtk-image. 
  //## Only the header is used, not the data vector! Use 
  //## SetVolume(vtkimage->GetScalarPointer()) to set the data vector.
  //##
  //## @param tDim override time dimension in @a vtkimagedata (if >0 and <)
  //## @param sDim override z-space dimension in @a vtkimagedata (if >0 and <)
  virtual void Initialize(vtkImageData* vtkimagedata, int channels = 1, int tDim = -1, int sDim = -1);

  //##ModelId=3E102D2601DF
  //##Documentation
  //## initialize new (or re-initialize) image information by @a itkimage, 
  //## a templated itk-image. 
  //## Only the header is used, not the data vector! Use 
  //## SetVolume(itkimage->GetBufferPointer()) to set the data vector.
  //##
  //## @param tDim override time dimension in @a itkimage (if >0 and <)
  //## @param sDim override z-space dimension in @a itkimage (if >0 and <)
  template <typename itkImageType> void InitializeByItk(const itkImageType* itkimage, int channels = 1, int tDim = -1, int sDim=-1)
  {
    if(itkimage==NULL) return;

    m_Dimension=itkimage->GetImageDimension();
    unsigned int i, *tmpDimensions=new unsigned int[m_Dimension>4?m_Dimension:4];
    for(i=0;i<m_Dimension;++i) tmpDimensions[i]=itkimage->GetLargestPossibleRegion().GetSize().GetSize()[i];
    if(m_Dimension<4)
    {
      unsigned int *p;
      for(i=0,p=tmpDimensions+m_Dimension;i<4-m_Dimension;++i, ++p)
        *p=1;
    }

    if((m_Dimension>2) && (sDim>=0))
      tmpDimensions[2]=sDim;
    if((m_Dimension>3) && (tDim>=0))
      tmpDimensions[3]=tDim;

    Initialize(mitk::PixelType(typeid(typename itkImageType::PixelType)), 
      m_Dimension, 
      tmpDimensions,
      channels);
#if (ITK_VERSION_MAJOR == 1 && ITK_VERSION_MINOR > 5) || ITK_VERSION_MAJOR > 1
    const typename itkImageType::SpacingType & itkspacing = itkimage->GetSpacing();  
#else
    const double *itkspacing = itkimage->GetSpacing();  
#endif 
    Vector3D spacing;
    FillVector3D(spacing, itkspacing[0], 1.0, 1.0);
    if(m_Dimension>=2)
      spacing[1]=itkspacing[1];
    if(m_Dimension>=3)
      spacing[2]=itkspacing[2];

    Point3D origin;
    const typename itkImageType::PointType & itkorigin = itkimage->GetOrigin();  
    FillVector3D(origin, itkorigin[0], 0.0, 0.0);
    if(m_Dimension>=2)
      origin[1]=itkorigin[1];
    if(m_Dimension>=3)
      origin[2]=itkorigin[2];

    PlaneGeometry* planeGeometry = static_cast<PlaneGeometry*>(GetSlicedGeometry(0)->GetGeometry2D(0));
    planeGeometry->SetOrigin(origin);
    SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);
    slicedGeometry->InitializeEvenlySpaced(planeGeometry, m_Dimensions[2]);
    slicedGeometry->SetSpacing(spacing);
    m_TimeSlicedGeometry->InitializeEvenlyTimed(slicedGeometry, m_Dimensions[3]);
    delete [] tmpDimensions;

    this->Initialize();
  };

  /**
  * \brief set the images spacing
  */
  void SetSpacing(const float aSpacing[3]);

  void SetSpacing(mitk::Vector3D aSpacing);

  //##ModelId=3E155CF000F6
  //##Documentation
  //## @brief Check whether slice @a s at time @a t in channel @a n is valid, i.e., 
  //## is (or can be) inside of the image
  virtual bool IsValidSlice(int s = 0, int t = 0, int n = 0) const;

  //##ModelId=3E155D2501A7
  //##Documentation
  //## @brief Check whether volume at time @a t in channel @a n is valid, i.e., 
  //## is (or can be) inside of the image
  virtual bool IsValidVolume(int t = 0, int n = 0) const;

  //##ModelId=3E157C53030B
  //##Documentation
  //## @brief Check whether the channel @a n is valid, i.e., 
  //## is (or can be) inside of the image
  virtual bool IsValidChannel(int n = 0) const;

  //##ModelId=3E19EA110396
  //##Documentation
  //## @brief Check whether the image has been initialize, i.e., at least the header has to be set
  virtual bool IsInitialized() const;
  //##ModelId=3E1A11530384
  //##Documentation
  //## @brief Get the sizes of all dimensions as an integer-array.
  //##
  //## @sa GetDimension(int i);
  unsigned int* GetDimensions() const;

  //##Documentation
  //## @brief reset image to non-initialized state, release memory
  virtual void Clear();

  virtual void SetGeometry(Geometry3D* aGeometry3D);

  virtual const HistogramType& GetScalarHistogram() const;

  virtual const void ComputeExtrema() const;

  //##Documentation
  //## \brief Get the minimum for scalar images
  virtual ScalarType GetScalarValueMin() const;

  //##Documentation
  //## \brief Get the maximum for scalar images
  virtual ScalarType GetScalarValueMax() const;

  //##Documentation
  //## \brief Get the second smallest value for scalar images
  virtual ScalarType GetScalarValue2ndMin() const;

  //##Documentation
  //## \brief Get the second largest value for scalar images
  virtual ScalarType GetScalarValue2ndMax() const;

  //##ModelId=3E0B4A6A01EC
  //##Documentation
  //## @warning for internal use only
  virtual mitk::ImageDataItem::Pointer GetSliceData(int s = 0, int t = 0, int n = 0);
  //##ModelId=3E0B4A82001A
  //##Documentation
  //## @warning for internal use only
  virtual mitk::ImageDataItem::Pointer GetVolumeData(int t = 0, int n = 0);
  //##ModelId=3E0B4A9100BC
  //##Documentation
  //## @warning for internal use only
  virtual mitk::ImageDataItem::Pointer GetChannelData(int n = 0);

  template < typename ItkImageType >
    friend void _ComputeExtremaInItkImage(ItkImageType* itkImage, mitk::Image * mitkImage);

protected:
  //##ModelId=3E155C940248
  int GetSliceIndex(int s = 0, int t = 0, int n = 0) const;

  //##ModelId=3E155C76012D
  int GetVolumeIndex(int t = 0, int n = 0) const;

  //##ModelId=3E155E7A0374
  void ComputeOffsetTable();

  //##ModelId=3E1569310328
  virtual mitk::ImageDataItem::Pointer AllocateSliceData(int s = 0, int t = 0, int n = 0);

  //##ModelId=3E15694500EC
  virtual mitk::ImageDataItem::Pointer AllocateVolumeData(int t = 0, int n = 0);

  //##ModelId=3E1569500322
  virtual mitk::ImageDataItem::Pointer AllocateChannelData(int n = 0);

  //##ModelId=3E15F6C60103
  Image();

  //##ModelId=3E15F6CA014F
  ~Image();

  //##Documentation
  //## @warning Has to be called by every Initialize method!
  virtual void Initialize();

  //##ModelId=3E0B7B3C0245
  mutable ImageDataItemPointerArray m_Channels;
  //##ModelId=3E0B7B3C0263
  mutable ImageDataItemPointerArray m_Volumes;
  //##ModelId=3E0B4C1C02C3
  mutable ImageDataItemPointerArray m_Slices;

  //##ModelId=3E156293019B
  unsigned int m_Dimension;
  //##ModelId=3E0B890702C4
  unsigned int *m_Dimensions;
  //##ModelId=3E0B8907036E
  unsigned int *m_OffsetTable;
  //##ModelId=3E10151F001F
  ImageDataItem::Pointer m_CompleteData;
  //##ModelId=3E13FE96005F
  PixelType m_PixelType;

  //##ModelId=3E19EA110292
  bool m_Initialized;

  mutable itk::Object::Pointer m_HistogramGeneratorObject;

  mutable ScalarType m_ScalarMin;
  mutable ScalarType m_ScalarMax;
  mutable ScalarType m_Scalar2ndMin;
  mutable ScalarType m_Scalar2ndMax;

};

template <typename ItkOutputImageType> 
void CastToMitkImage(const itk::SmartPointer<ItkOutputImageType>& itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage)
{
  mitkoutputimage->InitializeByItk(itkimage.GetPointer());
  mitkoutputimage->SetVolume(itkimage->GetBufferPointer());
}

template <typename ItkOutputImageType> 
void CastToMitkImage(const ItkOutputImageType* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage)
{
  mitkoutputimage->InitializeByItk(itkimage);
  mitkoutputimage->SetVolume(itkimage->GetBufferPointer());
}
} // namespace mitk

#endif /* MITKIMAGE_H_HEADER_INCLUDED_C1C2FCD2 */
