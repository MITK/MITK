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

#ifndef mitkNamedContourModelSet_h
#define mitkNamedContourModelSet_h

#include <mitkDicomRTExports.h>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>

namespace mitk
{

  class mitkDicomRT_EXPORT NamedContourModelSet: public itk::Object
  {

  public:

    mitkClassMacro( NamedContourModelSet, itk::Object );

    itkNewMacro( Self );

    /**
    * Virtual destructor.
    */
    virtual ~NamedContourModelSet();

  protected:

    /**
    * Constructor.
    */
    NamedContourModelSet();

  };

}

#endif
