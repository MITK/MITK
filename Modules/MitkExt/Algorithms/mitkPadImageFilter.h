/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKPADIMAGEFILTER_H_HEADER_INCLUDED
#define MITKPADIMAGEFILTER_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"
#include "mitkImageTimeSelector.h"

#include "itkImage.h"


namespace mitk {

/**
 * \brief PadImageFilter class pads the first input image to the size of the second input image.
 * Two Images have to be set.
 * The first image is the image to pad. The second image defines the pad size.
 * It is also possible to use an included binary filter.
 *
 * \ingroup Process
 */
class MITKEXT_CORE_EXPORT PadImageFilter : public ImageToImageFilter
{
public:    
  mitkClassMacro(PadImageFilter, ImageToImageFilter);

  itkNewMacro(Self);  

  /** \brief Sets the intensity of the pixel to pad */
  itkSetMacro(PadConstant, int);

  /** \brief sets the binary filter ON or OFF */
  itkSetMacro(BinaryFilter, bool);

  /** \brief Sets the lower threshold of the included binary filter */
  itkSetMacro(LowerThreshold, int);

  /** \brief Sets the upper threshold of the included binary filter */
  itkSetMacro(UpperThreshold, int);

protected:
  PadImageFilter();
  
  virtual ~PadImageFilter();
  
  virtual void GenerateData();

private:
  bool m_BinaryFilter;
  int  m_PadConstant, m_LowerThreshold, m_UpperThreshold;

};

} // namespace mitk

#endif /* MITKPADIMAGEFILTER_H_HEADER_INCLUDED */
