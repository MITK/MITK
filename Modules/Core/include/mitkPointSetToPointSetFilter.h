/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetToPointSetFilter_h
#define mitkPointSetToPointSetFilter_h

#include <mitkPointSetSource.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
  * \brief Superclass of all classes/algorithms having one or more PointSets
  * as input and output.
  *
  * \sa PointSetSource
  * \sa PointSet
  * \ingroup Process
  */
  class MITKCORE_EXPORT PointSetToPointSetFilter : public PointSetSource
  {
  public:
    mitkClassMacro(PointSetToPointSetFilter, PointSetSource);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      typedef mitk::PointSet InputType;

    typedef mitk::PointSet OutputType;

    typedef InputType::Pointer InputTypePointer;

    typedef InputType::ConstPointer InputTypeConstPointer;

    using itk::ProcessObject::SetInput;
    /**
    * \brief Sets the input of this process object.
    *
    * \param input the PointSet to use as the primary input.
    */
    virtual void SetInput(const InputType *input);

    /**
    * \brief Sets the n-th input of this process object.
    *
    * \param idx the index associated with the given input.
    * \param input the PointSet to use as the input at position \p idx.
    */
    virtual void SetInput(const unsigned int &idx, const InputType *input);

    /**
    * \brief Returns the primary input of the process object.
    *
    * \return The input PointSet, or nullptr if no input is set.
    */
    const InputType *GetInput(void);

    /**
    * \brief Returns the input at the given index.
    *
    * \param idx the index of the input to return.
    * \return The input PointSet at position \p idx, or nullptr if the index is out of range.
    */
    const InputType *GetInput(const unsigned int &idx);

  protected:
    /** \brief Default constructor. Sets the number of required inputs to 1. */
    PointSetToPointSetFilter();

    /** \brief Destructor. */
    ~PointSetToPointSetFilter() override;

  private:
    void operator=(const Self &); // purposely not implemented
  };

} // end of namespace mitk

#endif
