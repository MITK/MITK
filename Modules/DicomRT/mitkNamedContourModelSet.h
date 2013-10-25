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

#include <mitkContourModelSet.h>

namespace mitk
{

  class mitkDicomRT_EXPORT NamedContourModelSet: public mitk::ContourModelSet
  {

  public:

    char* roiName;
    mitk::ContourModelSet::Pointer contourModelSet;

    mitkClassMacro( NamedContourModelSet, itk::Object )
    itkNewMacro( Self )

    void SetRoiName(char*);
    void SetContourModelSet(mitk::ContourModelSet::Pointer);

    char* GetRoiName();
    mitk::ContourModelSet::Pointer GetContourModelSet();


    virtual ~NamedContourModelSet();

  protected:

    NamedContourModelSet();

  };

}

#endif
