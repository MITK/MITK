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

    NonBlockingAlgorithmEvent( const NonBlockingAlgorithm* algorithm = nullptr )
      : m_Algorithm(algorithm) {}

    virtual ~NonBlockingAlgorithmEvent() {}

    virtual const char * GetEventName() const override
    {
      return "NonBlockingAlgorithmEvent";
    }

    virtual bool CheckEvent(const ::itk::EventObject* e) const override
    {
      return dynamic_cast<const Self*>(e);
    }

    virtual ::itk::EventObject* MakeObject() const override
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

      ResultAvailable( const NonBlockingAlgorithm* algorithm = nullptr )
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

      ProcessingError( const NonBlockingAlgorithm* algorithm = nullptr )
      : NonBlockingAlgorithmEvent(algorithm)
      {
      }

      virtual ~ProcessingError()
      {
      }
  };



}

#endif
