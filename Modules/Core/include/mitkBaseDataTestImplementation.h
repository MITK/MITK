/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseDataTestImplementation_h
#define mitkBaseDataTestImplementation_h

#include <mitkBaseData.h>

namespace mitk
{
  /**
   * \brief Implementation of BaseData (for testing).
   *
   * As BaseData is an abstract class, we need an implementation for testing its methods.
   *
   * \ingroup Data
   */
  class BaseDataTestImplementation : public BaseData
  {
  public:
    mitkClassMacro(BaseDataTestImplementation, BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Initialize the time geometry with the given number of time steps.
     * \param timeSteps Number of time steps to initialize.
     */
    void InitializeTimeGeometry(unsigned int timeSteps /* = 1 */) override
    {
      Superclass::InitializeTimeGeometry(timeSteps);
    }

  protected:
    mitkCloneMacro(Self);

    /** \brief Copy constructor. */
    BaseDataTestImplementation(const BaseDataTestImplementation& other) : BaseData(other) {};

    /** \brief Verify the requested region. \return Always returns false. */
    bool VerifyRequestedRegion() override { return false; };
    /** \brief Check if the requested region is outside of the buffered region. \return Always returns false. */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return false; };
    /** \brief Set the requested region to the largest possible region. */
    void SetRequestedRegionToLargestPossibleRegion() override{};
    /** \brief Set the requested region from the given data object (no-op for testing). */
    void SetRequestedRegion(const itk::DataObject * /*data*/) override{};

    /** \brief Default constructor. */
    BaseDataTestImplementation(){};
    /** \brief Destructor. */
    ~BaseDataTestImplementation() override{};
  };

} // namespace

#endif
