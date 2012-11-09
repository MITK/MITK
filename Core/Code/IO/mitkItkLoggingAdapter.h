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


#ifndef ItkLoggingAdapter_H_HEADER_INCLUDED
#define ItkLoggingAdapter_H_HEADER_INCLUDED

#include <MitkExports.h>
#include <itkOutputWindow.h>
#include <itkObjectFactory.h>

namespace mitk {
//##Documentation
//## @brief Adapter that overwrites the standard itk logging output window and sends the logging messages to the MITK logging instead.
//## @ingroup IO

// this class is used to send output to stdout and not the itk window
class MITK_CORE_EXPORT ItkLoggingAdapter : public itk::OutputWindow
{
public:
  typedef ItkLoggingAdapter                Self;
  typedef itk::SmartPointer<Self>                Pointer;
  typedef itk::SmartPointer<const Self>          ConstPointer;

  /** Run-time type information (and related methods).   */
  itkTypeMacro( ItkLoggingAdapter, itk::OutputWindow );

  /** New macro for creation of through a Smart Pointer   */
  itkNewMacro(ItkLoggingAdapter);

  /** @brief Initializes the logging adapter. Itk logging
    *        messages are redirected to MITK logging afterwards.
    */
  static void Initialize();

  virtual void DisplayText(const char* s);

protected:
  ItkLoggingAdapter();
  virtual ~ItkLoggingAdapter();

private:
  ItkLoggingAdapter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};


}

#endif /* mitkItkLoggingAdapter_H_HEADER_INCLUDED */
