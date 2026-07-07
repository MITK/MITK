/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNonBlockingAlgorithmEvents_h
#define mitkNonBlockingAlgorithmEvents_h

#include <itkEventObject.h>

namespace mitk
{
  class NonBlockingAlgorithm;

  /**
   * \brief Base event class for NonBlockingAlgorithm notifications.
   *
   * This event is emitted by NonBlockingAlgorithm to notify observers about
   * state changes. It carries a const pointer to the algorithm that raised the event.
   *
   * \sa NonBlockingAlgorithm
   * \sa ResultAvailable
   * \sa ProcessingError
   */
  class NonBlockingAlgorithmEvent : public itk::AnyEvent
  {
  public:
    typedef NonBlockingAlgorithmEvent Self;
    typedef itk::AnyEvent Superclass;

    /**
     * \brief Construct the event, optionally referencing the originating algorithm.
     * \param[in] algorithm Pointer to the algorithm that raised the event, or nullptr.
     */
    NonBlockingAlgorithmEvent(const NonBlockingAlgorithm *algorithm = nullptr) : m_Algorithm(algorithm) {}
    ~NonBlockingAlgorithmEvent() override {}

    /** \brief Return the human-readable name of this event. */
    const char *GetEventName() const override { return "NonBlockingAlgorithmEvent"; }

    /**
     * \brief Check whether a given event object is of this type.
     * \param[in] e The event object to check.
     * \return True if \p e is a NonBlockingAlgorithmEvent.
     */
    bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }

    /**
     * \brief Create a copy of this event on the heap.
     * \return A new NonBlockingAlgorithmEvent.
     */
    ::itk::EventObject *MakeObject() const override { return new Self(m_Algorithm); }

    /**
     * \brief Get the algorithm that raised this event.
     * \return Const pointer to the originating NonBlockingAlgorithm, or nullptr.
     */
    const mitk::NonBlockingAlgorithm *GetAlgorithm() const { return m_Algorithm.GetPointer(); }

    /** \brief Copy constructor. */
    NonBlockingAlgorithmEvent(const Self &s) : itk::AnyEvent(s), m_Algorithm(s.m_Algorithm){};

  protected:
    mitk::NonBlockingAlgorithm::ConstPointer m_Algorithm; ///< The algorithm that raised this event.

  private:
    void operator=(const Self &);
  };

  /**
   * \brief Event indicating that a NonBlockingAlgorithm has produced a result.
   *
   * Emitted when ThreadedUpdateFunction() returns successfully.
   *
   * \sa NonBlockingAlgorithm
   * \sa NonBlockingAlgorithmEvent
   */
  class ResultAvailable : public NonBlockingAlgorithmEvent
  {
  public:
    /**
     * \brief Construct the event.
     * \param[in] algorithm Pointer to the algorithm that produced the result.
     */
    ResultAvailable(const NonBlockingAlgorithm *algorithm = nullptr) : NonBlockingAlgorithmEvent(algorithm) {}
    ~ResultAvailable() override {}
  };

  /**
   * \brief Event indicating that a NonBlockingAlgorithm has encountered an error.
   *
   * Emitted when ThreadedUpdateFunction() returns false.
   *
   * \sa NonBlockingAlgorithm
   * \sa NonBlockingAlgorithmEvent
   */
  class ProcessingError : public NonBlockingAlgorithmEvent
  {
  public:
    /**
     * \brief Construct the event.
     * \param[in] algorithm Pointer to the algorithm that encountered the error.
     */
    ProcessingError(const NonBlockingAlgorithm *algorithm = nullptr) : NonBlockingAlgorithmEvent(algorithm) {}
    ~ProcessingError() override {}
  };
}

#endif
