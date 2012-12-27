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

#ifndef mitkVtkImageOverwrite_h_Included
#define mitkVtkImageOverwrite_h_Included

#include <vtkImageReslice.h>
#include "SegmentationExports.h"

  /** \brief A vtk Filter based on vtkImageReslice with the aditional feature to write a slice into the given input volume.
    All optimizations for e.g. the plane directions or interpolation are stripped away, the algorithm only interpolates nearest
    neighbor and uses the non optimized execute function of vtkImageReslice. Note that any interpolation doesn't make sense
  for round trip use extract->edit->overwrite, because it is nearly impossible to invert the interolation.
    There are two use cases for the Filter which are specified by the overwritemode property:

    1)Extract slices from a 3D volume.
      Overwritemode = false

      In this mode the class can be used like vtkImageReslice. The usual way to do this is:
      - Set an image volume as input
      - Set the ResliceAxes via SetResliceAxesDirectionCosines and SetResliceAxesOrigin
      - Set the the OutputSpacing, OutputOrigin and OutputExtent
      - Call Update


    2)Overwrite a 3D volume at a given slice.
      Overwritemode = true

      The handling in this mode is quite similar to the description above with the addition that the
      InputSlice needs to be specified via SetInputSlice(vtkImageData*).
      - Set the properties mentioned above (Note that SetInput specifies the volume to write to)
      - Set the slice to that has to be overwritten in the volume ( SetInputSlice(vtkImageData*)

      After calling Update() there is no need to retrieve the output as the input volume is modified.

      \sa vtkImageReslice
      (Note that the execute and interpolation functions are no members and thus can not be overriden)
   */
  class Segmentation_EXPORT mitkVtkImageOverwrite : public vtkImageReslice
  {
  public:
    static mitkVtkImageOverwrite *New();
    vtkTypeMacro(mitkVtkImageOverwrite, vtkImageReslice);

    /** \brief Set the mode either to reslice (false) or to overwrite (true).
        Default: false
     */
    void SetOverwriteMode(bool b);
    bool IsOverwriteMode(){return m_Overwrite_Mode;}

    /** \brief Set the slice for overwrite mode.
      Note:
      It is recommend not to use this in reslice mode because otherwise the slice will be modified!
      */
    void SetInputSlice(vtkImageData* slice);


  protected:

    mitkVtkImageOverwrite();
    virtual ~mitkVtkImageOverwrite();

    bool m_Overwrite_Mode;

    /** Overridden from vtkImageReslice. \sa vtkImageReslice::ThreadedRequestData */
    virtual void ThreadedRequestData(vtkInformation *vtkNotUsed(request),
                                      vtkInformationVector **vtkNotUsed(inputVector),
                                      vtkInformationVector *vtkNotUsed(outputVector),
                                      vtkImageData ***inData,
                                      vtkImageData **outData,
                                      int outExt[6], int id);
  };


#endif //mitkVtkImageOverwrite_h_Included
