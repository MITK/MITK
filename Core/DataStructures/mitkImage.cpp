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

#include "mitkHistogramGenerator.h"

#include "mitkImage.h"
#include "mitkImageDataItem.h"

#include "mitkBaseProcess.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkPicHelper.h"
#include "mitkImageTimeSelector.h"

#include "ipFunc/ipFunc.h"
#include <itkSmartPointerForwardReference.txx>
#include <vtkImageData.h>
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>
#include <mitkImageWriter.h>
#include "mitkDataTreeNodeFactory.h"


//template class itk::SmartPointerForwardReference<ImageDataItem>;


mitk::Image::Image() : 
  m_Dimension(0), m_Dimensions(NULL), m_OffsetTable(NULL),
  m_CompleteData(NULL), m_PixelType(NULL),
  m_TimeSelectorForExtremaObject(NULL)
{
  m_CountOfMinValuedVoxels.resize(1, 0);
  m_CountOfMaxValuedVoxels.resize(1, 0);
  m_ScalarMin.resize(1, itk::NumericTraits<ScalarType>::max());
  m_ScalarMax.resize(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_Scalar2ndMin.resize(1, itk::NumericTraits<ScalarType>::max());
  m_Scalar2ndMax.resize(1, itk::NumericTraits<ScalarType>::NonpositiveMin());

  m_Initialized = false;
  mitk::HistogramGenerator::Pointer generator = mitk::HistogramGenerator::New();
  m_HistogramGeneratorObject = generator;
  generator->SetImage(this);
  this->UnRegister();
}

mitk::Image::~Image()
{
  Clear();
  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 3;
  m_ReferenceCountLock.Unlock();
  m_HistogramGeneratorObject = NULL;
  m_TimeSelectorForExtremaObject = NULL;
  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0;
  m_ReferenceCountLock.Unlock();
  delete [] m_OffsetTable;
}

const mitk::PixelType& mitk::Image::GetPixelType(int /*n*/) const
{
  return m_PixelType;
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
    if(GetSource()==NULL)
      return NULL;
    if(GetSource()->Updating()==false)
      GetSource()->UpdateOutputInformation();
  }
  m_CompleteData=GetChannelData();
  return m_CompleteData->GetData();
}

vtkImageData* mitk::Image::GetVtkImageData(int t, int n)
{
  if(m_Initialized==false)
  {
    if(GetSource()==NULL)
      return NULL;
    if(GetSource()->Updating()==false)
      GetSource()->UpdateOutputInformation();
  }
  ImageDataItemPointer volume=GetVolumeData(t, n);
  if(volume.GetPointer()==NULL || volume->GetVtkImageData() == NULL)
    return NULL;

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  float *fspacing = const_cast<float *>(GetSlicedGeometry(t)->GetFloatSpacing());
  double dspacing[3] = {fspacing[0],fspacing[1],fspacing[2]};
  volume->GetVtkImageData()->SetSpacing( dspacing );
#else
  volume->GetVtkImageData()->SetSpacing(const_cast<float*>(GetSlicedGeometry(t)->GetFloatSpacing()));
#endif
  return volume->GetVtkImageData();
}

ipPicDescriptor* mitk::Image::GetPic()
{
  if(m_Initialized==false)
  {
    if(GetSource()==NULL)
      return NULL;
    if(GetSource()->Updating()==false)
      GetSource()->UpdateOutputInformation();
  }
  m_CompleteData=GetChannelData();
  if(m_CompleteData.GetPointer()==NULL) 
    return NULL;
  return m_CompleteData->GetPicDescriptor();
}

mitk::Image::ImageDataItemPointer mitk::Image::GetSliceData(int s, int t, int n, void *data, ImportMemoryManagementType importMemoryManagement)
{
  if(IsValidSlice(s,t,n)==false) return NULL;

  // slice directly available?
  int pos=GetSliceIndex(s,t,n);
  if(m_Slices[pos].GetPointer()!=NULL)
    return m_Slices[pos];

  // is slice available as part of a volume that is available?
  ImageDataItemPointer sl, ch, vol;
  vol=m_Volumes[GetVolumeIndex(t,n)];
  if((vol.GetPointer()!=NULL) && (vol->IsComplete()))
  {
    sl=new ImageDataItem(*vol, 2, data, importMemoryManagement == ManageMemory, ((size_t) s)*m_OffsetTable[2]*(m_PixelType.GetBpe()/8));
    sl->SetComplete(true);
    return m_Slices[pos]=sl;
  }

  // is slice available as part of a channel that is available?
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))
  {
    sl=new ImageDataItem(*ch, 2, data, importMemoryManagement == ManageMemory, (((size_t) s)*m_OffsetTable[2]+((size_t) t)*m_OffsetTable[3])*(m_PixelType.GetBpe()/8));
    sl->SetComplete(true);
    return m_Slices[pos]=sl;
  }

  // slice is unavailable. Can we calculate it?
  if((GetSource()!=NULL) && (GetSource()->Updating()==false))
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
    if(IsSliceSet(s,t,n))
      //yes: now we can call ourselves without the risk of a endless loop (see "if" above)
      return GetSliceData(s,t,n,data,importMemoryManagement);
    else
      return NULL;
  }
  else
  {
    ImageDataItemPointer item = AllocateSliceData(s,t,n,data,importMemoryManagement);
    item->SetComplete(true);
    return item;
  }
}

mitk::Image::ImageDataItemPointer mitk::Image::GetVolumeData(int t, int n, void *data, ImportMemoryManagementType importMemoryManagement)
{
  if(IsValidVolume(t,n)==false) return NULL;

  ImageDataItemPointer ch, vol;

  // volume directly available?
  int pos=GetVolumeIndex(t,n);
  vol=m_Volumes[pos];
  if((vol.GetPointer()!=NULL) && (vol->IsComplete()))
    return vol;

  // is volume available as part of a channel that is available?
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))
  {
    vol=new ImageDataItem(*ch, 3, data, importMemoryManagement == ManageMemory, (((size_t) t)*m_OffsetTable[3])*(m_PixelType.GetBpe()/8));
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
      sl=GetSliceData(0,t,n,data,importMemoryManagement);
      vol=new ImageDataItem(*sl, 3, data, importMemoryManagement == ManageMemory);
      vol->SetComplete(true);
    }
    else
    {
      vol=m_Volumes[pos];
      // ok, let's combine the slices!
      if(vol.GetPointer()==NULL)
        vol=new ImageDataItem(m_PixelType, 3, m_Dimensions, NULL, true);
      vol->SetComplete(true);
      size_t size=m_OffsetTable[2]*(m_PixelType.GetBpe()/8);
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
          memcpy(static_cast<char*>(vol->GetData())+offset, sl->GetData(), size);

          ipPicDescriptor * pic = sl->GetPicDescriptor();

          // replace old slice with reference to volume
          sl=new ImageDataItem(*vol, 2, data, importMemoryManagement == ManageMemory, ((size_t) s)*size);
          sl->SetComplete(true);
          ipFuncCopyTags(sl->GetPicDescriptor(), pic);
          m_Slices[posSl]=sl;
        }
      }
      if(vol->GetPicDescriptor()->info->tags_head==NULL)
        ipFuncCopyTags(vol->GetPicDescriptor(), m_Slices[GetSliceIndex(0,t,n)]->GetPicDescriptor());
    }
    return m_Volumes[pos]=vol;
  }

  // volume is unavailable. Can we calculate it?
  if((GetSource()!=NULL) && (GetSource()->Updating()==false))
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
    if(IsVolumeSet(t,n))
      //yes: now we can call ourselves without the risk of a endless loop (see "if" above)
      return GetVolumeData(t,n,data,importMemoryManagement);
    else
      return NULL;
  }
  else
  {
    ImageDataItemPointer item = AllocateVolumeData(t,n,data,importMemoryManagement);
    item->SetComplete(true);
    return item;
  }
}

mitk::Image::ImageDataItemPointer mitk::Image::GetChannelData(int n, void *data, ImportMemoryManagementType importMemoryManagement)
{
  if(IsValidChannel(n)==false) return NULL;
  ImageDataItemPointer ch, vol;
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))
    return ch;

  // let's see if all volumes are set, so that we can (could) combine them to a channel
  if(IsChannelSet(n))
  {
    // if there is only one time frame we do not need to combine anything
    if(m_Dimensions[3]<=1)
    {
      vol=GetVolumeData(0,n,data,importMemoryManagement);
      ch=new ImageDataItem(*vol, 3, data, importMemoryManagement == ManageMemory);
      ch->SetComplete(true);
    }
    else
    {
      ch=m_Channels[n];
      // ok, let's combine the volumes!
      if(ch.GetPointer()==NULL)
        ch=new ImageDataItem(m_PixelType, m_Dimension, m_Dimensions, NULL, true);
      ch->SetComplete(true);
      size_t size=m_OffsetTable[m_Dimension-1]*(m_PixelType.GetBpe()/8);
      unsigned int t;
      for(t=0;t<m_Dimensions[3];++t)
      {
        int posVol;
        ImageDataItemPointer vol;

        posVol=GetVolumeIndex(t,n);
        vol=GetVolumeData(t,n,data,importMemoryManagement);

        if(vol->GetParent()!=ch)
        {
          // copy data of volume in channel
          size_t offset = ((size_t) t)*m_OffsetTable[3]*(m_PixelType.GetBpe()/8);
          memcpy(static_cast<char*>(ch->GetData())+offset, vol->GetData(), size);

          ipPicDescriptor * pic = vol->GetPicDescriptor();

          // replace old volume with reference to channel
          vol=new ImageDataItem(*ch, 3, data, importMemoryManagement == ManageMemory, offset);
          vol->SetComplete(true);
          ipFuncCopyTags(vol->GetPicDescriptor(), pic);

          m_Volumes[posVol]=vol;
        }
        // get rid of slices - they may point to old volume
        ImageDataItemPointer dnull=NULL;
        m_Slices.assign(GetNumberOfChannels()*m_Dimensions[3]*m_Dimensions[2], dnull);
      }
      if(ch->GetPicDescriptor()->info->tags_head==NULL)
        ipFuncCopyTags(ch->GetPicDescriptor(), m_Volumes[GetVolumeIndex(0,n)]->GetPicDescriptor());
    }
    return m_Channels[n]=ch;
  }

  // channel is unavailable. Can we calculate it?
  if((GetSource()!=NULL) && (GetSource()->Updating()==false))
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
    if(IsChannelSet(n))
      //yes: now we can call ourselves without the risk of a endless loop (see "if" above)
      return GetChannelData(n,data,importMemoryManagement);
    else
      return NULL;
  }
  else
  {
    ImageDataItemPointer item = AllocateChannelData(n,data,importMemoryManagement);
    item->SetComplete(true);
    return item;
  }
}

bool mitk::Image::IsSliceSet(int s, int t, int n) const
{
  if(IsValidSlice(s,t,n)==false) return false;

  if(m_Slices[GetSliceIndex(s,t,n)].GetPointer()!=NULL)
    return true;

  ImageDataItemPointer ch, vol;
  vol=m_Volumes[GetVolumeIndex(t,n)];
  if((vol.GetPointer()!=NULL) && (vol->IsComplete()))
    return true;
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))
    return true;
  return false;
}

bool mitk::Image::IsVolumeSet(int t, int n) const
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
    if(m_Slices[GetSliceIndex(s,t,n)].GetPointer()==NULL)
      return false;
  return true;
}

bool mitk::Image::IsChannelSet(int n) const
{
  if(IsValidChannel(n)==false) return false;
  ImageDataItemPointer ch, vol;
  ch=m_Channels[n];
  if((ch.GetPointer()!=NULL) && (ch->IsComplete()))

    return true;
  // let's see if all volumes are set, so that we can (could) combine them to a channel
  unsigned int t;
  for(t=0;t<m_Dimensions[3];++t)
    if(IsVolumeSet(t,n)==false)
      return false;
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
  if(IsSliceSet(s,t,n))
  {
    sl=GetSliceData(s,t,n,data,importMemoryManagement);
    if(sl->GetManageMemory()==false)
    {
      sl=AllocateSliceData(s,t,n,data,importMemoryManagement);
      if(sl.GetPointer()==NULL) return false;
    }
    if ( sl->GetData() != data )
      memcpy(sl->GetData(), data, m_OffsetTable[2]*(m_PixelType.GetBpe()/8));
    sl->Modified();
    //we have changed the data: call Modified()! 
    Modified();
  }
  else
  {
    sl=AllocateSliceData(s,t,n,data,importMemoryManagement);
    if(sl.GetPointer()==NULL) return false;
    if ( sl->GetData() != data )
      memcpy(sl->GetData(), data, m_OffsetTable[2]*(m_PixelType.GetBpe()/8));   
    //we just added a missing slice, which is not regarded as modification.
    //Therefore, we do not call Modified()!
  }
  return true;
}

bool mitk::Image::SetImportVolume(void *data, int t, int n, ImportMemoryManagementType importMemoryManagement)
{
  if(IsValidVolume(t,n)==false) return false;
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
      memcpy(vol->GetData(), data, m_OffsetTable[3]*(m_PixelType.GetBpe()/8));
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
      memcpy(vol->GetData(), data, m_OffsetTable[3]*(m_PixelType.GetBpe()/8));
    }
    vol->SetComplete(true);
    //we just added a missing Volume, which is not regarded as modification.
    //Therefore, we do not call Modified()!
  }
  return true;
}

bool mitk::Image::SetImportChannel(void *data, int n, ImportMemoryManagementType importMemoryManagement)
{
  if(IsValidChannel(n)==false) return false;
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
      memcpy(ch->GetData(), data, m_OffsetTable[4]*(m_PixelType.GetBpe()/8));
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
      memcpy(ch->GetData(), data, m_OffsetTable[4]*(m_PixelType.GetBpe()/8));
    ch->SetComplete(true);
    //we just added a missing Channel, which is not regarded as modification.
    //Therefore, we do not call Modified()!
  }
  return true;
}

bool mitk::Image::SetPicSlice(const ipPicDescriptor *pic, int s, int t, int n, ImportMemoryManagementType /*importMemoryManagement*/)
{
  if(pic==NULL) return false;
  if(pic->dim!=2) return false;
  if((pic->n[0]!=m_Dimensions[0]) || (pic->n[1]!=m_Dimensions[1])) return false;
  if(SetSlice(pic->data,s,t,n)) //@todo: add geometry!
  {
    ImageDataItemPointer sl;
    sl=GetSliceData(s,t,n,NULL,CopyMemory);
    ipFuncCopyTags(sl->GetPicDescriptor(), const_cast<ipPicDescriptor *>(pic));
    return true;
  }
  else
    return false;
}

bool mitk::Image::SetPicVolume(const ipPicDescriptor *pic, int t, int n, ImportMemoryManagementType /*importMemoryManagement*/)
{
  if(pic==NULL) return false;
  if((pic->dim==2) && ((m_Dimension==2) || ((m_Dimension>2) && (m_Dimensions[2]==1)))) return SetPicSlice(pic, 0, t, n);
  if(pic->dim!=3) return false;
  if((pic->n[0]!=m_Dimensions[0]) || (pic->n[1]!=m_Dimensions[1]) || (pic->n[2]!=m_Dimensions[2])) return false;
  if(SetVolume(pic->data,t,n)) //@todo: add geometry!
  {
    ImageDataItemPointer vol;
    vol=GetVolumeData(t,n,NULL,CopyMemory);
    ipFuncCopyTags(vol->GetPicDescriptor(), const_cast<ipPicDescriptor *>(pic));
    return true;
  }
  else
    return false;
}

bool mitk::Image::SetPicChannel(const ipPicDescriptor *pic, int n, ImportMemoryManagementType /*importMemoryManagement*/)
{
  if(pic==NULL) return false;
  if(pic->dim<=3) return SetPicVolume(pic, 0, n);
  if(pic->dim!=m_Dimension) return false;
  unsigned int i;
  for(i=0;i<m_Dimension; ++i)
  {
    if(pic->n[i]!=m_Dimensions[i]) return false;
  }
  if(SetChannel(pic->data,n)) //@todo: add geometry!
  {
    ImageDataItemPointer ch;
    ch=GetChannelData(n,NULL,CopyMemory);
    // commented the next line, because 
    // it crashes when called from mitkDICOMFileReader for the Live3D data
    // ipFuncCopyTags(ch->GetPicDescriptor(), pic);
    return true;
  }
  else
    return false;
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
 
  this->GetTimeSelector(); // just to create m_TimeSelectorForExtremaObject 

  SetRequestedRegionToLargestPossibleRegion();
}

mitk::ImageTimeSelector* mitk::Image::GetTimeSelector() const
{
  if(m_TimeSelectorForExtremaObject.IsNull())
  {
    m_TimeSelectorForExtremaObject = ImageTimeSelector::New().GetPointer();

    ImageTimeSelector* timeSelector = static_cast<mitk::ImageTimeSelector*>( m_TimeSelectorForExtremaObject.GetPointer() );
    timeSelector->SetInput(this);
	this->UnRegister();
  }

  return static_cast<ImageTimeSelector*>( m_TimeSelectorForExtremaObject.GetPointer() );
}

void mitk::Image::Initialize(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions, unsigned int channels)
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

  m_Dimensions=new unsigned int[m_Dimension>4?m_Dimension:4];
  memcpy(m_Dimensions, dimensions, sizeof(unsigned int)*m_Dimension);
  if(m_Dimension<4)
  {
    unsigned int *p;
    for(i=0,p=m_Dimensions+m_Dimension;i<4-m_Dimension;++i, ++p)
      *p=1;
  }

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

  m_PixelType=type;

  PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();
  planegeometry->InitializeStandardPlane(m_Dimensions[0], m_Dimensions[1]);

  SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New();
  slicedGeometry->InitializeEvenlySpaced(planegeometry, m_Dimensions[2]);

  if(dimension>=4)
  {
    TimeBounds timebounds;
    timebounds[0] = 0.0;
    timebounds[1] = 1.0;
    slicedGeometry->SetTimeBounds(timebounds);
  }

  TimeSlicedGeometry::Pointer timeSliceGeometry = TimeSlicedGeometry::New();
  timeSliceGeometry->InitializeEvenlyTimed(slicedGeometry, m_Dimensions[3]);

  SetGeometry(timeSliceGeometry);

  ImageDataItemPointer dnull=NULL;

  m_Channels.assign(GetNumberOfChannels(), dnull);

  m_Volumes.assign(GetNumberOfChannels()*m_Dimensions[3], dnull);

  m_Slices.assign(GetNumberOfChannels()*m_Dimensions[3]*m_Dimensions[2], dnull);

  ComputeOffsetTable();

  Initialize();

  m_Initialized = true;
}

void mitk::Image::Initialize(const mitk::PixelType& type, const mitk::Geometry3D& geometry, unsigned int channels, int tDim ) 
{
  unsigned int dimensions[5];
  dimensions[0] = (unsigned int)(geometry.GetExtent(0)+0.5);
  dimensions[1] = (unsigned int)(geometry.GetExtent(1)+0.5);
  dimensions[2] = (unsigned int)(geometry.GetExtent(2)+0.5);
  dimensions[3] = 0;
  dimensions[4] = 0;

  unsigned int dimension = 2;
  if ( dimensions[2] > 1 )
    dimension = 3;

  if ( tDim > 0)
  {
    dimensions[3] = tDim;
  }
  else
  {
    const mitk::TimeSlicedGeometry* timeGeometry = dynamic_cast<const mitk::TimeSlicedGeometry*>(&geometry);
    if ( timeGeometry != NULL ) 
    {
      dimensions[3] = timeGeometry->GetTimeSteps();
    }
  }

  if ( dimensions[3] > 1 )
    dimension = 4;

  Initialize( type, dimension, dimensions, channels );

  SetGeometry(static_cast<Geometry3D*>(geometry.Clone().GetPointer()));
  
  mitk::BoundingBox::BoundsArrayType bounds = geometry.GetBoundingBox()->GetBounds();
  if( (bounds[0] != 0.0) || (bounds[2] != 0.0) || (bounds[4] != 0.0) )
  {
    SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);
  
    mitk::Point3D origin; origin.Fill(0.0);
    slicedGeometry->IndexToWorld(origin, origin);

    bounds[1]-=bounds[0]; bounds[3]-=bounds[2]; bounds[5]-=bounds[4];
    bounds[0] = 0.0;      bounds[2] = 0.0;      bounds[4] = 0.0;

    slicedGeometry->SetBounds(bounds);
    slicedGeometry->GetIndexToWorldTransform()->SetOffset(origin.Get_vnl_vector().data_block());  
  
    GetTimeSlicedGeometry()->InitializeEvenlyTimed(slicedGeometry, m_Dimensions[3]);
  }
}

void mitk::Image::Initialize(const mitk::PixelType& type, int sDim, const mitk::Geometry2D& geometry2d, bool flipped, unsigned int channels, int tDim )
{
  SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New();
  slicedGeometry->InitializeEvenlySpaced(static_cast<Geometry2D*>(geometry2d.Clone().GetPointer()), sDim, flipped);
  Initialize(type, *slicedGeometry, channels, tDim);
}

void mitk::Image::Initialize(const mitk::Image* image) 
{
  Initialize(*image->GetPixelType().GetTypeId(), *image->GetTimeSlicedGeometry());
}

void mitk::Image::Initialize(vtkImageData* vtkimagedata, int channels, int tDim, int sDim)
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

  mitk::PixelType pixelType;

  switch ( vtkimagedata->GetScalarType() ) 
  {
  case VTK_BIT: 
  case VTK_CHAR: 
    pixelType.Initialize(typeid(char), vtkimagedata->GetNumberOfScalarComponents());
    break;
  case VTK_UNSIGNED_CHAR: 
    pixelType.Initialize(typeid(unsigned char), vtkimagedata->GetNumberOfScalarComponents());
    break;
  case VTK_SHORT: 
    pixelType.Initialize(typeid(short), vtkimagedata->GetNumberOfScalarComponents());
    break;
  case VTK_UNSIGNED_SHORT: 
    pixelType.Initialize(typeid(unsigned short), vtkimagedata->GetNumberOfScalarComponents());
    break;
  case VTK_INT: 
    pixelType.Initialize(typeid(int), vtkimagedata->GetNumberOfScalarComponents());
    break;
  case VTK_UNSIGNED_INT: 
    pixelType.Initialize(typeid(unsigned int), vtkimagedata->GetNumberOfScalarComponents());
    break;
  case VTK_LONG: 
    pixelType.Initialize(typeid(long), vtkimagedata->GetNumberOfScalarComponents());
    break;
  case VTK_UNSIGNED_LONG: 
    pixelType.Initialize(typeid(unsigned long), vtkimagedata->GetNumberOfScalarComponents());
    break;
  case VTK_FLOAT:
    pixelType.Initialize(typeid(float), vtkimagedata->GetNumberOfScalarComponents());
    break;
  case VTK_DOUBLE: 
    pixelType.Initialize(typeid(double), vtkimagedata->GetNumberOfScalarComponents());
    break;
  default:
    break;
  }
  Initialize(pixelType, 
    m_Dimension, 
    tmpDimensions,
    channels);

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  const double *spacinglist = vtkimagedata->GetSpacing();
#else
  const float *spacinglist = vtkimagedata->GetSpacing();
#endif
  Vector3D spacing;
  FillVector3D(spacing, spacinglist[0], 1.0, 1.0);
  if(m_Dimension>=2)
    spacing[1]=spacinglist[1];
  if(m_Dimension>=3)
    spacing[2]=spacinglist[2];

  // access origin of vtkImage
    Point3D origin;
  vtkFloatingPointType vtkorigin[3];
  vtkimagedata->GetOrigin(vtkorigin);
  FillVector3D(origin, vtkorigin[0], 0.0, 0.0);
  if(m_Dimension>=2)
    origin[1]=vtkorigin[1];
  if(m_Dimension>=3)
    origin[2]=vtkorigin[2];

  SlicedGeometry3D* slicedGeometry = GetSlicedGeometry(0);

  // re-initialize PlaneGeometry with origin and direction
  PlaneGeometry* planeGeometry = static_cast<PlaneGeometry*>(slicedGeometry->GetGeometry2D(0));
  planeGeometry->SetOrigin(origin);

  // re-initialize SlicedGeometry3D
  slicedGeometry->SetSpacing(spacing);
  GetTimeSlicedGeometry()->InitializeEvenlyTimed(slicedGeometry, m_Dimensions[3]);

  delete [] tmpDimensions;
}

void mitk::Image::Initialize(const ipPicDescriptor* pic, int channels, int tDim, int sDim)
{
  if(pic==NULL) return;

  Clear();

  m_Dimension=pic->dim;

  m_Dimensions=new unsigned int[m_Dimension>4?m_Dimension:4];
  memcpy(m_Dimensions, pic->n, sizeof(unsigned int)*m_Dimension);
  if(m_Dimension<4)
  {
    unsigned int i, *p;
    for(i=0,p=m_Dimensions+m_Dimension;i<4-m_Dimension;++i, ++p)
      *p=1;
  }

  if(sDim>=0)
  {
    m_Dimensions[2]=sDim;
    if(m_Dimension < 3)
      m_Dimension = 3;
  }
  if(tDim>=0)
  {
    m_Dimensions[3]=tDim;
    if(m_Dimension < 4)
      m_Dimension = 4;
  }

  unsigned int i;
  for(i=0;i<4;++i)
  {
    m_LargestPossibleRegion.SetIndex(i, 0);
    m_LargestPossibleRegion.SetSize (i, m_Dimensions[i]);
  }
  m_LargestPossibleRegion.SetIndex(i, 0);
  m_LargestPossibleRegion.SetSize(i, channels);

  m_PixelType=PixelType(pic);
  SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New(); 
  PicHelper::InitializeEvenlySpaced(pic, m_Dimensions[2], slicedGeometry);

  TimeSlicedGeometry::Pointer timeSliceGeometry = TimeSlicedGeometry::New();
  timeSliceGeometry->InitializeEvenlyTimed(slicedGeometry, m_Dimensions[3]);

  SetGeometry(timeSliceGeometry);  

  ImageDataItemPointer dnull=NULL;

  m_Channels.assign(GetNumberOfChannels(), dnull);

  m_Volumes.assign(GetNumberOfChannels()*m_Dimensions[3], dnull);

  m_Slices.assign(GetNumberOfChannels()*m_Dimensions[3]*m_Dimensions[2], dnull);

  ComputeOffsetTable();

  Initialize();

  m_Initialized = true;
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

mitk::Image::ImageDataItemPointer mitk::Image::AllocateSliceData(int s, int t, int n, void *data, ImportMemoryManagementType importMemoryManagement)
{
  int pos;
  pos=GetSliceIndex(s,t,n);

  // is slice available as part of a volume that is available?
  ImageDataItemPointer sl, ch, vol;
  vol=m_Volumes[GetVolumeIndex(t,n)];
  if(vol.GetPointer()!=NULL)
  {
    sl=new ImageDataItem(*vol, 2, data, importMemoryManagement == ManageMemory, ((size_t) s)*m_OffsetTable[2]*(m_PixelType.GetBpe()/8));
    sl->SetComplete(true);
    return m_Slices[pos]=sl;
  }

  // is slice available as part of a channel that is available?
  ch=m_Channels[n];
  if(ch.GetPointer()!=NULL)
  {
    sl=new ImageDataItem(*ch, 2, data, importMemoryManagement == ManageMemory, (((size_t) s)*m_OffsetTable[2]+((size_t) t)*m_OffsetTable[3])*(m_PixelType.GetBpe()/8));
    sl->SetComplete(true);
    return m_Slices[pos]=sl;
  }

  // allocate new volume (instead of a single slice to keep data together!)
  m_Volumes[GetVolumeIndex(t,n)]=vol=AllocateVolumeData(t,n,NULL,importMemoryManagement);
  sl=new ImageDataItem(*vol, 2, data, importMemoryManagement == ManageMemory, ((size_t) s)*m_OffsetTable[2]*(m_PixelType.GetBpe()/8));
  sl->SetComplete(true);
  return m_Slices[pos]=sl;

  ////ALTERNATIVE:
  //// allocate new slice
  //sl=new ImageDataItem(m_PixelType, 2, m_Dimensions);
  //m_Slices[pos]=sl;
  //return vol;
}

mitk::Image::ImageDataItemPointer mitk::Image::AllocateVolumeData(int t, int n, void *data, ImportMemoryManagementType importMemoryManagement)
{
  int pos;
  pos=GetVolumeIndex(t,n);

  // is volume available as part of a channel that is available?
  ImageDataItemPointer ch, vol;
  ch=m_Channels[n];
  if(ch.GetPointer()!=NULL)
  {
    vol=new ImageDataItem(*ch, 3, data, importMemoryManagement == ManageMemory, (((size_t) t)*m_OffsetTable[3])*(m_PixelType.GetBpe()/8));
    return m_Volumes[pos]=vol;
  }

  // allocate new volume
  if(importMemoryManagement == CopyMemory)
  {
    vol=new ImageDataItem(m_PixelType, 3, m_Dimensions, NULL, true);
    if(data != NULL)
      memcpy(vol->GetData(), data, m_OffsetTable[3]*(m_PixelType.GetBpe()/8));
  }
  else
  {
    vol=new ImageDataItem(m_PixelType, 3, m_Dimensions, data, importMemoryManagement == ManageMemory);
  }
  m_Volumes[pos]=vol;
  return vol;
}

mitk::Image::ImageDataItemPointer mitk::Image::AllocateChannelData(int n, void *data, ImportMemoryManagementType importMemoryManagement)
{
  ImageDataItemPointer ch;
  // allocate new channel
  if(importMemoryManagement == CopyMemory)
  {
    ch=new ImageDataItem(m_PixelType, m_Dimension, m_Dimensions, NULL, true);
    if(data != NULL)
      memcpy(ch->GetData(), data, m_OffsetTable[4]*(m_PixelType.GetBpe()/8));
  }
  else
  {
    ch=new ImageDataItem(m_PixelType, m_Dimension, m_Dimensions, data, importMemoryManagement == ManageMemory);
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

bool mitk::Image::WriteXMLData( XMLWriter& xmlWriter ) 
{
  std::string fileName = xmlWriter.GetRelativePath();
  std::string imageExtension = xmlWriter.GetImageExtension();
  if(!xmlWriter.IsFileExtension(imageExtension, fileName))
    fileName += imageExtension;

  if(xmlWriter.SaveSourceFiles()){
    mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
    imageWriter->SetInput( this );
    std::string absolutePath = xmlWriter.GetAbsolutePath().c_str();
    if(xmlWriter.IsFileExtension(imageExtension, absolutePath))
      // remove file extension
      absolutePath.erase(absolutePath.find(imageExtension), absolutePath.length()-absolutePath.find(imageExtension));
    imageWriter->SetFileName( absolutePath.c_str() );
    imageWriter->SetExtension( imageExtension.c_str() );
    imageWriter->Write();
    fileName = absolutePath + imageExtension;
  }
  xmlWriter.WriteProperty( "FILENAME", fileName.c_str() );

  mitk::Geometry3D* geomety = GetGeometry();

  if ( geomety )
    geomety->WriteXML( xmlWriter );

  return true;
}

bool mitk::Image::ReadXMLData( XMLReader& xmlReader )
{
  BaseData::ReadXMLData( xmlReader );
  std::string fileName;
  xmlReader.GetAttribute( XMLReader::FILENAME, fileName );
  std::cout << fileName << std::endl;

  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  try
  {
    factory->SetFileName( fileName.c_str() );
    factory->Update();
    mitk::DataTreeNode::Pointer tmpNode = factory->GetOutput( 0 );

    if ( tmpNode.IsNull() )
      return false;

    mitk::Image::Pointer tmpImage = dynamic_cast<mitk::Image*>(tmpNode->GetData());
    Initialize( tmpImage.GetPointer() );
    SetPicVolume( tmpImage->GetPic() );    
  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
    return false;
  }

  return true;
}

void mitk::Image::SetGeometry(Geometry3D* aGeometry3D)
{
  Superclass::SetGeometry(aGeometry3D);
  GetTimeSlicedGeometry()->ImageGeometryOn();
}

const mitk::Image::HistogramType* mitk::Image::GetScalarHistogram(int t) const
{
  mitk::ImageTimeSelector* timeSelector = this->GetTimeSelector();
  if(timeSelector!=NULL)
  {
    timeSelector->SetTimeNr(t);
    timeSelector->UpdateLargestPossibleRegion();

    mitk::HistogramGenerator* generator = static_cast<mitk::HistogramGenerator*>(m_HistogramGeneratorObject.GetPointer());
    generator->SetImage(timeSelector->GetOutput());
    generator->ComputeHistogram();
    return static_cast<const mitk::Image::HistogramType*>(generator->GetHistogram());
  }
  return NULL;
}

#include "mitkImageAccessByItk.h"

//#define BOUNDINGOBJECT_IGNORE

template < typename ItkImageType >
void mitk::_ComputeExtremaInItkImage(ItkImageType* itkImage, mitk::Image* mitkImage, int t)
{
  typename ItkImageType::RegionType region;
  region = itkImage->GetBufferedRegion();
  if(region.Crop(itkImage->GetRequestedRegion()) == false) return;
  if(region != itkImage->GetRequestedRegion()) return;

  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, region);
  typedef typename ItkImageType::PixelType TPixel;
  TPixel value = 0;

  mitkImage->m_CountOfMinValuedVoxels[t] = 0;
  mitkImage->m_CountOfMaxValuedVoxels[t] = 0;

  mitkImage->m_Scalar2ndMin[t]=
    mitkImage->m_ScalarMin[t] = itk::NumericTraits<ScalarType>::max();
  mitkImage->m_Scalar2ndMax[t]=
    mitkImage->m_ScalarMax[t] = itk::NumericTraits<ScalarType>::NonpositiveMin();

  while( !it.IsAtEnd() )
  {
    value = it.Get();  
    //  if ( (value > mitkImage->m_ScalarMin) && (value < mitkImage->m_Scalar2ndMin) )        mitkImage->m_Scalar2ndMin = value;  
    //  else if ( (value < mitkImage->m_ScalarMax) && (value > mitkImage->m_Scalar2ndMax) )   mitkImage->m_Scalar2ndMax = value;  
    //  else if (value > mitkImage->m_ScalarMax)                                              mitkImage->m_ScalarMax = value;
    //  else if (value < mitkImage->m_ScalarMin)                                              mitkImage->m_ScalarMin = value;

    // if numbers start with 2ndMin or 2ndMax and never have that value again, the previous above logic failed
#ifdef BOUNDINGOBJECT_IGNORE
    if( value > -32765)
    {
#endif
    // update min
    if ( value < mitkImage->m_ScalarMin[t] )
    {
        mitkImage->m_Scalar2ndMin[t] = mitkImage->m_ScalarMin[t];    mitkImage->m_ScalarMin[t] = value;
        mitkImage->m_CountOfMinValuedVoxels[t] = 1;
    }
    else if ( value == mitkImage->m_ScalarMin[t] )
    {
        ++mitkImage->m_CountOfMinValuedVoxels[t];
    }
    else if ( value < mitkImage->m_Scalar2ndMin[t] )
    {
        mitkImage->m_Scalar2ndMin[t] = value;
    }

    // update max
    if ( value > mitkImage->m_ScalarMax[t] )
    {
        mitkImage->m_Scalar2ndMax[t] = mitkImage->m_ScalarMax[t];    mitkImage->m_ScalarMax[t] = value;
        mitkImage->m_CountOfMaxValuedVoxels[t] = 1;
    }
    else if ( value == mitkImage->m_ScalarMax[t] )
    {
        ++mitkImage->m_CountOfMaxValuedVoxels[t];
    }
    else if ( value > mitkImage->m_Scalar2ndMax[t] )
    {
        mitkImage->m_Scalar2ndMax[t] = value;
    }
#ifdef BOUNDINGOBJECT_IGNORE
    }
#endif

    ++it;
  }

  //// guard for wrong 2dMin/Max on single constant value images
  if (mitkImage->m_ScalarMax[t] == mitkImage->m_ScalarMin[t])
  {
      mitkImage->m_Scalar2ndMax[t] = mitkImage->m_Scalar2ndMin[t] = mitkImage->m_ScalarMax[t];
  }
  mitkImage->m_LastRecomputeTimeStamp.Modified();
  //itkGenericOutputMacro(<<"extrema "<<itk::NumericTraits<TPixel>::NonpositiveMin()<<" "<<mitkImage->m_ScalarMin<<" "<<mitkImage->m_Scalar2ndMin<<" "<<mitkImage->m_Scalar2ndMax<<" "<<mitkImage->m_ScalarMax<<" "<<itk::NumericTraits<TPixel>::max());
}

bool mitk::Image::IsValidTimeStep(int t) const
{
  return ( ( m_Dimension >= 4 && t <= (int)m_Dimensions[3] && t > 0 ) || (t == 0) ); 
}

void mitk::Image::Expand( int timeSteps ) const
{
  if(timeSteps < 1) itkExceptionMacro(<< "Invalid timestep in Image!");
  if(! IsValidTimeStep( timeSteps-1 ) ) return;
  if(timeSteps > (int)m_ScalarMin.size() )
  {
    m_ScalarMin.resize(timeSteps, itk::NumericTraits<ScalarType>::max());
    m_ScalarMax.resize(timeSteps, itk::NumericTraits<ScalarType>::NonpositiveMin());
    m_Scalar2ndMin.resize(timeSteps, itk::NumericTraits<ScalarType>::max());
    m_Scalar2ndMax.resize(timeSteps, itk::NumericTraits<ScalarType>::NonpositiveMin());
    m_CountOfMinValuedVoxels.resize(timeSteps, 0);
    m_CountOfMaxValuedVoxels.resize(timeSteps, 0);
  }
}

void mitk::Image::ResetImageStatistics() const
{
  m_ScalarMin.assign(1, itk::NumericTraits<ScalarType>::max());
  m_ScalarMax.assign(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_Scalar2ndMin.assign(1, itk::NumericTraits<ScalarType>::max());
  m_Scalar2ndMax.assign(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_CountOfMinValuedVoxels.assign(1, 0);
  m_CountOfMaxValuedVoxels.assign(1, 0);
}

void mitk::Image::ComputeImageStatistics(int t) const
{
  if(this->m_PixelType.GetNumberOfComponents() == 1)
  {
    // timestep valid?
    if (!IsValidTimeStep(t)) return;

    // image modified?
    if (this->GetMTime() > m_LastRecomputeTimeStamp.GetMTime())
      this->ResetImageStatistics();

    // adapt vector length
    this->Expand(t+1);

    // do we have valid information already?
    if( m_ScalarMin[t] != itk::NumericTraits<ScalarType>::max() || 
      m_Scalar2ndMin[t] != itk::NumericTraits<ScalarType>::max() ) return; // Values already calculated before...

    // recompute
    mitk::ImageTimeSelector* timeSelector = this->GetTimeSelector();
    if(timeSelector!=NULL)
    {
      timeSelector->SetTimeNr(t);
      timeSelector->UpdateLargestPossibleRegion();
      mitk::Image* image = timeSelector->GetOutput();
      mitk::Image* thisImage = const_cast<Image*>(this);
      AccessByItk_2( image, _ComputeExtremaInItkImage, thisImage, t );
    }
  }
}


mitk::ScalarType mitk::Image::GetScalarValueMin(int t) const
{
  ComputeImageStatistics(t);
  return m_ScalarMin[t];
}

mitk::ScalarType mitk::Image::GetScalarValueMax(int t) const
{
  ComputeImageStatistics(t);
  return m_ScalarMax[t];
}

mitk::ScalarType mitk::Image::GetScalarValue2ndMin(int t) const
{
  ComputeImageStatistics(t);
  return m_Scalar2ndMin[t];
}

mitk::ScalarType mitk::Image::GetScalarValue2ndMax(int t) const
{
  ComputeImageStatistics(t);
  return m_Scalar2ndMax[t];
}

mitk::ScalarType mitk::Image::GetCountOfMinValuedVoxels(int t) const
{
  ComputeImageStatistics(t);
  return m_CountOfMinValuedVoxels[t];
}

mitk::ScalarType mitk::Image::GetCountOfMaxValuedVoxels(int t) const
{
  ComputeImageStatistics(t);
  return m_CountOfMaxValuedVoxels[t];
}

void mitk::Image::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  unsigned char i;
  if(m_Initialized)
  {
    os << indent << " PixelType: " << m_PixelType.GetTypeId()->name() << std::endl;
    os << indent << " BitsPerElement: " << m_PixelType.GetBpe() << std::endl;
    os << indent << " NumberOfComponents: " << m_PixelType.GetNumberOfComponents() << std::endl;
    os << indent << " BitsPerComponent: " << m_PixelType.GetBitsPerComponent() << std::endl;
    os << indent << " Dimension: " << m_Dimension << std::endl;
    os << indent << " Dimensions: ";
    for(i=0; i < m_Dimension; ++i)
      os << GetDimension(i) << " ";
    os << std::endl;
  }
  else
  {
    os << indent << " Image not initialized: m_Initialized: false" << std::endl;
  }

  Superclass::PrintSelf(os,indent);
}
