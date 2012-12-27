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

#ifndef _MITK_CONTOURMODEL_SOURCE_H
#define _MITK_CONTOURMODEL_SOURCE_H

#include "SegmentationExports.h"
#include "mitkBaseProcess.h"
#include "mitkContourModel.h"

namespace mitk
{
  /**
  * @brief Superclass of all classes generating ContourModels.
  * @ingroup Process
  */
  class Segmentation_EXPORT ContourModelSource : public BaseProcess
  {
  public:
    mitkClassMacro( ContourModelSource, BaseProcess );

    itkNewMacro( Self );

    typedef ContourModel OutputType;

    typedef OutputType::Pointer OutputTypePointer;

    /**
    * Allocates a new output object and returns it. Currently the
    * index idx is not evaluated.
    * @param idx the index of the output for which an object should be created
    * @returns the new object
    */
    virtual itk::DataObject::Pointer MakeOutput ( unsigned int idx );

    /**
    * Allows to set the output of the point set source.
    * @param output the intended output of the point set source
    */
    virtual void SetOutput( OutputType* output );

    virtual void GraftOutput(OutputType *output);
    virtual void GraftNthOutput(unsigned int idx, OutputType *output);

    /**
    * Returns the output with index 0 of the point set source
    * @returns the output
    */
    virtual OutputType* GetOutput();

    /**
    * Returns the n'th output of the point set source
    * @param idx the index of the wanted output
    * @returns the output with index idx.
    */
    virtual OutputType* GetOutput ( unsigned int idx );

  protected:

    ContourModelSource();

    virtual ~ContourModelSource();

  };

}
#endif // #_MITK_CONTOURMODEL_SOURCE_H
