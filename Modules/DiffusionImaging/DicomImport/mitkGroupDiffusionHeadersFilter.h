/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-03-05 12:05:09 +0100 (Mo, 05 Mrz 2007) $
Version:   $Revision: 9583 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _mitkGroupDiffusionHeadersFilter_h
#define _mitkGroupDiffusionHeadersFilter_h

#include "MitkDiffusionImagingExports.h"

#include "mitkBaseProcess.h"
#include "mitkDiffusionVolumeHeaderInformation.h"

namespace mitk
{
  /**
  * @brief Groups Headers containing Diffusion Information
  * @ingroup Process
  */
  class MitkDiffusionImaging_EXPORT GroupDiffusionHeadersFilter : public BaseProcess
  {
  public:
    mitkClassMacro( GroupDiffusionHeadersFilter, BaseProcess );

    itkNewMacro( Self );

    typedef mitk::DiffusionVolumeHeaderInformation HeaderType;
    typedef HeaderType::Pointer HeaderPointer;
    typedef std::vector<HeaderPointer> InputType;

    typedef std::vector<InputType> OutputType;

    /**
    * Sets the input of this process object 
    * @param input the input
    */
    virtual void SetInput( InputType input );

    /**
    * Sets the input n'th of this process object 
    * @param idx the number associated with the given input
    */
    virtual void SetInput( const unsigned int& idx, InputType input );

    /**
    * Sets the input n'th of this process object 
    * @param idx the number associated with the given input
    */
    virtual void SetNthOutput( const unsigned int& idx, InputType output );

    /**
    * @returns the input tree of the process object  
    */
    InputType GetInput( void );

    /**
    * @param idx the index of the input to return
    * @returns the input object with the given index  
    */
    InputType GetInput( const unsigned int& idx );

    OutputType GetOutput();

    virtual void GenerateOutputInformation();

    virtual void Update();

  protected:

    /**
    * A default constructor  
    */
    GroupDiffusionHeadersFilter();

    /**
    * The destructor  
    */
    virtual ~GroupDiffusionHeadersFilter();

    OutputType m_Output;
    InputType m_Input;

  private:

    void operator=( const Self& ); //purposely not implemented
  }
  ;

} //end of namespace mitk

#endif


