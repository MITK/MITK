/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-08-27 16:07:48 +0200 (Mi, 27 Aug 2008) $
Version:   $Revision: 15092 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKTENSORIMAGE_H_HEADER_INCLUDED_C1C2FCD2
#define MITKTENSORIMAGE_H_HEADER_INCLUDED_C1C2FCD2

namespace mitk {

  class MITK_CORE_EXPORT TensorImage : public Image
{
public:
  mitkClassMacro(TensorImage, Image);    
  itkNewMacro(Self);

  //##Documentation
  //## @brief Get a volume at a specific time @a t of channel @a n as a vtkImageData.
  virtual vtkImageData* GetVtkImageData(int t = 0, int n = 0);


protected:

  TensorImage();

  virtual ~TensorImage();

  

};

} // namespace mitk

#endif /* MITKTENSORIMAGE_H_HEADER_INCLUDED_C1C2FCD2 */
