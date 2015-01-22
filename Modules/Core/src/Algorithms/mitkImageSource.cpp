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


#include "mitkImageSource.h"

#include "mitkImageVtkReadAccessor.h"
#include "mitkImageVtkWriteAccessor.h"

mitk::ImageSource::ImageSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type TOutputImage
  OutputImageType::Pointer output
    = static_cast<OutputImageType*>(this->MakeOutput(0).GetPointer());
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

itk::DataObject::Pointer mitk::ImageSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
  return static_cast<itk::DataObject *>(mitk::Image::New().GetPointer());
}


itk::DataObject::Pointer mitk::ImageSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject *>(mitk::Image::New().GetPointer());
}


//----------------------------------------------------------------------------
unsigned int mitk::ImageSource::SplitRequestedRegion(unsigned int i, unsigned int num, OutputImageRegionType& splitRegion)
{
  // Get the output pointer
  OutputImageType * outputPtr = this->GetOutput();
  const SlicedData::SizeType& requestedRegionSize
    = outputPtr->GetRequestedRegion().GetSize();

  int splitAxis;
  SlicedData::IndexType splitIndex;
  SlicedData::SizeType splitSize;

  // Initialize the splitRegion to the output requested region
  splitRegion = outputPtr->GetRequestedRegion();
  splitIndex = splitRegion.GetIndex();
  splitSize = splitRegion.GetSize();

  // split on the outermost dimension available
  splitAxis = outputPtr->GetDimension() - 1;
  while (requestedRegionSize[splitAxis] == 1)
    {
    --splitAxis;
    if (splitAxis < 0)
      { // cannot split
      itkDebugMacro("  Cannot Split");
      return 1;
      }
    }

  // determine the actual number of pieces that will be generated
  SlicedData::SizeType::SizeValueType range = requestedRegionSize[splitAxis];
  unsigned int valuesPerThread = itk::Math::Ceil< unsigned int>(range / (double)num);
  unsigned int maxThreadIdUsed = itk::Math::Ceil< unsigned int>(range / (double)valuesPerThread) - 1;

  // Split the region
  if (i < maxThreadIdUsed)
    {
    splitIndex[splitAxis] += i*valuesPerThread;
    splitSize[splitAxis] = valuesPerThread;
    }
  if (i == maxThreadIdUsed)
    {
    splitIndex[splitAxis] += i*valuesPerThread;
    // last thread needs to process the "rest" dimension being split
    splitSize[splitAxis] = splitSize[splitAxis] - i*valuesPerThread;
    }

  // set the split region ivars
  splitRegion.SetIndex( splitIndex );
  splitRegion.SetSize( splitSize );

  itkDebugMacro("  Split Piece: " << splitRegion );

  return maxThreadIdUsed + 1;
}

//----------------------------------------------------------------------------

void mitk::ImageSource::AllocateOutputs()
{
  OutputImagePointer outputPtr;

  // Allocate the output memory
  for (unsigned int i=0; i < this->GetNumberOfOutputs(); i++)
    {
    outputPtr = this->GetOutput(i);
//    outputPtr->SetBufferedRegion(outputPtr->GetRequestedRegion()); @FIXME???
//    outputPtr->Allocate(); @FIXME???
    }
}

//----------------------------------------------------------------------------

void mitk::ImageSource::GenerateData()
{
  // Call a method that can be overriden by a subclass to allocate
  // memory for the filter's outputs
  this->AllocateOutputs();

  // Call a method that can be overridden by a subclass to perform
  // some calculations prior to splitting the main computations into
  // separate threads
  this->BeforeThreadedGenerateData();

  // Set up the multithreaded processing
  ThreadStruct str;
  str.Filter = this;

  this->GetMultiThreader()->SetNumberOfThreads(this->GetNumberOfThreads());
  this->GetMultiThreader()->SetSingleMethod(this->ThreaderCallback, &str);

  // multithread the execution
  this->GetMultiThreader()->SingleMethodExecute();

  // Call a method that can be overridden by a subclass to perform
  // some calculations after all the threads have completed
  this->AfterThreadedGenerateData();
}


//----------------------------------------------------------------------------
// The execute method created by the subclass.

void mitk::ImageSource::ThreadedGenerateData(const OutputImageRegionType&, itk::ThreadIdType)
{
  itkExceptionMacro("subclass should override this method!!!");
}

// Callback routine used by the threading library. This routine just calls
// the ThreadedGenerateData method after setting the correct region for this
// thread.

ITK_THREAD_RETURN_TYPE mitk::ImageSource::ThreaderCallback( void *arg )
{
  ThreadStruct *str;
  itk::ThreadIdType total, threadId, threadCount;

  threadId = ((itk::MultiThreader::ThreadInfoStruct *)(arg))->ThreadID;
  threadCount = ((itk::MultiThreader::ThreadInfoStruct *)(arg))->NumberOfThreads;

  str = (ThreadStruct *)(((itk::MultiThreader::ThreadInfoStruct *)(arg))->UserData);

  // execute the actual method with appropriate output region
  // first find out how many pieces extent can be split into.
  SlicedData::RegionType splitRegion;
  total = str->Filter->SplitRequestedRegion(threadId, threadCount,
                                            splitRegion);

  if (threadId < total)
    {
    str->Filter->ThreadedGenerateData(splitRegion, threadId);
    }
  // else
  //   {
  //   otherwise don't use this thread. Sometimes the threads dont
  //   break up very well and it is just as efficient to leave a
  //   few threads idle.
  //   }

  return ITK_THREAD_RETURN_VALUE;
}

void mitk::ImageSource::PrepareOutputs()
{
  Superclass::PrepareOutputs();
}

vtkImageData* mitk::ImageSource::GetVtkImageData()
{
  Update();
  return GetOutput()->GetVtkImageData();
}

const vtkImageData* mitk::ImageSource::GetVtkImageData() const
{
  return GetOutput()->GetVtkImageData();
}

mitkBaseDataSourceGetOutputDefinitions(mitk::ImageSource)
