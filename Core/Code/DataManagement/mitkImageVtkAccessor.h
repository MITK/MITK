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

#ifndef MITKIMAGEVTKACCESSOR_H
#define MITKIMAGEVTKACCESSOR_H

#include <algorithm>
#include <itkIndex.h>
#include <itkPoint.h>
#include <mitkCommon.h>
#include <itkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include "mitkImageAccessorBase.h"
#include "mitkImageDataItem.h"
#include "mitkPixelType.h"

namespace mitk {

class Image;
typedef itk::SmartPointer<mitk::Image> ImagePointer;

//##Documentation
//## @brief ImageVtkAccessor class provides any image read access which is required by Vtk methods
//## @ingroup Data
  class ImageVtkAccessor : public ImageAccessorBase, public vtkImageData
  {
    friend class Image;

  protected:
    /** \brief Creates an ImageVtkAccessor for a whole Image
     *  \param Image::Pointer specifies the associated Image
     */
    ImageVtkAccessor(
      ImagePointer iP
    );

  public:

  static ImageVtkAccessor *New(ImagePointer); /* {
    vtkObject* object = vtkObjectFactory::CreateInstance("ImageVtkAccessor");
    if(object)
    {
      return static_cast<ImageVtkAccessor*>(object);
    }
    else return new ImageVtkAccessor;
  }*/

  //vtkTypeMacro(ImageVtkAccessor,vtkDataSet)

    /** \brief Gives const access to the data. */
    inline const void * GetData()
    {
      return m_AddressBegin;
    }


    /** Destructor */
    virtual ~ImageVtkAccessor()
    {
    }

  protected:

    // protected members


  private:

  };


}

#endif // MITKIMAGEVTKACCESSOR_H
