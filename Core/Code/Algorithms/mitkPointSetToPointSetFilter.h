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


#ifndef _mitkpointsettopointsetfilter_h
#define _mitkpointsettopointsetfilter_h

#include <MitkExports.h>
#include "mitkPointSetSource.h"

namespace mitk
{
  /**
  * @brief Superclass of all classes/algorithms having one or more PointSets
  * as input and output
  * @ingroup Process
  */
  class MITK_CORE_EXPORT PointSetToPointSetFilter : public PointSetSource
  {
  public:
    mitkClassMacro( PointSetToPointSetFilter, PointSetSource );

    itkNewMacro( Self );

    typedef mitk::PointSet InputType;

    typedef mitk::PointSet OutputType;

    typedef InputType::Pointer InputTypePointer;

    typedef InputType::ConstPointer InputTypeConstPointer;

    /**
    * Sets the input of this process object
    * @param input the input
    */
    virtual void SetInput( const InputType* input );

    /**
    * Sets the input n'th of this process object
    * @param idx the number associated with the given input
    */
    virtual void SetInput( const unsigned int& idx, const InputType* input );

    /**
    * @returns the input tree of the process object
    */
    const InputType * GetInput( void );

    /**
    * @param idx the index of the input to return
    * @returns the input object with the given index
    */
    const InputType * GetInput( const unsigned int& idx );

  protected:

    /**
    * A default constructor
    */
    PointSetToPointSetFilter();

    /**
    * The destructor
    */
    virtual ~PointSetToPointSetFilter();

  private:

    void operator=( const Self& ); //purposely not implemented
  }
  ;

} //end of namespace mitk

#endif


