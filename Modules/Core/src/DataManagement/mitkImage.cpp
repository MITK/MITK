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

//MITK
#include "mitkImage.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkPixelTypeMultiplex.h"
#include <mitkProportionalTimeGeometry.h>
#include "mitkCompareImageDataFilter.h"
#include "mitkImageVtkReadAccessor.h"
#include "mitkImageVtkWriteAccessor.h"

//VTK
#include <vtkImageData.h>

//ITK
#include <itkMutexLockHolder.h>

//Other
#include <cmath>

#define FILL_C_ARRAY( _arr, _size, _value) for(unsigned int i=0u; i<_size; i++) \
{ _arr[i] = _value; }


mitk::Image::Image() :
  m_Dimension(0), m_Dimensions(NULL), m_ImageDescriptor(NULL), m_OffsetTable(NULL), m_CompleteData(NULL),
  m_ImageStatistics(NULL)
{
  m_Dimensions = new unsigned int[MAX_IMAGE_DIMENSIONS];
  FILL_C_ARRAY( m_Dimensions, MAX_IMAGE_DIMENSIONS, 0u);

  m_Initialized = false;
}

mitk::Image::Image(const Image &other) : SlicedData(other), m_Dimension(0), m_Dimensions(NULL),
  m_ImageDescriptor(NULL), m_OffsetTable(NULL), m_CompleteData(NULL), m_ImageStatistics(NULL)
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
  Clear();
  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 3;
  m_ReferenceCountLock.Unlock();
  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0;
  m_ReferenceCountLock.Unlock();
  if(m_OffsetTable != NULL)
    delete [] m_OffsetTable;

  if(m_ImageStatistics != NULL)
    delete m_ImageStatistics;
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

void* mitk::Image::GetData()
{
  if(m_Initialized==false)
  {
    if(GetSource().IsNull())
      return NULL;
    if(GetSource()->Updating()==false)
      GetSource()->UpdateOutputInformation();
  }
  m_CompleteData=GetChannelData();

  // update channel's data
  // if data was not available at creation point, the m_Data of channel descriptor is NULL
  // if data present, it won't be overwritten
  m_ImageDescriptor->GetChannelDescriptor(0).SetData(m_CompleteData->GetData());

  return m_CompleteData->GetData();
}


template <class T>
void AccessPixel( const mitk::PixelType ptype, void* data, const unsigned int offset, double& value )
{
  value = 0.0;
  if( data == NULL ) return;

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

    mitkPixelTypeMultiplex3( AccessPixel, ptype, this->GetData(), offset, value );
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

vtkImageData* mitk::Image::GetVtkImageData(int t, int n)
{
  if(m_Initialized==false)
  {
    if(GetSource().IsNull())
      return NULL;
    if(GetSource()->Updating()==false)
      GetSource()->UpdateOutputInformation();
  }
  ImageDataItemPointer volume=GetVolumeData(t, n);
  return volume.GetPointer() == NULL ? NULL : volume->GetVtkImageAccessor(this)->GetVtkImageData();
}

const vtkImageData* mitk::Image::GetVtkImageData(int t, int n) const
{
  if(m_Initialized==false)
  {
    if(GetSource().IsNull())
      return NULL;
    if(GetSource()->Updating()==false)
      GetSource()->UpdateOutputInformation();
  }
  ImageDataItemPointer volume=GetVolumeData(t, n);
  return volume.GetPointer() == NULL ? NULL : volume->GetVtkImageAccessor(this)->GetVtkImageData();
}

mitk::Image::ImageDataItemPointer mitk::Image::GetSliceData(int s, int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return GetSliceData_unlocked(s, t, n, data, importMemoryManagement);
}

mitk::Image::ImageDataItemPointer mitk::Image::GetSliceData_unlocked(int s, int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  if(IsValidSlice(s,t,n)==false) return NULL;

  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();

  // slice directly available?
  int pos=GetSliceIndex(s,t,n);
  if(m_Slices[pos].GetPointer()!=NULL)
  {
    return m_Slices[pos];
  }

  // is slice available as part of a volume that is available?
  ImageDataItemPointer sl, ch, vol;
  vol=m_Volumes[GetVolumeIndex(t,n)];
  if((vol.GetPointer()!=NULL) && (vol->IsComplete()))
  {
    sl=new ImageDataItem(*vol, m_ImageDescriptor, t, 2, data, importMemoryManagement == ManageMemory, ((size_t) s)*m_OffsetTable[2]*(ptypeSize));
    sl->SetComplete(true);
    return m_Slices[pos]=sl;
  }

  // is slice available as part of a channel that is available?
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))
  {
    sl=new ImageDataItem(*ch, m_ImageDescriptor, t, 2, data, importMemoryManagement == ManageMemory, (((size_t) s)*m_OffsetTable[2]+((size_t) t)*m_OffsetTable[3])*(ptypeSize));
    sl->SetComplete(true);
    return m_Slices[pos]=sl;
  }

  // slice is unavailable. Can we calculate it?
  if((GetSource().IsNotNull()) && (GetSource()->Updating()==false))
  {
    // ... wir mussen rechnen!!! ....
    m_RequestedRegion.SetIndex(0, 0);
    m_RequestedRegion.SetIndex(1, 0);
    m_RequestedRegion.SetIndex(2, s);
    m_RequestedRegion.SetIndex(3, t);
    m_RequestedRegion.SetIndex(4, n);
    m_RequestedRegion.SetSize(0, m_Dimensions[0]);
    m_RequestedRegion.SetSize(1, m_Dimensions[1]);
    m_RequestedRegion.SetSize(2, 1);
    m_RequestedRegion.SetSize(3, 1);
    m_RequestedRegion.SetSize(4, 1);
    m_RequestedRegionInitialized=true;
    GetSource()->Update();
    if(IsSliceSet_unlocked(s,t,n))
      //yes: now we can call ourselves without the risk of a endless loop (see "if" above)
      return GetSliceData_unlocked(s,t,n,data,importMemoryManagement);
    else
      return NULL;
  }
  else
  {
    ImageDataItemPointer item = AllocateSliceData_unlocked(s,t,n,data,importMemoryManagement);
    item->SetComplete(true);
    return item;
  }
}

mitk::Image::ImageDataItemPointer mitk::Image::GetVolumeData(int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return GetVolumeData_unlocked(t, n, data, importMemoryManagement);
}
mitk::Image::ImageDataItemPointer mitk::Image::GetVolumeData_unlocked(int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  if(IsValidVolume(t,n)==false) return NULL;

  ImageDataItemPointer ch, vol;

  // volume directly available?
  int pos=GetVolumeIndex(t,n);
  vol=m_Volumes[pos];
  if((vol.GetPointer()!=NULL) && (vol->IsComplete()))
    return vol;

  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();

  // is volume available as part of a channel that is available?
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))
  {
    vol=new ImageDataItem(*ch, m_ImageDescriptor, t, 3, data, importMemoryManagement == ManageMemory, (((size_t) t)*m_OffsetTable[3])*(ptypeSize));
    vol->SetComplete(true);
    return m_Volumes[pos]=vol;
  }

  // let's see if all slices of the volume are set, so that we can (could) combine them to a volume
  bool complete=true;
  unsigned int s;
  for(s=0;s<m_Dimensions[2];++s)
  {
    if(m_Slices[GetSliceIndex(s,t,n)].GetPointer()==NULL)
    {
      complete=false;
      break;
    }
  }
  if(complete)
  {
    // if there is only single slice we do not need to combine anything
    if(m_Dimensions[2]<=1)
    {
      ImageDataItemPointer sl;
      sl=GetSliceData_unlocked(0,t,n,data,importMemoryManagement);
      vol=new ImageDataItem(*sl, m_ImageDescriptor, t, 3, data, importMemoryManagement == ManageMemory);
      vol->SetComplete(true);
    }
    else
    {
      mitk::PixelType chPixelType = this->m_ImageDescriptor->GetChannelTypeById(n);

      vol=m_Volumes[pos];
      // ok, let's combine the slices!
      if(vol.GetPointer()==NULL)
      {
        vol=new ImageDataItem( chPixelType, t, 3, m_Dimensions, NULL, true);
      }
      vol->SetComplete(true);
      size_t size=m_OffsetTable[2]*(ptypeSize);
      for(s=0;s<m_Dimensions[2];++s)
      {
        int posSl;
        ImageDataItemPointer sl;
        posSl=GetSliceIndex(s,t,n);

        sl=m_Slices[posSl];
        if(sl->GetParent()!=vol)
        {
          // copy data of slices in volume
          size_t offset = ((size_t) s)*size;
          std::memcpy(static_cast<char*>(vol->GetData())+offset, sl->GetData(), size);

          // FIXME mitkIpPicDescriptor * pic = sl->GetPicDescriptor();

          // replace old slice with reference to volume
          sl=new ImageDataItem(*vol, m_ImageDescriptor, t, 2, data, importMemoryManagement == ManageMemory, ((size_t) s)*size);
          sl->SetComplete(true);
          //mitkIpFuncCopyTags(sl->GetPicDescriptor(), pic);
          m_Slices[posSl]=sl;
        }
      }
      //if(vol->GetPicDescriptor()->info->tags_head==NULL)
      //  mitkIpFuncCopyTags(vol->GetPicDescriptor(), m_Slices[GetSliceIndex(0,t,n)]->GetPicDescriptor());
    }
    return m_Volumes[pos]=vol;
  }

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
      return NULL;
  }
  else
  {
    ImageDataItemPointer item = AllocateVolumeData_unlocked(t,n,data,importMemoryManagement);
    item->SetComplete(true);
    return item;
  }

}

mitk::Image::ImageDataItemPointer mitk::Image::GetChannelData(int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return GetChannelData_unlocked(n, data, importMemoryManagement);
}

mitk::Image::ImageDataItemPointer mitk::Image::GetChannelData_unlocked(int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  if(IsValidChannel(n)==false) return NULL;
  ImageDataItemPointer ch, vol;
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))
    return ch;

  // let's see if all volumes are set, so that we can (could) combine them to a channel
  if(IsChannelSet_unlocked(n))
  {
    // if there is only one time frame we do not need to combine anything
    if(m_Dimensions[3]<=1)
    {
      vol=GetVolumeData_unlocked(0,n,data,importMemoryManagement);
      ch=new ImageDataItem(*vol, m_ImageDescriptor, 0, m_ImageDescriptor->GetNumberOfDimensions(), data, importMemoryManagement == ManageMemory);
      ch->SetComplete(true);
    }
    else
    {
      const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();

      ch=m_Channels[n];
      // ok, let's combine the volumes!
      if(ch.GetPointer()==NULL)
        ch=new ImageDataItem(this->m_ImageDescriptor, -1, NULL, true);
      ch->SetComplete(true);
      size_t size=m_OffsetTable[m_Dimension-1]*(ptypeSize);
      unsigned int t;
      ImageDataItemPointerArray::iterator slicesIt = m_Slices.begin()+n*m_Dimensions[2]*m_Dimensions[3];
      for(t=0;t<m_Dimensions[3];++t)
      {
        int posVol;
        ImageDataItemPointer vol;

        posVol=GetVolumeIndex(t,n);
        vol=GetVolumeData_unlocked(t,n,data,importMemoryManagement);

        if(vol->GetParent()!=ch)
        {
          // copy data of volume in channel
          size_t offset = ((size_t) t)*m_OffsetTable[3]*(ptypeSize);
          std::memcpy(static_cast<char*>(ch->GetData())+offset, vol->GetData(), size);

          // REVEIW FIX mitkIpPicDescriptor * pic = vol->GetPicDescriptor();

          // replace old volume with reference to channel
          vol=new ImageDataItem(*ch, m_ImageDescriptor, t, 3, data, importMemoryManagement == ManageMemory, offset);
          vol->SetComplete(true);
          //mitkIpFuncCopyTags(vol->GetPicDescriptor(), pic);

          m_Volumes[posVol]=vol;

          // get rid of slices - they may point to old volume
          ImageDataItemPointer dnull=NULL;
          for(unsigned int i = 0; i < m_Dimensions[2]; ++i, ++slicesIt)
          {
            assert(slicesIt != m_Slices.end());
            *slicesIt = dnull;
          }
        }
      }
      // REVIEW FIX
      //   if(ch->GetPicDescriptor()->info->tags_head==NULL)
      //     mitkIpFuncCopyTags(ch->GetPicDescriptor(), m_Volumes[GetVolumeIndex(0,n)]->GetPicDescriptor());
    }
    return m_Channels[n]=ch;
  }

  // channel is unavailable. Can we calculate it?
  if((GetSource().IsNotNull()) && (GetSource()->Updating()==false))
  {
    // ... wir muessen rechnen!!! ....
    m_RequestedRegion.SetIndex(0, 0);
    m_RequestedRegion.SetIndex(1, 0);
    m_RequestedRegion.SetIndex(2, 0);
    m_RequestedRegion.SetIndex(3, 0);
    m_RequestedRegion.SetIndex(4, n);
    m_RequestedRegion.SetSize(0, m_Dimensions[0]);
    m_RequestedRegion.SetSize(1, m_Dimensions[1]);
    m_RequestedRegion.SetSize(2, m_Dimensions[2]);
    m_RequestedRegion.SetSize(3, m_Dimensions[3]);
    m_RequestedRegion.SetSize(4, 1);
    m_RequestedRegionInitialized=true;
    GetSource()->Update();
    // did it work?
    if(IsChannelSet_unlocked(n))
      //yes: now we can call ourselves without the risk of a endless loop (see "if" above)
      return GetChannelData_unlocked(n,data,importMemoryManagement);
    else
      return NULL;
  }
  else
  {
    ImageDataItemPointer item = AllocateChannelData_unlocked(n,data,importMemoryManagement);
    item->SetComplete(true);
    return item;
  }
}

bool mitk::Image::IsSliceSet(int s, int t, int n) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return IsSliceSet_unlocked(s, t, n);
}

bool mitk::Image::IsSliceSet_unlocked(int s, int t, int n) const
{
  if(IsValidSlice(s,t,n)==false) return false;

  if(m_Slices[GetSliceIndex(s,t,n)].GetPointer()!=NULL)
  {
    return true;
  }

  ImageDataItemPointer ch, vol;
  vol=m_Volumes[GetVolumeIndex(t,n)];
  if((vol.GetPointer()!=NULL) && (vol->IsComplete()))
  {
    return true;
  }
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))
  {
    return true;
  }
  return false;
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
  if((vol.GetPointer()!=NULL) && (vol->IsComplete()))
    return true;

  // is volume available as part of a channel that is available?
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))
    return true;

  // let's see if all slices of the volume are set, so that we can (could) combine them to a volume
  unsigned int s;
  for(s=0;s<m_Dimensions[2];++s)
  {
    if(m_Slices[GetSliceIndex(s,t,n)].GetPointer()==NULL)
    {
      return false;
    }
  }
  return true;
}

bool mitk::Image::IsChannelSet(int n) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return IsChannelSet_unlocked(n);
}

bool mitk::Image::IsChannelSet_unlocked(int n) const
{
  if(IsValidChannel(n)==false) return false;
  ImageDataItemPointer ch, vol;
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))

    return true;
  // let's see if all volumes are set, so that we can (could) combine them to a channel
  unsigned int t;
  for(t=0;t<m_Dimensions[3];++t)
  {
    if(IsVolumeSet_unlocked(t,n)==false)
    {
      return false;
    }
  }
  return true;
}

bool mitk::Image::SetSlice(const void *data, int s, int t, int n)
{
  // const_cast is no risk for ImportMemoryManagementType == CopyMemory
  return SetImportSlice(const_cast<void*>(data), s, t, n, CopyMemory);
}

bool mitk::Image::SetVolume(const void *data, int t, int n)
{
  // const_cast is no risk for ImportMemoryManagementType == CopyMemory
  return SetImportVolume(const_cast<void*>(data), t, n, CopyMemory);
}

bool mitk::Image::SetChannel(const void *data, int n)
{
  // const_cast is no risk for ImportMemoryManagementType == CopyMemory
  return SetImportChannel(const_cast<void*>(data), n, CopyMemory);
}

bool mitk::Image::SetImportSlice(void *data, int s, int t, int n, ImportMemoryManagementType importMemoryManagement)
{
  if(IsValidSlice(s,t,n)==false) return false;
  ImageDataItemPointer sl;
  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();

  if(IsSliceSet(s,t,n))
  {
    sl=GetSliceData(s,t,n,data,importMemoryManagement);
    if(sl->GetManageMemory()==false)
    {
      sl=AllocateSliceData(s,t,n,data,importMemoryManagement);
      if(sl.GetPointer()==NULL) return false;
    }
    if ( sl->GetData() != data )
      std::memcpy(sl->GetData(), data, m_OffsetTable[2]*(ptypeSize));
    sl->Modified();
    //we have changed the data: call Modified()!
    Modified();
  }
  else
  {
    sl=AllocateSliceData(s,t,n,data,importMemoryManagement);
    if(sl.GetPointer()==NULL) return false;
    if ( sl->GetData() != data )
      std::memcpy(sl->GetData(), data, m_OffsetTable[2]*(ptypeSize));
    //we just added a missing slice, which is not regarded as modification.
    //Therefore, we do not call Modified()!
  }
  return true;
}

bool mitk::Image::SetImportVolume(void *data, int t, int n, ImportMemoryManagementType importMemoryManagement)
{
  if(IsValidVolume(t,n)==false) return false;

  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();
  ImageDataItemPointer vol;
  if(IsVolumeSet(t,n))
  {
    vol=GetVolumeData(t,n,data,importMemoryManagement);
    if(vol->GetManageMemory()==false)
    {
      vol=AllocateVolumeData(t,n,data,importMemoryManagement);
      if(vol.GetPointer()==NULL) return false;
    }
    if ( vol->GetData() != data )
      std::memcpy(vol->GetData(), data, m_OffsetTable[3]*(ptypeSize));
    vol->Modified();
    vol->SetComplete(true);
    //we have changed the data: call Modified()!
    Modified();
  }
  else
  {
    vol=AllocateVolumeData(t,n,data,importMemoryManagement);
    if(vol.GetPointer()==NULL) return false;
    if ( vol->GetData() != data )
    {
      std::memcpy(vol->GetData(), data, m_OffsetTable[3]*(ptypeSize));
    }
    vol->SetComplete(true);
    this->m_ImageDescriptor->GetChannelDescriptor(n).SetData( vol->GetData() );
    //we just added a missing Volume, which is not regarded as modification.
    //Therefore, we do not call Modified()!
  }
  return true;
}

bool mitk::Image::SetImportChannel(void *data, int n, ImportMemoryManagementType importMemoryManagement)
{
  if(IsValidChannel(n)==false) return false;

  // channel descriptor

  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();

  ImageDataItemPointer ch;
  if(IsChannelSet(n))
  {
    ch=GetChannelData(n,data,importMemoryManagement);
    if(ch->GetManageMemory()==false)
    {
      ch=AllocateChannelData(n,data,importMemoryManagement);
      if(ch.GetPointer()==NULL) return false;
    }
    if ( ch->GetData() != data )
      std::memcpy(ch->GetData(), data, m_OffsetTable[4]*(ptypeSize));
    ch->Modified();
    ch->SetComplete(true);
    //we have changed the data: call Modified()!
    Modified();
  }
  else
  {
    ch=AllocateChannelData(n,data,importMemoryManagement);
    if(ch.GetPointer()==NULL) return false;
    if ( ch->GetData() != data )
      std::memcpy(ch->GetData(), data, m_OffsetTable[4]*(ptypeSize));
    ch->SetComplete(true);

    this->m_ImageDescriptor->GetChannelDescriptor(n).SetData( ch->GetData() );
    //we just added a missing Channel, which is not regarded as modification.
    //Therefore, we do not call Modified()!
  }
  return true;
}

void mitk::Image::Initialize()
{
  ImageDataItemPointerArray::iterator it, end;
  for( it=m_Slices.begin(), end=m_Slices.end(); it!=end; ++it )
  {
    (*it)=NULL;
  }
  for( it=m_Volumes.begin(), end=m_Volumes.end(); it!=end; ++it )
  {
    (*it)=NULL;
  }
  for( it=m_Channels.begin(), end=m_Channels.end(); it!=end; ++it )
  {
    (*it)=NULL;
  }
  m_CompleteData = NULL;

  if( m_ImageStatistics == NULL)
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
    m_Dimensions = NULL;
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
  for (TimeStepType step = 0; step < timeGeometry->CountTimeSteps(); ++step)
  {
    timeGeometry->GetGeometryForTimeStep(step)->ImageGeometryOn();
  }
  SetTimeGeometry(timeGeometry);

  ImageDataItemPointer dnull=NULL;

  m_Channels.assign(GetNumberOfChannels(), dnull);

  m_Volumes.assign(GetNumberOfChannels()*m_Dimensions[3], dnull);

  m_Slices.assign(GetNumberOfChannels()*m_Dimensions[3]*m_Dimensions[2], dnull);

  ComputeOffsetTable();

  Initialize();

  m_Initialized = true;
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
  }
  else
    Superclass::SetGeometry(geometry.GetGeometryForTimeStep(0));
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
  if(vtkimagedata==NULL) return;

  m_Dimension=vtkimagedata->GetDataDimension();
  unsigned int i, *tmpDimensions=new unsigned int[m_Dimension>4?m_Dimension:4];
  for(i=0;i<m_Dimension;++i) tmpDimensions[i]=vtkimagedata->GetDimensions()[i];
  if(m_Dimension<4)
  {
    unsigned int *p;
    for(i=0,p=tmpDimensions+m_Dimension;i<4-m_Dimension;++i, ++p)
      *p=1;
  }

  if(pDim>=0)
  {
    tmpDimensions[1]=pDim;
    if(m_Dimension < 2)
      m_Dimension = 2;
  }
  if(sDim>=0)
  {
    tmpDimensions[2]=sDim;
    if(m_Dimension < 3)
      m_Dimension = 3;
  }
  if(tDim>=0)
  {
    tmpDimensions[3]=tDim;
    if(m_Dimension < 4)
      m_Dimension = 4;
  }


  switch ( vtkimagedata->GetScalarType() )
  {
  case VTK_BIT:
  case VTK_CHAR:
    //pixelType.Initialize(typeid(char), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<char>(), m_Dimension, tmpDimensions, channels);
    break;
  case VTK_UNSIGNED_CHAR:
    //pixelType.Initialize(typeid(unsigned char), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<unsigned char>(), m_Dimension, tmpDimensions, channels);
    break;
  case VTK_SHORT:
    //pixelType.Initialize(typeid(short), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<short>(), m_Dimension, tmpDimensions, channels);
    break;
  case VTK_UNSIGNED_SHORT:
    //pixelType.Initialize(typeid(unsigned short), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<unsigned short>(), m_Dimension, tmpDimensions, channels);
    break;
  case VTK_INT:
    //pixelType.Initialize(typeid(int), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<int>(), m_Dimension, tmpDimensions, channels);
    break;
  case VTK_UNSIGNED_INT:
    //pixelType.Initialize(typeid(unsigned int), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<unsigned int>(), m_Dimension, tmpDimensions, channels);
    break;
  case VTK_LONG:
    //pixelType.Initialize(typeid(long), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<long>(), m_Dimension, tmpDimensions, channels);
    break;
  case VTK_UNSIGNED_LONG:
    //pixelType.Initialize(typeid(unsigned long), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<unsigned long>(), m_Dimension, tmpDimensions, channels);
    break;
  case VTK_FLOAT:
    //pixelType.Initialize(typeid(float), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<float>(), m_Dimension, tmpDimensions, channels);
    break;
  case VTK_DOUBLE:
    //pixelType.Initialize(typeid(double), vtkimagedata->GetNumberOfScalarComponents());
    Initialize(mitk::MakeScalarPixelType<double>(), m_Dimension, tmpDimensions, channels);
    break;
  default:
    break;
  }
  /*
  Initialize(pixelType,
    m_Dimension,
    tmpDimensions,
    channels);
*/

  const double *spacinglist = vtkimagedata->GetSpacing();
  Vector3D spacing;
  FillVector3D(spacing, spacinglist[0], 1.0, 1.0);
  if(m_Dimension>=2)
    spacing[1]=spacinglist[1];
  if(m_Dimension>=3)
    spacing[2]=spacinglist[2];

  // access origin of vtkImage
  Point3D origin;
  double vtkorigin[3];
  vtkimagedata->GetOrigin(vtkorigin);
  FillVector3D(origin, vtkorigin[0], 0.0, 0.0);
  if(m_Dimension>=2)
    origin[1]=vtkorigin[1];
  if(m_Dimension>=3)
    origin[2]=vtkorigin[2];

  SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);

  // re-initialize PlaneGeometry with origin and direction
  PlaneGeometry* planeGeometry = static_cast<PlaneGeometry*>(slicedGeometry->GetPlaneGeometry(0));
  planeGeometry->SetOrigin(origin);

  // re-initialize SlicedGeometry3D
  slicedGeometry->SetOrigin(origin);
  slicedGeometry->SetSpacing(spacing);

  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(slicedGeometry, m_Dimensions[3]);
  SetTimeGeometry(timeGeometry);

  delete [] tmpDimensions;
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
  if(m_OffsetTable!=NULL)
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

mitk::Image::ImageDataItemPointer mitk::Image::AllocateSliceData(int s, int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return AllocateSliceData_unlocked(s, t, n, data, importMemoryManagement);
}

mitk::Image::ImageDataItemPointer mitk::Image::AllocateSliceData_unlocked(int s, int t, int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  int pos;
  pos=GetSliceIndex(s,t,n);

  const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();

  // is slice available as part of a volume that is available?
  ImageDataItemPointer sl, ch, vol;
  vol=m_Volumes[GetVolumeIndex(t,n)];
  if(vol.GetPointer()!=NULL)
  {
    sl=new ImageDataItem(*vol, m_ImageDescriptor, t, 2, data, importMemoryManagement == ManageMemory, ((size_t) s)*m_OffsetTable[2]*(ptypeSize));
    sl->SetComplete(true);
    return m_Slices[pos]=sl;
  }

  // is slice available as part of a channel that is available?
  ch=m_Channels[n];
  if(ch.GetPointer()!=NULL)
  {
    sl=new ImageDataItem(*ch, m_ImageDescriptor, t, 2, data, importMemoryManagement == ManageMemory, (((size_t) s)*m_OffsetTable[2]+((size_t) t)*m_OffsetTable[3])*(ptypeSize));
    sl->SetComplete(true);
    return m_Slices[pos]=sl;
  }

  // allocate new volume (instead of a single slice to keep data together!)
  m_Volumes[GetVolumeIndex(t,n)]=vol=AllocateVolumeData_unlocked(t,n,NULL,importMemoryManagement);
  sl=new ImageDataItem(*vol, m_ImageDescriptor, t, 2, data, importMemoryManagement == ManageMemory, ((size_t) s)*m_OffsetTable[2]*(ptypeSize));
  sl->SetComplete(true);
  return m_Slices[pos]=sl;

  ////ALTERNATIVE:
  //// allocate new slice
  //sl=new ImageDataItem(*m_PixelType, 2, m_Dimensions);
  //m_Slices[pos]=sl;
  //return vol;
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

  // is volume available as part of a channel that is available?
  ImageDataItemPointer ch, vol;
  ch=m_Channels[n];
  if(ch.GetPointer()!=NULL)
  {
    vol=new ImageDataItem(*ch, m_ImageDescriptor, t, 3, data,importMemoryManagement == ManageMemory, (((size_t) t)*m_OffsetTable[3])*(ptypeSize));
    return m_Volumes[pos]=vol;
  }

  mitk::PixelType chPixelType = this->m_ImageDescriptor->GetChannelTypeById(n);

  // allocate new volume
  if(importMemoryManagement == CopyMemory)
  {
    vol=new ImageDataItem( chPixelType, t, 3, m_Dimensions, NULL, true);
    if(data != NULL)
      std::memcpy(vol->GetData(), data, m_OffsetTable[3]*(ptypeSize));
  }
  else
  {
    vol=new ImageDataItem( chPixelType, t, 3, m_Dimensions, data, importMemoryManagement == ManageMemory);
  }
  m_Volumes[pos]=vol;
  return vol;
}

mitk::Image::ImageDataItemPointer mitk::Image::AllocateChannelData(int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  MutexHolder lock(m_ImageDataArraysLock);
  return AllocateChannelData_unlocked(n, data, importMemoryManagement);
}

mitk::Image::ImageDataItemPointer mitk::Image::AllocateChannelData_unlocked(int n, void *data, ImportMemoryManagementType importMemoryManagement) const
{
  ImageDataItemPointer ch;
  // allocate new channel
  if(importMemoryManagement == CopyMemory)
  {
    const size_t ptypeSize = this->m_ImageDescriptor->GetChannelTypeById(n).GetSize();

    ch=new ImageDataItem(this->m_ImageDescriptor, -1, NULL, true);
    if(data != NULL)
      std::memcpy(ch->GetData(), data, m_OffsetTable[4]*(ptypeSize));
  }
  else
  {
    ch=new ImageDataItem(this->m_ImageDescriptor, -1, data, importMemoryManagement == ManageMemory);
  }
  m_Channels[n]=ch;
  return ch;
}

unsigned int* mitk::Image::GetDimensions() const
{
  return m_Dimensions;
}

void mitk::Image::Clear()
{
  Superclass::Clear();
  delete [] m_Dimensions;
  m_Dimensions = NULL;
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
  unsigned char i;
  if(m_Initialized)
  {
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
  if((leftHandSide == NULL)  || (rightHandSide == NULL))
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
