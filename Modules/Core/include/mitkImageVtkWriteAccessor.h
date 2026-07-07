/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageVtkWriteAccessor_h
#define mitkImageVtkWriteAccessor_h

#include <mitkImageAccessorBase.h>

class vtkImageData;

namespace mitk
{
  class Image;
  class ImageDataItem;

  /**
   * \brief Provides write access to image data in the form required by VTK methods.
   *
   * This class locks the image data for writing and provides access to the
   * underlying vtkImageData representation.
   *
   * \ingroup Data
   */
  class MITKCORE_EXPORT ImageVtkWriteAccessor : public ImageAccessorBase
  {
  public:
    typedef itk::SmartPointer<Image> ImagePointer;

    /** \brief Creates an ImageVtkWriteAccessor for a whole Image.
     *  \param iP The associated Image (smart pointer).
     *  \param iDI The image data item describing the allocated image part.
     *  \param imageDataVtk The VTK image data representation to provide access to.
     */
    ImageVtkWriteAccessor(ImagePointer iP, const ImageDataItem *iDI, vtkImageData *imageDataVtk);

    /** \brief Destructor unregisters this accessor from the image. */
    ~ImageVtkWriteAccessor() override;

    /** \brief Returns the VTK image data for write access. */
    vtkImageData *GetVtkImageData() const;

  protected:
    /** \brief Returns a const pointer to the associated Image. */
    const Image *GetImage() const override;

  private:
    // due to smart pointer issues, the image is only kept as a weak pointer.
    Image *m_Image;
    vtkImageData *m_ImageDataVtk;
  };
}

#endif
