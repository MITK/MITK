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


#include "mitkSurfaceToImageFilter.h"

#include <fstream>

#include "itkCommand.h"
class ReferenceCountWatcher : public itk::Object
{
public:
  typedef itk::SimpleMemberCommand<ReferenceCountWatcher> CommandType;

  mitkClassMacro(ReferenceCountWatcher, itk::Object);

protected:
  itk::Object* m_Object;
  std::string m_Comment;
  bool m_Deleted;
  CommandType::Pointer m_DeleteCommand;

public:
  ReferenceCountWatcher(itk::Object* o, const char *comment="") : m_Object(o), m_Comment(comment), m_Deleted(false)
  {
    m_DeleteCommand = CommandType::New();
    m_DeleteCommand->SetCallbackFunction(this, &ReferenceCountWatcher::DeleteObserver);
    if(m_Object!=NULL)
      m_Object->AddObserver(itk::DeleteEvent(), m_DeleteCommand);
  }
  int GetReferenceCount() const 
  {
    if(m_Object == NULL) return -1;
    if(m_Deleted) return 0;
    return m_Object->GetReferenceCount();
  }
  itkGetStringMacro(Comment);
protected:
  void DeleteObserver()
  {
    m_Deleted = true;
  }
};

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

template <class FilterType>
int runPipelineSmartPointerCorrectnessTestForFilterType()
{
  typename FilterType::Pointer filter;
  std::cout << "Testing " << typeid(FilterType).name() << "::New(): ";
  filter = FilterType::New();
  if (filter.IsNull()) 
  {
    std::cout<<"[FAILED]"<<std::endl;
     return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  } 

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
  std::cout << "Initializing ReferenceCountWatcher: ";
  ReferenceCountWatcher::Pointer filterWatcher, filterOutputWatcher;
  filterWatcher = new ReferenceCountWatcher(filter, "filter1");
  filterOutputWatcher = new ReferenceCountWatcher(filter->GetOutput(), "filter1Output");
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
  filterWatcher = new ReferenceCountWatcher(filter, "filter2");
  filterOutputWatcher = new ReferenceCountWatcher(filter->GetOutput(), "filter2Output");
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
      filterWatcher = new ReferenceCountWatcher(localFilter, "filter3");
      filterOutputWatcher = new ReferenceCountWatcher(localFilter->GetOutput(), "filter3Output");
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
    {
      typename FilterType::Pointer localFilter = FilterType::New();
      filterWatcher = new ReferenceCountWatcher(localFilter, "filter4");
      filterOutputWatcher = new ReferenceCountWatcher(localFilter->GetOutput(), "filter4Output");
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

int mitkPipelineSmartPointerCorrectnessTest(int argc, char* argv[])
{
  int result;
  result = runPipelineSmartPointerCorrectnessTestForFilterType<mitk::SurfaceToImageFilter>();
  if( result != EXIT_SUCCESS )
    return result;
  std::cout << std::endl;

  result = runPipelineSmartPointerCorrectnessTestForFilterType<TwoOutputsFilter>();
  if( result != EXIT_SUCCESS )
    return result;
  
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
