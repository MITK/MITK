/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITK_WIIMOTEACTIVATOR_H_
#define MITK_WIIMOTEACTIVATOR_H_

#include "mitkIInputDevice.h"

namespace mitk
{
	class WiiMoteActivator : public IInputDevice
	{
		public:

			WiiMoteActivator();

			bool RegisterInputDevice();
			bool UnRegisterInputDevice();

		protected:

		private:
	}; // end class WiiMoteActivator
} // end namespace mitk

#endif /* MITK_WIIMOTEACTIVATOR_H_ */
