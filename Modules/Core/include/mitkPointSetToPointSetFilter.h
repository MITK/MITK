/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mitkpointsettopointsetfilter_h
#define _mitkpointsettopointsetfilter_h

#include "mitkPointSetSource.h"
#include <MitkCoreExports.h>

namespace mitk
{
  /**
  * @brief Superclass of all classes/algorithms having one or more PointSets
  * as input and output
  * @ingroup Process
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
    * Sets the input of this process object
    * @param input the input
    */
    virtual void SetInput(const InputType *input);

    /**
    * Sets the input n'th of this process object
    * @param idx the number associated with the given input
    * @param input
    */
    virtual void SetInput(const unsigned int &idx, const InputType *input);

    /**
    * @returns the input tree of the process object
    */
    const InputType *GetInput(void);

    /**
    * @param idx the index of the input to return
    * @returns the input object with the given index
    */
    const InputType *GetInput(const unsigned int &idx);

  protected:
    /**
    * A default constructor
    */
    PointSetToPointSetFilter();

    /**
    * The destructor
    */
    ~PointSetToPointSetFilter() override;

  private:
    void operator=(const Self &); // purposely not implemented
  };

} // end of namespace mitk

#endif
