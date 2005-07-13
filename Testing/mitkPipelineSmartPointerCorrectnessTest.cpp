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

int mitkPipelineSmartPointerCorrectnessTest(int argc, char* argv[])
{
  mitk::SurfaceToImageFilter::Pointer filter;
  std::cout << "Testing mitk::Surface::New(): ";
  filter = mitk::SurfaceToImageFilter::New();
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
  //  return EXIT_FAILURE;
  //}
  //else {
  //std::cout<<"[PASSED]"<<std::endl;
  //} 

//#ifdef MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED
  //MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED test
  ReferenceCountWatcher::Pointer filterWatcher;
  filterWatcher = new ReferenceCountWatcher(filter, "filter1");
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
  }
  catch(...)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testing MITK_WEAKPOINTER_PROBLEM_WORKAROUND_ENABLED part2: "<<std::endl;
  filter = mitk::SurfaceToImageFilter::New();
  filterWatcher = new ReferenceCountWatcher(filter, "filter2");
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
      mitk::SurfaceToImageFilter::Pointer localFilter = mitk::SurfaceToImageFilter::New();
      filterWatcher = new ReferenceCountWatcher(localFilter, "filter4");
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
      mitk::SurfaceToImageFilter::Pointer localFilter = mitk::SurfaceToImageFilter::New();
      filterWatcher = new ReferenceCountWatcher(localFilter, "filter4");
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
  }
  catch(...)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
//#endif


  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
