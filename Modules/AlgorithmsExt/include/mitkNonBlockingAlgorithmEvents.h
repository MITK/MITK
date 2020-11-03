/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_NON_BLOCKING_ALGORHITHMS_ENVETS_H_INCLDUED
#define MITK_NON_BLOCKING_ALGORHITHMS_ENVETS_H_INCLDUED

#include <itkEventObject.h>

namespace mitk
{
  class NonBlockingAlgorithm;

  class NonBlockingAlgorithmEvent : public itk::AnyEvent
  {
  public:
    typedef NonBlockingAlgorithmEvent Self;
    typedef itk::AnyEvent Superclass;

    NonBlockingAlgorithmEvent(const NonBlockingAlgorithm *algorithm = nullptr) : m_Algorithm(algorithm) {}
    ~NonBlockingAlgorithmEvent() override {}
    const char *GetEventName() const override { return "NonBlockingAlgorithmEvent"; }
    bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }
    ::itk::EventObject *MakeObject() const override { return new Self(m_Algorithm); }
    const mitk::NonBlockingAlgorithm *GetAlgorithm() const { return m_Algorithm.GetPointer(); }
    NonBlockingAlgorithmEvent(const Self &s) : itk::AnyEvent(s), m_Algorithm(s.m_Algorithm){};

  protected:
    mitk::NonBlockingAlgorithm::ConstPointer m_Algorithm;

  private:
    void operator=(const Self &);
  };

  class ResultAvailable : public NonBlockingAlgorithmEvent
  {
  public:
    ResultAvailable(const NonBlockingAlgorithm *algorithm = nullptr) : NonBlockingAlgorithmEvent(algorithm) {}
    ~ResultAvailable() override {}
  };

  class ProcessingError : public NonBlockingAlgorithmEvent
  {
  public:
    ProcessingError(const NonBlockingAlgorithm *algorithm = nullptr) : NonBlockingAlgorithmEvent(algorithm) {}
    ~ProcessingError() override {}
  };
}

#endif
