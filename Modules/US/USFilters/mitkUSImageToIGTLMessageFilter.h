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

#ifndef _MITKIMAGETOIGTLMessageFILTER_H__
#define _MITKIMAGETOIGTLMessageFILTER_H__

#include <mitkCommon.h>
#include <MitkUSExports.h>
#include <mitkIGTLMessageSource.h>
#include <mitkUSImage.h>
#include <mitkUSImageSource.h>

namespace mitk
{
/**Documentation
 *
 * \brief This filter creates IGTL messages from mitk::USImage objects
 *
 * \ingroup US
 *
 */
class MITKUS_EXPORT USImageToIGTLMessageFilter : public IGTLMessageSource
{
 public:
  mitkClassMacro(USImageToIGTLMessageFilter, IGTLMessageSource);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  /**
   * \brief filter execute method
   */
  virtual void GenerateData();

  using Superclass::SetInput;

  /**
   * \brief Sets one input Image
   */
  virtual void SetInput(const mitk::USImage* im);

  /**
   * Sets the input Image at a specific index
   */
  virtual void SetInput(unsigned int idx, const mitk::USImage* im);

  /**
   * \brief Returns the first input of this filter
   */
  const mitk::USImage* GetInput(void);

  /**
   * \brief Returns the input number idx of this filter
   */
  const mitk::USImage* GetInput(unsigned int idx);

  /**
   * empty implementation to prevent calling of the superclass method
   */
  void GenerateOutputInformation(){};

  /**
   * \brief Connects the input of this filter to the outputs of the given
   *NavigationDataSource
   *
   * Thes method does not support smartpointer. use FilterX.GetPointer() to
   *retrieve a dumbpointer.
   */
  virtual void ConnectTo(mitk::USImageSource* upstream);

 protected:
  USImageToIGTLMessageFilter();

  virtual ~USImageToIGTLMessageFilter();

  /**
  * \brief create output objects for all inputs
  */
  virtual void CreateOutputsForAllInputs();

  unsigned int m_CurrentTimeStep;  ///< Indicates the current timestamp
};
}  // namespace mitk

#endif  // _MITKIMAGETOIGTLMessageFILTER_H__
