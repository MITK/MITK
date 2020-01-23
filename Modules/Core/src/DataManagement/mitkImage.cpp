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
#include <boost/format.hpp>
#include <boost/algorithm/string_regex.hpp>

#include <iostream>
#include <string>
#include <vector>

//MITK
#include "mitkImage.h"
#include "mitkImageRegionAccessor.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkPixelTypeMultiplex.h"
#include <mitkProportionalTimeGeometry.h>
#include "mitkCompareImageDataFilter.h"
#include "mitkImageVtkAccessor.h"

#include "mitkDicomTagsList.h"
#include "mitkSliceNavigationController.h"

//VTK
#include <vtkImageData.h>

//ITK
#include <itkMutexLockHolder.h>
#include <itkMetaDataObjectBase.h>

//Other
#include <cmath>

#define FILL_C_ARRAY( _arr, _size, _value) for(unsigned int i=0u; i<_size; i++) \
{ _arr[i] = _value; }

mitk::ReaderType::DictionaryArrayType mitk::Image::GetMetaDataDictionaryArray() const
{
  if (m_MetaDataDictionaryArray.size()) {
    return m_MetaDataDictionaryArray;
  }

  int numberSlices = GetLargestPossibleRegion().GetSize()[2];
  int timeSteps = GetTimeSteps();
  int numberOfComponents = GetPixelType().GetNumberOfComponents();

  //mitk::ReaderType::DictionaryArrayType outputArray;
  m_MetaDataDictionaryArray.resize(numberSlices * timeSteps * numberOfComponents);

  for (unsigned int i = 0; i < m_MetaDataDictionaryArray.size(); ++i)
  {
    m_MetaDataDictionaryArray[i] = new mitk::ReaderType::DictionaryType;
  }

  mitk::SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);
  for (unsigned int i = 0; i < dicomTagsList.size(); ++i)
  {
    std::string tagkey = dicomTagsList[i];
    mitk::StringLookupTableProperty* tagsValueList =
      dynamic_cast<mitk::StringLookupTableProperty*>(GetProperty(tagkey.c_str()).GetPointer());

    for (unsigned int j = 0; j < m_MetaDataDictionaryArray.size(); ++j)
    {
      std::string tagvalue = std::string();

      if (tagsValueList != nullptr && tagsValueList->GetValue().ValueExists(j))
      {
        tagvalue = tagsValueList->GetValue().GetTableValue(j);
      }
      else
      {
        // TODO: To skip these tags is necessary in order for the gdcm to generate them correctly.
        if (tagkey == TAG_SOP_CLASS_UID || tagkey == TAG_SOP_INST_UID)
        {
          continue;
        }

        auto iter = tagToPropertyMap.find(tagkey);
        if (iter != tagToPropertyMap.end())
        {
          mitk::BaseProperty* modalityProp = GetProperty(iter->second.c_str());
          tagvalue = (modalityProp) ? modalityProp->GetValueAsString() : "";
        }

        mitk::PlaneGeometry* planegeometry = nullptr;
        if (slicedGeometry != nullptr)
        {
          planegeometry = slicedGeometry->GetPlaneGeometry(j);
        }

        if (planegeometry != nullptr)
        {
          mitk::Vector3D spacingVector = planegeometry->GetSpacing();
          mitk::Point3D originVector = planegeometry->GetOrigin();
          mitk::Matrix3D imageMatrix = planegeometry->GetIndexToWorldTransform()->GetMatrix();

          imageMatrix[0][0] = imageMatrix[0][0] / spacingVector[0];
          imageMatrix[1][1] = imageMatrix[1][1] / spacingVector[1];

          std::string imageOrientation = boost::str(boost::format("%d\\%d\\%d\\%d\\%d\\%d")
          %imageMatrix[0][0] %imageMatrix[0][1] %imageMatrix[0][2]
          %imageMatrix[1][0] %imageMatrix[1][1] %imageMatrix[1][2]
          );

          std::string imagePosition = boost::str(boost::format("%d\\%d\\%d")
          %originVector[0] %originVector[1] %originVector[2]
          );

          std::string spacing = boost::str(boost::format("%d\\%d\\%d")
          %spacingVector[0] %spacingVector[1] %spacingVector[2]
          );

          if (tagkey == TAG_IMAGE_ORIENTATION)
          {
            itk::EncapsulateMetaData<std::string>(*(m_MetaDataDictionaryArray[j]), tagkey, imageOrientation);
            continue;
          }
          else if (tagkey == TAG_IMAGE_POSITION)
          {
            itk::EncapsulateMetaData<std::string>(*(m_MetaDataDictionaryArray[j]), tagkey, imagePosition);
            continue;
          }
          else if (tagkey == TAG_PIXEL_SPACING)
          {
            itk::EncapsulateMetaData<std::string>(*(m_MetaDataDictionaryArray[j]), tagkey, spacing);
            continue;
          }
        }
      }

      if (!tagvalue.empty())
      {
        itk::EncapsulateMetaData<std::string>(*(m_MetaDataDictionaryArray[j]), tagkey, tagvalue);
      }
    }
  }

  return m_MetaDataDictionaryArray;
}

void  mitk::Image::SetMetaDataDictionary(ReaderType::DictionaryArrayType metaData)
{
  int numberSlices = GetLargestPossibleRegion().GetSize()[2];

  // Setup slices
  mitk::SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);
  for (int i = 0; i < metaData.size(); i++) {
    for (auto tagKey : metaData[i]->GetKeys()) {

      SetProperty(tagKey.c_str(),   mitk::StringProperty::New(dynamic_cast<const itk::MetaDataObject<std::string>*>(metaData[i]->Get(tagKey))->GetMetaDataObjectValue()));

      // Skip tags
      if (tagKey == TAG_SOP_CLASS_UID || tagKey == TAG_SOP_INST_UID) {
        continue;
      }

      // Slice specific tags
      if (tagKey == TAG_IMAGE_POSITION || tagKey == TAG_PIXEL_SPACING) {
        mitk::PlaneGeometry* planegeometry = slicedGeometry->GetPlaneGeometry(i);
        if (planegeometry != nullptr) {
          if (tagKey == TAG_IMAGE_POSITION) {
            std::vector<std::string> imagePosition;
            boost::algorithm::split_regex(imagePosition, dynamic_cast<const itk::MetaDataObject<std::string>*>(metaData[i]->Get(tagKey))->GetMetaDataObjectValue(), boost::regex("\\\\"));

            mitk::Point3D originVector;
            originVector[0] = std::stof(imagePosition[0]);
            originVector[1] = std::stof(imagePosition[1]);
            originVector[2] = std::stof(imagePosition[2]);

            planegeometry->SetOrigin(originVector);

            continue;
          }

          std::vector<std::string> spacing;
          boost::algorithm::split_regex(spacing, dynamic_cast<const itk::MetaDataObject<std::string>*>(metaData[i]->Get(tagKey))->GetMetaDataObjectValue(), boost::regex("\\\\"));

          mitk::Vector3D spacingVector;

          spacingVector[0] = std::stof(spacing[0]);
          spacingVector[1] = std::stof(spacing[1]);
          spacingVector[2] = spacing.size() == 3 ? std::stof(spacing[2]) : 1.;

          planegeometry->SetSpacing(spacingVector);

          continue;
        }
      }
    }
  }
}

void mitk::Image::SetMetaDataDictionary(DicomTagToValueList& array)
{
  DicomTagToValueList::iterator iter = array.begin();
  for (; iter != array.end(); ++iter)
  {
    SetProperty(iter->first.c_str(), StringLookupTableProperty::New(iter->second));
  }
}

mitk::Image::Image() :
  m_Dimension(0), m_Dimensions(nullptr), m_ImageDescriptor(nullptr), m_OffsetTable(nullptr),
  m_ImageStatistics(nullptr)
{
  m_Dimensions = new unsigned int[MAX_IMAGE_DIMENSIONS];
  FILL_C_ARRAY( m_Dimensions, MAX_IMAGE_DIMENSIONS, 0u);

  m_Initialized = false;
}

mitk::Image::Image(const Image &other) : SlicedData(other), m_Dimension(0), m_Dimensions(nullptr),
  m_ImageDescriptor(nullptr), m_OffsetTable(nullptr), m_ImageStatistics(nullptr)
{
  m_Dimensions = new unsigned int[MAX_IMAGE_DIMENSIONS];
  FILL_C_ARRAY( m_Dimensions, MAX_IMAGE_DIMENSIONS, 0u);

  this->Initialize( other.GetPixelType(), other.GetDimension(), other.GetDimensions());

  //Since the above called "Initialize" method doesn't take the geometry into account we need to set it
  //here manually
  TimeGeometry::Pointer cloned = other.GetTimeGeometry()->Clone();
  this->SetTimeGeometry(cloned.GetPointer());

  if (this->GetDimension() > 3)
  {
    const unsigned int time_steps = this->GetDimension(3);

    for (unsigned int i = 0u; i < time_steps; ++i)
    {
      ImageDataItemPointer volume = const_cast<Image&>(other).GetVolumeData(i);

      this->SetVolume(volume->GetData(), i);
    }
  }
  else
  {
    ImageDataItemPointer volume = const_cast<Image&>(other).GetVolumeData(0);

    this->SetVolume(volume->GetData(), 0);
  }
}

mitk::Image::~Image()
{
  this->Clear();

  m_ReferenceCount = 3;
  m_ReferenceCount = 0;

  delete [] m_OffsetTable;
  delete m_ImageStatistics;

  for (unsigned int i = 0; i < m_MetaDataDictionaryArray.size(); ++i) {
    delete m_MetaDataDictionaryArray[i];
  }

  m_MetaDataDictionaryArray.clear();
}

const mitk::PixelType mitk::Image::GetPixelType(int n) const
{
  return this->m_ImageDescriptor->GetChannelTypeById(n);
}

unsigned int mitk::Image::GetDimension() const
{
  return m_Dimension;
}

unsigned int mitk::Image::GetDimension(int i) const
{
  if((i>=0) && (i<(int)m_Dimension))
    return m_Dimensions[i];
  return 1;
}

bool mitk::Image::canAddAccessLock(ImageAccessLock* lock)
{
  bool writeAccess = lock->getWriteAccess();
  if (writeAccess) {
    for (const auto& reader : m_ReaderLocks) {
      if (reader->getAccessor()->overlap(*lock->getAccessor())) {
        return false;
      }
    }
  }

  for (const auto& writer : m_WriterLocks) {
    if (writer->getAccessor()->overlap(*lock->getAccessor())) {
      return false;
    }
  }

  return true;
}

void mitk::Image::addAccessLock(ImageAccessLock* lock) {
  if (lock->getWriteAccess()) {
    m_WriterLocks.push_back(lock);
  } else {
    m_ReaderLocks.push_back(lock);
  }
}

void mitk::Image::removeAccessLock(ImageAccessLock* lock) {
  if (lock->getWriteAccess()) {
    m_WriterLocks.erase(std::remove(m_WriterLocks.begin(), m_WriterLocks.end(), lock), m_WriterLocks.end());
  } else {
    m_ReaderLocks.erase(std::remove(m_ReaderLocks.begin(), m_ReaderLocks.end(), lock), m_ReaderLocks.end());
  }

  m_ImageAccessLockRemoved.notify_all();
}

template <class T>
void AccessPixel( const mitk::PixelType ptype, void* data, const unsigned int offset, double& value )
{
  value = 0.0;
  if( data == nullptr ) return;

  if(ptype.GetBpe() != 24)
  {
    value = (double) (((T*) data)[ offset ]);
  }
  else
  {
    const unsigned int rgboffset = offset;

    double returnvalue = (((T*) data)[rgboffset ]);
    returnvalue += (((T*) data)[rgboffset + 1]);
    returnvalue += (((T*) data)[rgboffset + 2]);
    value = returnvalue;
  }

}

double mitk::Image::GetPixelValueByIndex(const itk::Index<3> &position, unsigned int timestep, unsigned int component)
{
  double value = 0;
  if (this->GetTimeSteps() < timestep)
  {
    timestep = this->GetTimeSteps();
  }

  value = 0.0;

  const unsigned int* imageDims = this->m_ImageDescriptor->GetDimensions();
  const mitk::PixelType ptype = this->m_ImageDescriptor->GetChannelTypeById(0);

  // Comparison ?>=0 not needed since all position[i] and timestep are unsigned int
  // (position[0]>=0 && position[1] >=0 && position[2]>=0 && timestep>=0)
  // bug-11978 : we still need to catch index with negative values
  if ( position[0] < 0 ||
       position[1] < 0 ||
       position[2] < 0 )
  {
    MITK_WARN << "Given position ("<< position << ") is out of image range, returning 0." ;
  }
  // check if the given position is inside the index range of the image, the 3rd dimension needs to be compared only if the dimension is not 0
  else if ( (unsigned int)position[0] >= imageDims[0] ||
            (unsigned int)position[1] >= imageDims[1] ||
            ( imageDims[2] && (unsigned int)position[2] >= imageDims[2] ))
  {
    MITK_WARN << "Given position ("<< position << ") is out of image range, returning 0." ;
  }
  else
  {
    const unsigned int offset = component + ptype.GetNumberOfComponents()*(position[0] + position[1]*imageDims[0] + position[2]*imageDims[0]*imageDims[1] + timestep*imageDims[0]*imageDims[1]*imageDims[2]);
    value = mitk::UnlockedSinglePixelAccess(this, position, timestep);
  }

  return value;
}

double mitk::Image::GetPixelValueByWorldCoordinate(const mitk::Point3D& position, unsigned int timestep, unsigned int component)
{
  double value = 0.0;
  if (this->GetTimeSteps() < timestep)
  {
    timestep = this->GetTimeSteps();
  }

  itk::Index<3> itkIndex;
  this->GetGeometry()->WorldToIndex(position, itkIndex);

  value = this->GetPixelValueByIndex( itkIndex, timestep, component);

  return value;
}

mitk::Image::ImageDataItemPointer mitk::Image::GetSliceData(int s, int t, int n) const
{
  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();
  ImageDataItemPointer vol = GetVolumeData(t, n);
  return new ImageDataItem(*vol, m_ImageDescriptor, t, 2, nullptr, false, ((size_t)s) * m_OffsetTable[2]*(ptypeSize));
}

mitk::Image::ImageDataItemPointer mitk::Image::GetVolumeData(int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return GetVolumeData_unlocked(t, n, data, importMemoryManagement);
}
mitk::Image::ImageDataItemPointer mitk::Image::GetVolumeData_unlocked(int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  if(IsValidVolume(t,n)==false) return nullptr;

  ImageDataItemPointer ch, vol;

  // volume directly available?
  int pos=GetVolumeIndex(t,n);
  vol=m_Volumes[pos];
  if((vol.GetPointer()!=nullptr) && (vol->IsComplete()))
    return vol;

  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();

  // volume is unavailable. Can we calculate it?
  if((GetSource().IsNotNull()) && (GetSource()->Updating()==false))
  {
    // ... wir muessen rechnen!!! ....
    m_RequestedRegion.SetIndex(0, 0);
    m_RequestedRegion.SetIndex(1, 0);
    m_RequestedRegion.SetIndex(2, 0);
    m_RequestedRegion.SetIndex(3, t);
    m_RequestedRegion.SetIndex(4, n);
    m_RequestedRegion.SetSize(0, m_Dimensions[0]);
    m_RequestedRegion.SetSize(1, m_Dimensions[1]);
    m_RequestedRegion.SetSize(2, m_Dimensions[2]);
    m_RequestedRegion.SetSize(3, 1);
    m_RequestedRegion.SetSize(4, 1);
    m_RequestedRegionInitialized=true;
    GetSource()->Update();
    if(IsVolumeSet_unlocked(t,n))
      //yes: now we can call ourselves without the risk of a endless loop (see "if" above)
      return GetVolumeData_unlocked(t,n,data,importMemoryManagement);
    else
      return nullptr;
  }
  else
  {
    ImageDataItemPointer item = AllocateVolumeData_unlocked(t,n,data,importMemoryManagement);
    item->SetComplete(true);
    return item;
  }
}

bool mitk::Image::IsVolumeSet(int t, int n) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return IsVolumeSet_unlocked(t, n);
}

bool mitk::Image::IsVolumeSet_unlocked(int t, int n) const
{
  if(IsValidVolume(t,n)==false) return false;
  ImageDataItemPointer ch, vol;

  // volume directly available?
  vol=m_Volumes[GetVolumeIndex(t,n)];
  if((vol.GetPointer()!=nullptr) && (vol->IsComplete()))
    return true;

  return false;
}

bool mitk::Image::SetVolume(const void *data, int t, int n)
{
  // const_cast is no risk for ImportMemoryManagementType == CopyMemory
  return SetImportVolume(const_cast<void*>(data), t, n, CopyMemory);
}

void mitk::Image::SetSlice(const void* data, int s, int t, int n)
{
  ImageDataItem::Pointer volume = GetVolumeData(t, n);
  size_t pixelSize = GetPixelType().GetSize();
  memcpy((char*)volume->GetData() + (size_t)s * m_OffsetTable[2] * pixelSize, data, m_OffsetTable[2] * pixelSize);
}

void mitk::Image::Initialize()
{
  ImageDataItemPointerArray::iterator it, end;
  for( it=m_Volumes.begin(), end=m_Volumes.end(); it!=end; ++it )
  {
    (*it)=nullptr;
  }

  m_Data = nullptr;

  if( m_ImageStatistics == nullptr)
  {
    m_ImageStatistics = new mitk::ImageStatisticsHolder( this );
  }

  SetRequestedRegionToLargestPossibleRegion();
}

void mitk::Image::Initialize(const mitk::ImageDescriptor::Pointer inDesc)
{
  // store the descriptor
  this->m_ImageDescriptor = inDesc;

  // initialize image
  this->Initialize( inDesc->GetChannelDescriptor(0).GetPixelType(), inDesc->GetNumberOfDimensions(), inDesc->GetDimensions(), 1 );
}

void mitk::Image::Initialize(const mitk::PixelType& type, unsigned int dimension, const unsigned int *dimensions, unsigned int channels)
{
  Clear();

  m_Dimension=dimension;

  if(!dimensions)
    itkExceptionMacro(<< "invalid zero dimension image");

  unsigned int i;
  for(i=0;i<dimension;++i)
  {
    if(dimensions[i]<1)
      itkExceptionMacro(<< "invalid dimension[" << i << "]: " << dimensions[i]);
  }

  // create new array since the old was deleted
  m_Dimensions = new unsigned int[MAX_IMAGE_DIMENSIONS];

  // initialize the first four dimensions to 1, the remaining 4 to 0
  FILL_C_ARRAY(m_Dimensions, 4, 1u);
  FILL_C_ARRAY((m_Dimensions+4), 4, 0u);

  // copy in the passed dimension information
  std::memcpy(m_Dimensions, dimensions, sizeof(unsigned int)*m_Dimension);

  this->m_ImageDescriptor = mitk::ImageDescriptor::New();
  this->m_ImageDescriptor->Initialize( this->m_Dimensions, this->m_Dimension );

  for(i=0;i<4;++i)
  {
    m_LargestPossibleRegion.SetIndex(i, 0);
    m_LargestPossibleRegion.SetSize (i, m_Dimensions[i]);
  }
  m_LargestPossibleRegion.SetIndex(i, 0);
  m_LargestPossibleRegion.SetSize(i, channels);

  if(m_LargestPossibleRegion.GetNumberOfPixels()==0)
  {
    delete [] m_Dimensions;
    m_Dimensions = nullptr;
    return;
  }

  for( unsigned int i=0u; i<channels; i++)
  {
    this->m_ImageDescriptor->AddNewChannel( type );
  }

  PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();
  planegeometry->InitializeStandardPlane(m_Dimensions[0], m_Dimensions[1]);

  SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New();
  slicedGeometry->InitializeEvenlySpaced(planegeometry, m_Dimensions[2]);

  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(slicedGeometry, m_Dimensions[3]);
  timeGeometry->componentSize = GetPixelType().GetNumberOfComponents();
  for (TimeStepType step = 0; step < timeGeometry->CountTimeSteps(); ++step)
  {
    timeGeometry->GetGeometryForTimeStep(step)->ImageGeometryOn();
  }
  SetTimeGeometry(timeGeometry);

  ImageDataItemPointer dnull=nullptr;

  m_Volumes.assign(GetNumberOfChannels()*m_Dimensions[3], dnull);

  ComputeOffsetTable();

  Initialize();

  m_Data = new ImageDataItem(GetImageDescriptor(), 0, nullptr, true);

  m_Initialized = true;
}

void mitk::Image::Initialize(const mitk::PixelType& type, unsigned int dimension, const unsigned int* dimensions, unsigned int channels, const Point3D& origin, const Matrix3D& matrix,
  const Vector3D& spacing)
{
  Initialize(type, dimension, dimensions, channels);

  // re-initialize PlaneGeometry with origin and direction
  PlaneGeometry* planeGeometry = static_cast<PlaneGeometry*>(GetSlicedGeometry(0)->GetPlaneGeometry(0));
  planeGeometry->SetOrigin(origin);
  planeGeometry->GetIndexToWorldTransform()->SetMatrix(matrix);

  // re-initialize SlicedGeometry3D
  SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);
  slicedGeometry->InitializeEvenlySpaced(planeGeometry, m_Dimensions[2]);
  slicedGeometry->SetSpacing(spacing);

  // re-initialize TimeGeometry
  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(slicedGeometry, m_Dimensions[3]);
  SetTimeGeometry(timeGeometry);

  this->Initialize();
}

void mitk::Image::Initialize(const mitk::PixelType& type, const mitk::BaseGeometry& geometry, unsigned int channels, int tDim )
{
  mitk::ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(geometry.Clone(), tDim);
  this->Initialize(type, *timeGeometry, channels, tDim);
}

void mitk::Image::Initialize(const mitk::PixelType& type, const mitk::TimeGeometry& geometry, unsigned int channels, int tDim )
{
  unsigned int dimensions[5];
  dimensions[0] = (unsigned int)(geometry.GetGeometryForTimeStep(0)->GetExtent(0)+0.5);
  dimensions[1] = (unsigned int)(geometry.GetGeometryForTimeStep(0)->GetExtent(1)+0.5);
  dimensions[2] = (unsigned int)(geometry.GetGeometryForTimeStep(0)->GetExtent(2)+0.5);
  dimensions[3] = (tDim > 0) ? tDim : geometry.CountTimeSteps();
  dimensions[4] = 0;

  unsigned int dimension = 2;
  if ( dimensions[2] > 1 )
    dimension = 3;
  if ( dimensions[3] > 1 )
    dimension = 4;

  Initialize( type, dimension, dimensions, channels );
  if (geometry.CountTimeSteps() > 1)
  {
    TimeGeometry::Pointer cloned = geometry.Clone();
    SetTimeGeometry(cloned.GetPointer());

    // make sure the image geometry flag is properly set for all time steps
    for (TimeStepType step = 0; step < cloned->CountTimeSteps(); ++step)
    {

      if( ! cloned->GetGeometryCloneForTimeStep(step)->GetImageGeometry() )
      {
        MITK_WARN("Image.3DnT.Initialize") << " Attempt to initialize an image with a non-image geometry. Re-interpretting the initialization geometry for timestep " << step
                                           << " as image geometry, the original geometry remains unchanged.";
        cloned->GetGeometryForTimeStep(step)->ImageGeometryOn();
      }
    }
  }
  else
  {
    // make sure the image geometry coming from outside has proper value of the image geometry flag
    BaseGeometry::Pointer cloned = geometry.GetGeometryCloneForTimeStep(0)->Clone();
    if( ! cloned->GetImageGeometry() )
    {
      MITK_WARN("Image.Initialize") << " Attempt to initialize an image with a non-image geometry. Re-interpretting the initialization geometry as image geometry, the original geometry remains unchanged.";
      cloned->ImageGeometryOn();

    }

    Superclass::SetGeometry( cloned );
  }
/* //Old //TODO_GOETZ Really necessary?
  mitk::BoundingBox::BoundsArrayType bounds = geometry.GetBoundingBoxInWorld()->GetBounds();
  if( (bounds[0] != 0.0) || (bounds[2] != 0.0) || (bounds[4] != 0.0) )
  {
    SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);

    mitk::Point3D origin; origin.Fill(0.0);
    slicedGeometry->IndexToWorld(origin, origin);

    bounds[1]-=bounds[0]; bounds[3]-=bounds[2]; bounds[5]-=bounds[4];
    bounds[0] = 0.0;      bounds[2] = 0.0;      bounds[4] = 0.0;
    this->m_ImageDescriptor->Initialize( this->m_Dimensions, this->m_Dimension );
    slicedGeometry->SetBounds(bounds);
    slicedGeometry->GetIndexToWorldTransform()->SetOffset(origin.GetVnlVector().data_block());

    ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
    timeGeometry->Initialize(slicedGeometry, m_Dimensions[3]);
    SetTimeGeometry(timeGeometry);
  }*/
}

void mitk::Image::Initialize(const mitk::PixelType& type, int sDim, const mitk::PlaneGeometry& geometry2d, bool flipped, unsigned int channels, int tDim )
{
  SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New();
  slicedGeometry->InitializeEvenlySpaced(static_cast<PlaneGeometry*>(geometry2d.Clone().GetPointer()), sDim, flipped);
  Initialize(type, *slicedGeometry, channels, tDim);
}

void mitk::Image::Initialize(const mitk::Image* image)
{
  Initialize(image->GetPixelType(), *image->GetTimeGeometry());
}

void mitk::Image::Initialize(vtkImageData* vtkimagedata, int channels, int tDim, int sDim, int pDim)
{
  mitk::PixelType pixelType(MakePixelType(vtkimagedata));
  Initialize(pixelType, vtkimagedata, channels, tDim, sDim, pDim);
}

void mitk::Image::Initialize(const mitk::PixelType& type, vtkImageData* vtkimagedata, int channels, int tDim, int sDim, int pDim)
{
  if (vtkimagedata == nullptr) return;

  m_Dimension = vtkimagedata->GetDataDimension();
  unsigned int i, *tmpDimensions = new unsigned int[m_Dimension>4 ? m_Dimension : 4];
  for (i = 0; i<m_Dimension; ++i) tmpDimensions[i] = vtkimagedata->GetDimensions()[i];
  if (m_Dimension<4)
  {
    unsigned int *p;
    for (i = 0, p = tmpDimensions + m_Dimension; i<4 - m_Dimension; ++i, ++p)
      *p = 1;
  }

  if (pDim >= 0)
  {
    tmpDimensions[1] = pDim;
    if (m_Dimension < 2)
      m_Dimension = 2;
  }
  if (sDim >= 0)
  {
    tmpDimensions[2] = sDim;
    if (m_Dimension < 3)
      m_Dimension = 3;
  }
  if (tDim >= 0)
  {
    tmpDimensions[3] = tDim;
    if (m_Dimension < 4)
      m_Dimension = 4;
  }

  Initialize(type, m_Dimension, tmpDimensions, channels);

  const double *spacinglist = vtkimagedata->GetSpacing();
  Vector3D spacing;
  FillVector3D(spacing, spacinglist[0], 1.0, 1.0);
  if (m_Dimension >= 2)
    spacing[1] = spacinglist[1];
  if (m_Dimension >= 3)
    spacing[2] = spacinglist[2];

  // access origin of vtkImage
  Point3D origin;
  double vtkorigin[3];
  vtkimagedata->GetOrigin(vtkorigin);
  FillVector3D(origin, vtkorigin[0], 0.0, 0.0);
  if (m_Dimension >= 2)
    origin[1] = vtkorigin[1];
  if (m_Dimension >= 3)
    origin[2] = vtkorigin[2];

  SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);

  // re-initialize PlaneGeometry with origin and direction
  PlaneGeometry* planeGeometry = static_cast<PlaneGeometry*>(slicedGeometry->GetPlaneGeometry(0));
  planeGeometry->SetOrigin(origin);

  // re-initialize SlicedGeometry3D
  slicedGeometry->SetOrigin(origin);
  slicedGeometry->SetSpacing(spacing);

  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(slicedGeometry, m_Dimensions[3]);
  timeGeometry->componentSize = GetPixelType().GetNumberOfComponents();
  SetTimeGeometry(timeGeometry);

  delete[] tmpDimensions;
}

bool mitk::Image::IsValidSlice(int s, int t, int n) const
{
  if(m_Initialized)
    return ((s>=0) && (s<(int)m_Dimensions[2]) && (t>=0) && (t< (int) m_Dimensions[3]) && (n>=0) && (n< (int)GetNumberOfChannels()));
  else
    return false;
}

bool mitk::Image::IsValidVolume(int t, int n) const
{
  if(m_Initialized)
    return IsValidSlice(0, t, n);
  else
    return false;
}

bool mitk::Image::IsValidChannel(int n) const
{
  if(m_Initialized)
    return IsValidSlice(0, 0, n);
  else
    return false;
}

void mitk::Image::ComputeOffsetTable()
{
  if(m_OffsetTable!=nullptr)
    delete [] m_OffsetTable;

  m_OffsetTable=new size_t[m_Dimension>4 ? m_Dimension+1 : 4+1];

  unsigned int i;
  size_t num=1;
  m_OffsetTable[0] = 1;
  for (i=0; i < m_Dimension; ++i)
  {
    num *= m_Dimensions[i];
    m_OffsetTable[i+1] = num;
  }
  for (;i < 4; ++i)
    m_OffsetTable[i+1] = num;
}

bool mitk::Image::IsValidTimeStep(int t) const
{
  return ( ( m_Dimension >= 4 && t <= (int)m_Dimensions[3] && t > 0 ) || (t == 0) );
}

void mitk::Image::Expand(unsigned int timeSteps)
{
  if(timeSteps < 1) itkExceptionMacro(<< "Invalid timestep in Image!");
  Superclass::Expand(timeSteps);
}

int mitk::Image::GetSliceIndex(int s, int t, int n) const
{
  if(IsValidSlice(s,t,n)==false) return false;
  return ((size_t)s)+((size_t) t)*m_Dimensions[2]+((size_t) n)*m_Dimensions[3]*m_Dimensions[2]; //??
}

int mitk::Image::GetVolumeIndex(int t, int n) const
{
  if(IsValidVolume(t,n)==false) return false;
  return ((size_t)t)+((size_t) n)*m_Dimensions[3]; //??
}

mitk::Image::ImageDataItemPointer mitk::Image::AllocateVolumeData(int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return AllocateVolumeData_unlocked(t, n, data, importMemoryManagement);
}

mitk::Image::ImageDataItemPointer mitk::Image::AllocateVolumeData_unlocked(int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  int pos;
  pos=GetVolumeIndex(t,n);

  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();

  ImageDataItemPointer vol;
  mitk::PixelType chPixelType = this->m_ImageDescriptor->GetChannelTypeById(n);

  if (m_Data == nullptr) {
    m_Data = new ImageDataItem(GetImageDescriptor(), 0, nullptr, true);
  }
  // Create volume data item on the main data
  vol = new ImageDataItem(*m_Data, GetImageDescriptor(), t, 3u, nullptr, false, m_OffsetTable[3] * (ptypeSize) * t);
  if (data != nullptr) {
    std::memcpy(vol->GetData(), data, m_OffsetTable[3] * (ptypeSize));
  }

  m_Volumes[pos]=vol;
  return vol;
}

unsigned int* mitk::Image::GetDimensions() const
{
  return m_Dimensions;
}

bool mitk::Image::SetImportVolume(void *data, int t, int n, ImportMemoryManagementType importMemoryManagement)
{
  if (!IsValidVolume(t, n)) {
    MITK_WARN << "Can't import volume for time " << t << " and channel " << n;
    return false;
  }

  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();
  ImageDataItemPointer vol;
  if (IsVolumeSet(t, n)) {
    vol = AllocateVolumeData(t, n, data, importMemoryManagement);
  } else {
    vol = GetVolumeData(t, n, data, importMemoryManagement);
  }

  if (vol.GetPointer() == nullptr) return false;
  if (vol->GetData() != data) {
    std::memcpy(vol->GetData(), data, m_OffsetTable[3] * (ptypeSize));
  }
  this->m_ImageDescriptor->GetChannelDescriptor(n).SetData(vol->GetData());
  vol->Modified();
  vol->SetComplete(true);
  Modified();

  return true;
}

void mitk::Image::Clear()
{
  Superclass::Clear();
  delete [] m_Dimensions;
  m_Dimensions = nullptr;
}

void mitk::Image::SetGeometry(BaseGeometry* aGeometry3D)
{
  // Please be aware of the 0.5 offset/pixel-center issue! See Geometry documentation for further information

  if(aGeometry3D->GetImageGeometry()==false)
  {
    MITK_INFO << "WARNING: Applied a non-image geometry onto an image. Please be SURE that this geometry is pixel-center-based! If it is not, you need to call Geometry3D->ChangeImageGeometryConsideringOriginOffset(true) before calling image->setGeometry(..)\n";
  }
  Superclass::SetGeometry(aGeometry3D);
  for (TimeStepType step = 0; step < GetTimeGeometry()->CountTimeSteps(); ++step)
    GetTimeGeometry()->GetGeometryForTimeStep(step)->ImageGeometryOn();
}

void mitk::Image::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  if(m_Initialized)
  {
    unsigned char i;
    os << indent << " Dimension: " << m_Dimension << std::endl;
    os << indent << " Dimensions: ";
    for(i=0; i < m_Dimension; ++i)
      os << GetDimension(i) << " ";
    os << std::endl;

    for(unsigned int ch=0; ch < this->m_ImageDescriptor->GetNumberOfChannels(); ch++)
    {
      mitk::PixelType chPixelType = this->m_ImageDescriptor->GetChannelTypeById(ch);

      os << indent << " Channel: " << this->m_ImageDescriptor->GetChannelName(ch) << std::endl;
      os << indent << " PixelType: " << chPixelType.GetPixelTypeAsString() << std::endl;
      os << indent << " BytesPerElement: " << chPixelType.GetSize() << std::endl;
      os << indent << " ComponentType: " << chPixelType.GetComponentTypeAsString() << std::endl;
      os << indent << " NumberOfComponents: " << chPixelType.GetNumberOfComponents() << std::endl;
      os << indent << " BitsPerComponent: " << chPixelType.GetBitsPerComponent() << std::endl;
    }

  }
  else
  {
    os << indent << " Image not initialized: m_Initialized: false" << std::endl;
  }

  Superclass::PrintSelf(os,indent);
}

bool mitk::Image::IsRotated() const
{
  const mitk::BaseGeometry* geo = this->GetGeometry();
  bool ret = false;

  if(geo)
  {
    const vnl_matrix_fixed<ScalarType, 3, 3> & mx = geo->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix();
    mitk::ScalarType ref = 0;
    for(short k = 0; k < 3; ++k)
      ref += mx[k][k];
    ref/=1000;  // Arbitrary value; if a non-diagonal (nd) element is bigger then this, matrix is considered nd.

    for(short i = 0; i < 3; ++i)
    {
      for(short j = 0; j < 3; ++j)
      {
        if(i != j)
        {
          if(std::abs(mx[i][j]) > ref) // matrix is nd
            ret = true;
        }
      }
    }
  }
  return ret;
}

mitk::ScalarType mitk::Image::GetScalarValueMin(int t) const
{
  return m_ImageStatistics->GetScalarValueMin(t);
}

//## \brief Get the maximum for scalar images
mitk::ScalarType mitk::Image::GetScalarValueMax(int t) const
{
  return m_ImageStatistics->GetScalarValueMax(t);
}

//## \brief Get the second smallest value for scalar images
mitk::ScalarType mitk::Image::GetScalarValue2ndMin(int t) const
{
  return m_ImageStatistics->GetScalarValue2ndMin(t);
}

mitk::ScalarType mitk::Image::GetScalarValueMinNoRecompute( unsigned int t ) const
{
  return m_ImageStatistics->GetScalarValueMinNoRecompute(t);
}

mitk::ScalarType mitk::Image::GetScalarValue2ndMinNoRecompute( unsigned int t ) const
{
  return m_ImageStatistics->GetScalarValue2ndMinNoRecompute(t);
}

mitk::ScalarType mitk::Image::GetScalarValue2ndMax(int t) const
{
  return m_ImageStatistics->GetScalarValue2ndMax(t);
}

mitk::ScalarType mitk::Image::GetScalarValueMaxNoRecompute( unsigned int t) const
{
  return m_ImageStatistics->GetScalarValueMaxNoRecompute(t);
}

mitk::ScalarType mitk::Image::GetScalarValue2ndMaxNoRecompute( unsigned int t ) const
{
  return m_ImageStatistics->GetScalarValue2ndMaxNoRecompute(t);
}

mitk::ScalarType mitk::Image::GetCountOfMinValuedVoxels(int t ) const
{
  return m_ImageStatistics->GetCountOfMinValuedVoxels(t);
}

mitk::ScalarType mitk::Image::GetCountOfMaxValuedVoxels(int t) const
{
  return m_ImageStatistics->GetCountOfMaxValuedVoxels(t);
}

unsigned int mitk::Image::GetCountOfMaxValuedVoxelsNoRecompute( unsigned int t  ) const
{
  return m_ImageStatistics->GetCountOfMaxValuedVoxelsNoRecompute(t);
}

unsigned int mitk::Image::GetCountOfMinValuedVoxelsNoRecompute( unsigned int t ) const
{
  return m_ImageStatistics->GetCountOfMinValuedVoxelsNoRecompute(t);
}

bool mitk::Equal(const mitk::Image* leftHandSide, const mitk::Image* rightHandSide, ScalarType eps, bool verbose)
{
  if((leftHandSide == nullptr)  || (rightHandSide == nullptr))
  {
    MITK_ERROR << "mitk::Equal(const mitk::Image* leftHandSide, const mitk::Image* rightHandSide, ScalarType eps, bool verbose) does not work with NULL pointer input.";
    return false;
  }
  return mitk::Equal( *leftHandSide, *rightHandSide, eps, verbose);
}

bool mitk::Equal(const mitk::Image& leftHandSide, const mitk::Image& rightHandSide, ScalarType eps, bool verbose)
{
  bool returnValue = true;

  // Dimensionality
  if( rightHandSide.GetDimension() != leftHandSide.GetDimension() )
  {
    if(verbose)
    {
      MITK_INFO << "[( Image )] Dimensionality differs.";
      MITK_INFO << "leftHandSide is " << leftHandSide.GetDimension()
       << "rightHandSide is " << rightHandSide.GetDimension();
    }
    returnValue = false;
  }

  // Pair-wise dimension (size) comparison
  unsigned int minDimensionality = std::min(rightHandSide.GetDimension(),leftHandSide.GetDimension());
  for( unsigned int i=0; i< minDimensionality; ++i)
  {
    if( rightHandSide.GetDimension(i) != leftHandSide.GetDimension(i) )
    {
      returnValue = false;
      if(verbose)
      {
        MITK_INFO << "[( Image )] dimension differs.";
        MITK_INFO << "leftHandSide->GetDimension("<<i<<") is " << leftHandSide.GetDimension(i)
         << "rightHandSide->GetDimension("<<i<<") is " << rightHandSide.GetDimension(i);
      }
    }
  }

  // Pixeltype
  mitk::PixelType pixelTypeRightHandSide = rightHandSide.GetPixelType();
  mitk::PixelType pixelTypeLeftHandSide = leftHandSide.GetPixelType();
  if( !( pixelTypeRightHandSide == pixelTypeLeftHandSide ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Image )] PixelType differs.";
      MITK_INFO << "leftHandSide is " << pixelTypeLeftHandSide.GetTypeAsString()
       << "rightHandSide is " << pixelTypeRightHandSide.GetTypeAsString();
    }
    returnValue = false;
  }

  // Geometries
  if( !mitk::Equal(  *leftHandSide.GetGeometry(),
                     *rightHandSide.GetGeometry(), eps, verbose) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Image )] Geometries differ.";
    }
    returnValue = false;
  }

  // Pixel values - default mode [ 0 threshold in difference ]
  // compare only if all previous checks were successfull, otherwise the ITK filter will throw an exception
  if( returnValue )
  {
    mitk::CompareImageDataFilter::Pointer compareFilter = mitk::CompareImageDataFilter::New();

    compareFilter->SetInput(0, &rightHandSide);
    compareFilter->SetInput(1, &leftHandSide);
    compareFilter->SetTolerance(eps);
    compareFilter->Update();

    if(( !compareFilter->GetResult() ) )
    {
      returnValue = false;
      if(verbose)
      {
        MITK_INFO << "[(Image)] Pixel values differ: ";
        compareFilter->GetCompareResults().PrintSelf();
      }
    }
  }

  return returnValue;
}
