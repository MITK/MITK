/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "mitkPixelType.h"
#include "mitkBaseData.h"
#include "mitkLevelWindow.h"
#include "mitkPlaneGeometry.h"

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif 

class vtkImageData;

namespace mitk {

class SubImageSelector;
class ImageDataItem;

//##Documentation
//## @brief Image class for storing images
//##
//## Can be asked for header information, the data vector, 
//## the ipPicDescriptor struct or vtkImageData objects. If not the complete
//## data is required, the appropriate SubImageSelector class should be used
//## for access.
//## Image organizes sets of slices (s x 2D), volumes (t x 3D) and channels (n
//## x ND). Channels are for different kind of data, e.g., morphology in 
//## channel 0, velocities in channel 1. All channels must have the same Geometry! In 
//## particular, the dimensions of all channels are the same, only the pixel-type
//## may differ between channels
//##
//## For importing ITK images use of mitk::ITKImageImport is recommended.
//## @ingroup Data
class MITK_CORE_EXPORT Image : public SlicedData
{
  friend class SubImageSelector;

public:
  mitkClassMacro(Image, SlicedData);    

  itkNewMacro(Self);

  /** Smart Pointer type to a ImageDataItem. */
  typedef itk::SmartPointerForwardReference<ImageDataItem> ImageDataItemPointer;
  enum ImportMemoryManagementType { CopyMemory, ManageMemory, ReferenceMemory, DontManageMemory = ReferenceMemory };

  //##Documentation
  //## @brief Type for STL Array of SmartPointers to ImageDataItems */
  typedef std::vector<ImageDataItemPointer> ImageDataItemPointerArray;

  typedef itk::Statistics::Histogram<double> HistogramType;

public:
  //##Documentation
  //## @brief Returns the PixelType of channel @a n.
  const mitk::PixelType& GetPixelType(int n = 0) const;

  //##Documentation
  //## @brief Get dimension of the image
  //##
  unsigned int GetDimension() const;

  //##Documentation
  //## @brief Get the size of dimension @a i (e.g., i=0 results in the number of pixels in x-direction).
  //##
  //## @sa GetDimensions()
  unsigned int GetDimension(int i) const;

  //## @brief Get the data vector of the complete image, i.e., of all channels linked together.
  //##
  //## If you only want to access a slice, volume at a specific time or single channel
  //## use one of the SubImageSelector classes.
  virtual void* GetData();

  //##Documentation
  //## @brief Get a volume at a specific time @a t of channel @a n as a vtkImageData.
  virtual vtkImageData* GetVtkImageData(int t = 0, int n = 0);

  //##Documentation
  //## @brief Get the complete image, i.e., all channels linked together, as a @a ipPicDescriptor.
  //##
  //## If you only want to access a slice, volume at a specific time or single channel
  //## use one of the SubImageSelector classes.
  virtual ipPicDescriptor* GetPic();

  //##Documentation
  //## @brief Check whether slice @a s at time @a t in channel @a n is set
  virtual bool IsSliceSet(int s = 0, int t = 0, int n = 0) const;

  //##Documentation
  //## @brief Check whether volume at time @a t in channel @a n is set
  virtual bool IsVolumeSet(int t = 0, int n = 0) const;

  //##Documentation
  //## @brief Check whether the channel @a n is set
  virtual bool IsChannelSet(int n = 0) const;

  //##Documentation
  //## @brief Set @a data as slice @a s at time @a t in channel @a n. It is in
  //## the responsibility of the caller to ensure that the data vector @a data 
  //## is really a slice (at least is not smaller than a slice), since there is 
  //## no chance to check this.
  //## 
  //## The data is copied to an array managed by the image. If the image shall
  //## reference the data, use SetImportSlice with ImportMemoryManagementType
  //## set to ReferenceMemory. For importing ITK images use of mitk::
  //## ITKImageImport is recommended.
  //## @sa SetPicSlice, SetImportSlice, SetImportVolume
  virtual bool SetSlice(const void *data, int s = 0, int t = 0, int n = 0);

  //##Documentation
  //## @brief Set @a data as volume at time @a t in channel @a n. It is in
  //## the responsibility of the caller to ensure that the data vector @a data 
  //## is really a volume (at least is not smaller than a volume), since there is 
  //## no chance to check this.
  //## 
  //## The data is copied to an array managed by the image. If the image shall
  //## reference the data, use SetImportVolume with ImportMemoryManagementType
  //## set to ReferenceMemory. For importing ITK images use of mitk::
  //## ITKImageImport is recommended.
  //## @sa SetPicVolume, SetImportVolume
  virtual bool SetVolume(const void *data, int t = 0, int n = 0);

  //##Documentation
  //## @brief Set @a data in channel @a n. It is in
  //## the responsibility of the caller to ensure that the data vector @a data 
  //## is really a channel (at least is not smaller than a channel), since there is 
  //## no chance to check this.
  //## 
  //## The data is copied to an array managed by the image. If the image shall
  //## reference the data, use SetImportChannel with ImportMemoryManagementType
  //## set to ReferenceMemory. For importing ITK images use of mitk::
  //## ITKImageImport is recommended.
  //## @sa SetPicChannel, SetImportChannel
  virtual bool SetChannel(const void *data, int n = 0);

  //##Documentation
  //## @brief Set @a data as slice @a s at time @a t in channel @a n. It is in
  //## the responsibility of the caller to ensure that the data vector @a data 
  //## is really a slice (at least is not smaller than a slice), since there is 
  //## no chance to check this.
  //## 
  //## The data is managed according to the parameter \a importMemoryManagement.
  //## @sa SetPicSlice
  virtual bool SetImportSlice(void *data, int s = 0, int t = 0, int n = 0, ImportMemoryManagementType importMemoryManagement = CopyMemory );

  //##Documentation
  //## @brief Set @a data as volume at time @a t in channel @a n. It is in
  //## the responsibility of the caller to ensure that the data vector @a data 
  //## is really a volume (at least is not smaller than a volume), since there is 
  //## no chance to check this.
  //## 
  //## The data is managed according to the parameter \a importMemoryManagement.
  //## @sa SetPicVolume
  virtual bool SetImportVolume(void *data, int t = 0, int n = 0, ImportMemoryManagementType importMemoryManagement = CopyMemory );

  //##Documentation
  //## @brief Set @a data in channel @a n. It is in
  //## the responsibility of the caller to ensure that the data vector @a data 
  //## is really a channel (at least is not smaller than a channel), since there is 
  //## no chance to check this.
  //## 
  //## The data is managed according to the parameter \a importMemoryManagement.
  //## @sa SetPicChannel
  virtual bool SetImportChannel(void *data, int n = 0, ImportMemoryManagementType importMemoryManagement = CopyMemory );

  //##Documentation
  //## @brief Set @a pic as slice @a s at time @a t in channel @a n. 
  //##
  //## The data is copied to an array managed by the image. 
  //## @todo The corresponding @a Geomety3D and depending @a Geometry2D entries 
  //## are updated according to the information provided in the tags of @a pic.
  //## @return @a false : dimensions and/or data-type of @a pic does not
  //## comply with image 
  //## @a true success
  virtual bool SetPicSlice(const ipPicDescriptor *pic, int s = 0, int t = 0, int n = 0, ImportMemoryManagementType importMemoryManagement = CopyMemory );

  //##Documentation
  //## @brief Set @a pic as volume at time @a t in channel @a n.
  //##
  //## The data is copied to an array managed by the image. 
  //## @todo The corresponding @a Geomety3D and depending @a Geometry2D entries 
  //## are updated according to the information provided in the tags of @a pic.
  //## @return @a false : dimensions and/or data-type of @a pic does not
  //## comply with image 
  //## @a true success
  virtual bool SetPicVolume(const ipPicDescriptor *pic, int t = 0, int n = 0, ImportMemoryManagementType importMemoryManagement = CopyMemory );

  //##Documentation
  //## @brief Set @a pic in channel @a n. 
  //##
  //## The data is copied to an array managed by the image. 
  //## @todo The corresponding @a Geomety3D and depending @a Geometry2D entries 
  //## are updated according to the information provided in the tags of @a pic.
  //## @return @a false : dimensions and/or data-type of @a pic does not
  //## comply with image 
  //## @a true success
  virtual bool SetPicChannel(const ipPicDescriptor *pic, int n = 0, ImportMemoryManagementType importMemoryManagement = CopyMemory );

  //##Documentation
  //## initialize new (or re-initialize) image information
  //## @warning Initialize() by pic assumes a plane, evenly spaced geometry starting at (0,0,0).
  virtual void Initialize(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions, unsigned int channels = 1);

  //##Documentation
  //## initialize new (or re-initialize) image information by a Geometry3D
  //##
  //## @param shiftBoundingBoxMinimumToZero The bounding-box (in units) 
  //## of @a geometry does not necessarily has its minimum of (0,0,0). 
  //## If @a shiftBoundingBoxMinimumToZero is @a true it is shifted there
  //## without changing the world coordinate in mm of the minimum position
  //## (the translation in mm is changed accordingly).
  //## @param tDim override time dimension (@a n[3]) if @a geometry is a TimeSlicedGeometry (if >0)
  virtual void Initialize(const mitk::PixelType& type, const mitk::Geometry3D& geometry, unsigned int channels = 1, int tDim=-1, bool shiftBoundingBoxMinimumToZero = true);

  //##Documentation
  //## initialize new (or re-initialize) image information by a Geometry2D and number of slices
  //##
  //## Initializes the bounding box according to the width/height of the 
  //## Geometry2D and @a sDim via SlicedGeometry3D::InitializeEvenlySpaced.
  //## The spacing is calculated from the Geometry2D.
  //## @param shiftBoundingBoxMinimumToZero The bounding-box (in units) 
  //## of @a geometry2d does not necessarily has its minimum of (0,0,0). 
  //## If @a shiftBoundingBoxMinimumToZero is @a true it is shifted there
  //## without changing the world coordinate in mm of the minimum position
  //## (the translation in mm is changed accordingly).
  //## @param tDim override time dimension (@a n[3]) if @a geometry is a TimeSlicedGeometry (if >0)
  //## \sa SlicedGeometry3D::InitializeEvenlySpaced
  virtual void Initialize(const mitk::PixelType& type, int sDim, const mitk::Geometry2D& geometry2d, bool flipped = false, unsigned int channels = 1, int tDim=-1, bool shiftBoundingBoxMinimumToZero = true);

  //##Documentation
  //## initialize new (or re-initialize) image information by another
  //## mitk-image.
  //## Only the header is used, not the data vector!
  //##
  virtual void Initialize(const mitk::Image* image);

  //##Documentation
  //## initialize new (or re-initialize) image information by @a pic. 
  //## Dimensions and @a Geometry3D /@a Geometry2D are set according 
  //## to the tags in @a pic.
  //## Only the header is used, not the data vector! Use SetPicVolume(pic)
  //## to set the data vector.
  //##
  //## @param tDim override time dimension (@a n[3]) in @a pic (if >0)
  //## @param sDim override z-space dimension (@a n[2]) in @a pic (if >0)
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

    // build array with dimensions in each direction with at least 4 entries
    m_Dimension=itkimage->GetImageDimension();
    unsigned int i, *tmpDimensions=new unsigned int[m_Dimension>4?m_Dimension:4];
    for(i=0;i<m_Dimension;++i) 
      tmpDimensions[i]=itkimage->GetLargestPossibleRegion().GetSize().GetSize()[i];
    if(m_Dimension<4)
    {
      unsigned int *p;
      for(i=0,p=tmpDimensions+m_Dimension;i<4-m_Dimension;++i, ++p)
        *p=1;
    }

    // overwrite number of slices if sDim is set
    if((m_Dimension>2) && (sDim>=0))
      tmpDimensions[2]=sDim;
    // overwrite number of time points if tDim is set
    if((m_Dimension>3) && (tDim>=0))
      tmpDimensions[3]=tDim;

    // rough initialization of Image
    Initialize(mitk::PixelType(typeid(typename itkImageType::PixelType)), 
      m_Dimension, 
      tmpDimensions,
      channels);
#if (ITK_VERSION_MAJOR == 1 && ITK_VERSION_MINOR > 5) || ITK_VERSION_MAJOR > 1
    const typename itkImageType::SpacingType & itkspacing = itkimage->GetSpacing();  
#else
    const double *itkspacing = itkimage->GetSpacing();  
#endif 

    // access spacing of itk::Image
    Vector3D spacing;
    FillVector3D(spacing, itkspacing[0], 1.0, 1.0);
    if(m_Dimension >= 2)
      spacing[1]=itkspacing[1];
    if(m_Dimension >= 3)
      spacing[2]=itkspacing[2];

    // access origin of itk::Image
    Point3D origin;
    const typename itkImageType::PointType & itkorigin = itkimage->GetOrigin();  
    FillVector3D(origin, itkorigin[0], 0.0, 0.0);
    if(m_Dimension>=2)
      origin[1]=itkorigin[1];
    if(m_Dimension>=3)
      origin[2]=itkorigin[2];

    // access direction of itk::Image and include spacing
    const typename itkImageType::DirectionType & itkdirection = itkimage->GetDirection();  
    mitk::Matrix3D matrix;
    matrix.SetIdentity();
    unsigned int j, itkDimMax3 = (m_Dimension >= 3? 3 : m_Dimension);
    // check if spacing has no zero entry and itkdirection has no zero columns
    bool itkdirectionOk = true;
    mitk::ScalarType columnSum;
    for( j=0; j < itkDimMax3; ++j )
    {
      columnSum = 0.0;
      for ( i=0; i < itkDimMax3; ++i)
      {
        columnSum += fabs(itkdirection[i][j]);
      }
      if(columnSum < mitk::eps)
      {
        itkdirectionOk = false;
      }
      if(spacing[j] < mitk::eps)
      {
        itkWarningMacro(<< "Illegal value of itk::Image::GetSpacing()[" << j <<"]=" << spacing[j] << ". Using 1.0 instead.");
        spacing[j] = 1.0;
      }
    }
    if(itkdirectionOk == false)
    {
      itkWarningMacro(<< "Illegal matrix returned by itk::Image::GetDirection():" << itkdirection << " Using identity instead.");
      for ( i=0; i < itkDimMax3; ++i)
        for( j=0; j < itkDimMax3; ++j )
          matrix[i][j] = spacing[j];
    }
    else
    {
      for ( i=0; i < itkDimMax3; ++i)
        for( j=0; j < itkDimMax3; ++j )
          matrix[i][j] = itkdirection[i][j]*spacing[j];
    }

    // re-initialize PlaneGeometry with origin and direction
    PlaneGeometry* planeGeometry = static_cast<PlaneGeometry*>(GetSlicedGeometry(0)->GetGeometry2D(0));
    planeGeometry->SetOrigin(origin);
    planeGeometry->GetIndexToWorldTransform()->SetMatrix(matrix);

    // re-initialize SlicedGeometry3D
    SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);
    slicedGeometry->InitializeEvenlySpaced(planeGeometry, m_Dimensions[2]);
    slicedGeometry->SetSpacing(spacing);

    // re-initialize TimeSlicedGeometry
    GetTimeSlicedGeometry()->InitializeEvenlyTimed(slicedGeometry, m_Dimensions[3]);
    
    // clean-up
    delete [] tmpDimensions;

    this->Initialize();
  };

  //##Documentation
  //## @brief Check whether slice @a s at time @a t in channel @a n is valid, i.e., 
  //## is (or can be) inside of the image
  virtual bool IsValidSlice(int s = 0, int t = 0, int n = 0) const;

  //##Documentation
  //## @brief Check whether volume at time @a t in channel @a n is valid, i.e., 
  //## is (or can be) inside of the image
  virtual bool IsValidVolume(int t = 0, int n = 0) const;

  //##Documentation
  //## @brief Check whether the channel @a n is valid, i.e., 
  //## is (or can be) inside of the image
  virtual bool IsValidChannel(int n = 0) const;

  //##Documentation
  //## @brief Get the sizes of all dimensions as an integer-array.
  //##
  //## @sa GetDimension(int i);
  unsigned int* GetDimensions() const;

  virtual void Clear();

  virtual void SetGeometry(Geometry3D* aGeometry3D);

  virtual const HistogramType* GetScalarHistogram(int t=0) const;

  //##Documentation
  //## \brief Get the minimum for scalar images
  virtual ScalarType GetScalarValueMin(int t=0) const;

  //##Documentation
  //## \brief Get the maximum for scalar images
  virtual ScalarType GetScalarValueMax(int t=0) const;

  //##Documentation
  //## \brief Get the second smallest value for scalar images
  virtual ScalarType GetScalarValue2ndMin(int t=0) const;

  //##Documentation
  //## \brief Get the smallest value for scalar images, but do not recompute it first
  virtual mitk::ScalarType GetScalarValueMinNoRecompute() const
  {
      return m_ScalarMin;
  }

  //##Documentation
  //## \brief Get the second smallest value for scalar images, but do not recompute it first
  virtual mitk::ScalarType GetScalarValue2ndMinNoRecompute() const
  {
      return m_Scalar2ndMin;
  }

  //##Documentation
  //## \brief Get the second largest value for scalar images
  virtual ScalarType GetScalarValue2ndMax(int t=0) const;

  //##Documentation
  //## \brief Get the largest value for scalar images, but do not recompute it first
  virtual mitk::ScalarType GetScalarValueMaxNoRecompute() const
  {
      return m_ScalarMax;
  }

  //##Documentation
  //## \brief Get the second largest value for scalar images, but do not recompute it first
  virtual mitk::ScalarType GetScalarValue2ndMaxNoRecompute() const
  {
      return m_Scalar2ndMax;
  }

  //##Documentation
  //## \brief Get the count of voxels with the largest scalar value in the dataset
  virtual unsigned int GetCountOfMaxValuedVoxelsNoRecompute() const
  {
      return m_CountOfMaxValuedVoxels;
  }

  //##Documentation
  //## \brief Get the count of voxels with the largest scalar value in the dataset
  virtual unsigned int GetCountOfMinValuedVoxelsNoRecompute() const
  {
      return m_CountOfMinValuedVoxels;
  }

  //##Documentation
  //## @warning for internal use only
  virtual ImageDataItemPointer GetSliceData(int s = 0, int t = 0, int n = 0, void *data = NULL, ImportMemoryManagementType importMemoryManagement = CopyMemory);

  //##Documentation
  //## @warning for internal use only
  virtual ImageDataItemPointer GetVolumeData(int t = 0, int n = 0, void *data = NULL, ImportMemoryManagementType importMemoryManagement = CopyMemory);

  //##Documentation
  //## @warning for internal use only
  virtual ImageDataItemPointer GetChannelData(int n = 0, void *data = NULL, ImportMemoryManagementType importMemoryManagement = CopyMemory);

  //##
  virtual bool WriteXMLData( XMLWriter& xmlWriter );
  //##
  virtual bool ReadXMLData( XMLReader& xmlReader );

  template < typename ItkImageType >
    friend void _ComputeExtremaInItkImage(ItkImageType* itkImage, mitk::Image * mitkImage);

protected:
  int GetSliceIndex(int s = 0, int t = 0, int n = 0) const;

  int GetVolumeIndex(int t = 0, int n = 0) const;

  void ComputeOffsetTable();

  virtual const void ComputeExtrema(int t=0) const;

  virtual ImageDataItemPointer AllocateSliceData(int s = 0, int t = 0, int n = 0, void *data = NULL, ImportMemoryManagementType importMemoryManagement = CopyMemory);

  virtual ImageDataItemPointer AllocateVolumeData(int t = 0, int n = 0, void *data = NULL, ImportMemoryManagementType importMemoryManagement = CopyMemory);

  virtual ImageDataItemPointer AllocateChannelData(int n = 0, void *data = NULL, ImportMemoryManagementType importMemoryManagement = CopyMemory);

  Image();

  virtual ~Image();

  //## @warning Has to be called by every Initialize method!
  virtual void Initialize();

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  mutable ImageDataItemPointerArray m_Channels;
  mutable ImageDataItemPointerArray m_Volumes;
  mutable ImageDataItemPointerArray m_Slices;

  unsigned int m_Dimension;
  unsigned int *m_Dimensions;
  size_t *m_OffsetTable;
  ImageDataItemPointer m_CompleteData;
  PixelType m_PixelType;

  mutable itk::Object::Pointer m_HistogramGeneratorObject;

  mutable itk::Object::Pointer m_TimeSelectorForExtremaObject;
  mutable unsigned int m_CountOfMinValuedVoxels;
  mutable unsigned int m_CountOfMaxValuedVoxels;
  mutable ScalarType m_ScalarMin;
  mutable ScalarType m_ScalarMax;
  mutable ScalarType m_Scalar2ndMin;
  mutable ScalarType m_Scalar2ndMax;

  itk::TimeStamp m_LastRecomputeTimeStamp;

};

//##Documentation
//## @brief Cast an itk::Image (with a specific type) to an mitk::Image.
//##
//## CastToMitkImage does not cast pixel types etc., just image data
//## Needs "mitkImage.h" header included.
//## If you get a compile error, try image.GetPointer();
//## @ingroup Adaptor
//## \sa mitkITKImageImport
template <typename ItkOutputImageType> 
void CastToMitkImage(const itk::SmartPointer<ItkOutputImageType>& itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage)
{
  if(mitkoutputimage.IsNull())
  {
    mitkoutputimage = mitk::Image::New();
  }
  mitkoutputimage->InitializeByItk(itkimage.GetPointer());
  mitkoutputimage->SetChannel(itkimage->GetBufferPointer());
}

//##Documentation
//## @brief Cast an itk::Image (with a specific type) to an mitk::Image.
//##
//## CastToMitkImage does not cast pixel types etc., just image data
//## Needs "mitkImage.h" header included.
//## If you get a compile error, try image.GetPointer();
//## @ingroup Adaptor
//## \sa mitkITKImageImport
template <typename ItkOutputImageType> 
void CastToMitkImage(const ItkOutputImageType* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage)
{
  if(mitkoutputimage.IsNull())
  {
    mitkoutputimage = mitk::Image::New();
  }
  mitkoutputimage->InitializeByItk(itkimage);
  mitkoutputimage->SetChannel(itkimage->GetBufferPointer());
}
} // namespace mitk

#endif /* MITKIMAGE_H_HEADER_INCLUDED_C1C2FCD2 */
