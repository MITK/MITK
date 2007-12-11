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


#include "mitkSurfaceToImageFilter.h"
#include "mitkGeometryClipImageFilter.h"
#include "mitkImageSliceSelector.h"

#include <fstream>

#include "mitkReferenceCountWatcher.h"

class TwoOutputsFilter : public mitk::SurfaceToImageFilter
{
public:
  mitkClassMacro(TwoOutputsFilter, SurfaceToImageFilter);
  itkNewMacro(Self);
protected:
  TwoOutputsFilter()
  {
    mitk::Image::Pointer output
      = static_cast<mitk::Image*>(this->MakeOutput(0).GetPointer()); 
    Superclass::SetNumberOfRequiredOutputs(2);
    Superclass::SetNthOutput(1, output.GetPointer());
  }

  virtual ~TwoOutputsFilter()
  {
  }
};

template <class FilterType, class InputType>
int runPipelineSmartPointerCorrectnessTestForFilterType(typename InputType::Pointer input)
{
  typename FilterType::Pointer filter;
  std::cout << "Testing " << typeid(FilterType).name() << "::New(): ";
  filter = FilterType::New();
  if (filter.IsNull()) 
  {
    std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else 
  {
    std::cout<<"[PASSED]"<<std::endl;
  } 

  std::cout << "Testing SetInput(" << typeid(FilterType).name() << "): ";
  filter->SetInput(input);
  std::cout<<"[PASSED]"<<std::endl;
  //std::cout << "Testing mitk::SurfaceToImageFilter::*TESTED_METHOD_DESCRIPTION: ";
  //// METHOD_TEST_CODE
  //if (filter.IsNull()) {
  //  std::cout<<"[FAILED]"<<std::endl;
  //  // return EXIT_FAILURE;
  //}
  //else {
  //std::cout<<"[PASSED]"<<std::endl;
  //} 

//#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
  //MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED test
  std::cout << "Initializing mitk::ReferenceCountWatcher: ";
  mitk::ReferenceCountWatcher::Pointer filterWatcher, filterOutputWatcher;
  filterWatcher = new mitk::ReferenceCountWatcher(filter, "filter1");
  filterOutputWatcher = new mitk::ReferenceCountWatcher(filter->GetOutput(), "filter1Output");
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED part1: "<<std::endl;
  try 
  {
    mitk::Image::Pointer output = filter->GetOutput();
    std::cout << "Testing to set filter to NULL, keeping reference to output:";
    filter = NULL;
    std::cout<<"[PASSED]"<<std::endl;
    std::cout << "Testing reference count of output: ";
    if(output->GetReferenceCount()!=2)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;
    std::cout << "Testing external reference count of output: ";
    if(output->GetExternalReferenceCount()!=1)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing to set output to NULL:";
    //output->GetSource()->DebugOn();
    //output->DebugOn();
    output = NULL;
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter:";
    if(filterWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter output:";
    if(filterOutputWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;
  }
  catch(...)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testing MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED part2: "<<std::endl;
  filter = FilterType::New();
  filterWatcher = new mitk::ReferenceCountWatcher(filter, "filter2");
  filterOutputWatcher = new mitk::ReferenceCountWatcher(filter->GetOutput(), "filter2Output");
  try 
  {
    std::cout << "Testing to set filter to NULL, keeping NO reference to output:";
    filter = NULL;
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter:";
    if(filterWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter output:";
    if(filterOutputWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;
  }
  catch(...)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testing MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED part3: "<<std::endl;
  try 
  {
    mitk::Image::Pointer output;
    {
      typename FilterType::Pointer localFilter = FilterType::New();
      filterWatcher = new mitk::ReferenceCountWatcher(localFilter, "filter3");
      filterOutputWatcher = new mitk::ReferenceCountWatcher(localFilter->GetOutput(), "filter3Output");
      output = localFilter->GetOutput();
      std::cout << "Testing running out of scope of filter, keeping reference to output:";
    }
    std::cout<<"[PASSED]"<<std::endl;
    std::cout << "Testing reference count of output: ";
    if(output->GetReferenceCount()!=2)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;
    std::cout << "Testing external reference count of output: ";
    if(output->GetExternalReferenceCount()!=1)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing to set output to NULL:";
    output = NULL;
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter:";
    if(filterWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter output:";
    if(filterOutputWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;
  }
  catch(...)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testing MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED part4: "<<std::endl;
  try 
  {
    mitk::Image::Pointer output;
    {
      typename FilterType::Pointer localFilter = FilterType::New();
      filterWatcher = new mitk::ReferenceCountWatcher(localFilter, "filter4");
      filterOutputWatcher = new mitk::ReferenceCountWatcher(localFilter->GetOutput(), "filter4Output");
      output = localFilter->GetOutput();
      std::cout << "Testing running out of scope of filter, keeping reference to output (as in part 3):";
    }
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing output->DisconnectPipeline(): ";
    output->DisconnectPipeline();
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of output: ";
    if(output->GetReferenceCount()!=1)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;
    std::cout << "Testing external reference count of output: ";
    if(output->GetExternalReferenceCount()!=1)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter:";
    if(filterWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing to set output to NULL:";
    output = NULL;
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter output:";
    if(filterOutputWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;
  }
  catch(...)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testing MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED part5: "<<std::endl;
  try 
  {
    {
      typename FilterType::Pointer localFilter = FilterType::New();
      filterWatcher = new mitk::ReferenceCountWatcher(localFilter, "filter5");
      filterOutputWatcher = new mitk::ReferenceCountWatcher(localFilter->GetOutput(), "filter5Output");
      std::cout << "Testing running out of scope of filter, keeping NO reference to output:";
    }
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter:";
    if(filterWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing reference count of filter output:";
    if(filterOutputWatcher->GetReferenceCount()!=0)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    else
      std::cout<<"[PASSED]"<<std::endl;
  }
  catch(...)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
//#endif
}

int mitkPipelineSmartPointerCorrectnessTest(int /*argc*/, char* /*argv*/[])
{
  int result;
  result = runPipelineSmartPointerCorrectnessTestForFilterType<mitk::SurfaceToImageFilter, mitk::Surface>(mitk::Surface::New());
  if( result != EXIT_SUCCESS )
    return result;
  std::cout << std::endl;

  result = runPipelineSmartPointerCorrectnessTestForFilterType<mitk::GeometryClipImageFilter, mitk::Image>(mitk::Image::New());
  if( result != EXIT_SUCCESS )
    return result;
  std::cout << std::endl;

  result = runPipelineSmartPointerCorrectnessTestForFilterType<TwoOutputsFilter, mitk::Surface>(mitk::Surface::New());
  if( result != EXIT_SUCCESS )
    return result;

  result = runPipelineSmartPointerCorrectnessTestForFilterType<mitk::ImageSliceSelector, mitk::Image>(mitk::Image::New());
  if( result != EXIT_SUCCESS )
    return result;
  
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
