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

#ifndef MITK_NON_BLOCKING_ALGORHITHMS_ENVETS_H_INCLDUED
#define MITK_NON_BLOCKING_ALGORHITHMS_ENVETS_H_INCLDUED

#include <itkEventObject.h>

namespace mitk {

  class NonBlockingAlgorithm;

  class NonBlockingAlgorithmEvent : public itk::AnyEvent
  { 
  public: 
    typedef NonBlockingAlgorithmEvent Self; 
    typedef itk::AnyEvent Superclass; 

    NonBlockingAlgorithmEvent( const NonBlockingAlgorithm* algorithm = NULL )
      : m_Algorithm(algorithm) {} 

    virtual ~NonBlockingAlgorithmEvent() {} 

    virtual const char * GetEventName() const 
    { 
      return "NonBlockingAlgorithmEvent"; 
    } 

    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( m_Algorithm ); 
    } 

    const mitk::NonBlockingAlgorithm* GetAlgorithm() const 
    { 
      return m_Algorithm.GetPointer(); 
    }
    
    NonBlockingAlgorithmEvent(const Self& s) : itk::AnyEvent(s), m_Algorithm(s.m_Algorithm) {}; 

  protected:
    mitk::NonBlockingAlgorithm::ConstPointer m_Algorithm;
  
  private: 
    void operator=(const Self&); 

  };


  class ResultAvailable : public NonBlockingAlgorithmEvent
  {
    public:

      ResultAvailable( const NonBlockingAlgorithm* algorithm = NULL )
      : NonBlockingAlgorithmEvent(algorithm)
      {
      }

      virtual ~ResultAvailable()
      {
      }
  };

  class ProcessingError : public NonBlockingAlgorithmEvent
  {
    public:

      ProcessingError( const NonBlockingAlgorithm* algorithm = NULL )
      : NonBlockingAlgorithmEvent(algorithm)
      {
      }

      virtual ~ProcessingError()
      {
      }
  };



}

#endif

