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


#include "mitkImageSource.h"

mitk::ImageSource::ImageSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type TOutputImage
  OutputImageType::Pointer output
    = static_cast<OutputImageType*>(this->MakeOutput(0).GetPointer()); 
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}

/**
 *
 */
//##ModelId=3E1886F103DB
mitk::ImageSource::DataObjectPointer mitk::ImageSource::MakeOutput(unsigned int)
{
	return static_cast<itk::DataObject*>(OutputImageType::New().GetPointer());
}
  
/**
 *
 */
mitk::ImageSource::OutputImageType* mitk::ImageSource::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
    {
    return 0;
    }
  
  return static_cast<OutputImageType*>
                     (this->BaseProcess::GetOutput(0));
}

  
/**
 *
 */
//##ModelId=3E1886F101F0
mitk::ImageSource::OutputImageType* mitk::ImageSource::GetOutput(unsigned int idx)
{
  return static_cast<OutputImageType*>
                     (this->ProcessObject::GetOutput(idx));
}


/**
 *
 */
//##ModelId=3E1886F1024A
void mitk::ImageSource::SetOutput(OutputImageType *output)
{
  itkWarningMacro(<< "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
  BaseProcess::SetNthOutput(0, output);
}


/**
 * 
 */
//##ModelId=3E1886F102A5
void mitk::ImageSource::GraftOutput(OutputImageType *graft)
{
  this->GraftNthOutput(0, graft);
}


/**
 * 
 */

//##ModelId=3E1886F10313
void mitk::ImageSource::GraftNthOutput(unsigned int idx, OutputImageType* graft)
{
  itkWarningMacro(<< "GraftNthOutput(): This method is not yet implemented for mitk. Implement it before using!!" );
  assert(false);
  if (idx < this->GetNumberOfOutputs())
    {
    OutputImageType * output = this->GetOutput(idx);

    if (output && graft)
      {
      // grab a handle to the bulk data of the specified data object
//      output->SetPixelContainer( graft->GetPixelContainer() ); @FIXME!!!!
      
      // copy the region ivars of the specified data object
      output->SetRequestedRegion( graft );//graft->GetRequestedRegion() );
//      output->SetLargestPossibleRegion( graft->GetLargestPossibleRegion() ); @FIXME!!!!
//      output->SetBufferedRegion( graft->GetBufferedRegion() ); @FIXME!!!!
      
      // copy the meta-information
      output->CopyInformation( graft );
      }
    }
}

//----------------------------------------------------------------------------

//##ModelId=3E1886F202F6
int mitk::ImageSource::SplitRequestedRegion(int i, int num, OutputImageRegionType& splitRegion)
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
  int valuesPerThread = (int)ceil(range/(double)num);
  int maxThreadIdUsed = (int)ceil(range/(double)valuesPerThread) - 1;

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

//##ModelId=3E1886F2022E
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

//##ModelId=3E1886F200CF
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

//##ModelId=3E1886F2010B
void mitk::ImageSource::ThreadedGenerateData(const OutputImageRegionType&, int)
{
  itkExceptionMacro("subclass should override this method!!!");
}

// Callback routine used by the threading library. This routine just calls
// the ThreadedGenerateData method after setting the correct region for this
// thread. 

//##ModelId=3E1886F30063
ITK_THREAD_RETURN_TYPE mitk::ImageSource::ThreaderCallback( void *arg )
{
  ThreadStruct *str;
  int total, threadId, threadCount;

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

//##ModelId=3E3BCBD4000C
void* mitk::ImageSource::GetData()
{
    Update();
    return GetOutput()->GetData();
}

//##ModelId=3E3BCBD0024B
ipPicDescriptor* mitk::ImageSource::GetPic()
{
    Update();
    return GetOutput()->GetPic();
}

//##ModelId=3E3BCBD502FD
vtkImageData* mitk::ImageSource::GetVtkImageData()
{
    Update();
    return GetOutput()->GetVtkImageData();
}
