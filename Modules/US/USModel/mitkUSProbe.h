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


#ifndef MITKUSProbe_H_HEADER_INCLUDED_
#define MITKUSProbe_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk {

    /**Documentation
    * \brief Right now, the US Probe is only a fancy name for a string. Later, it could handle probe specific parameters
    *  like the current frequency etc. It is able to compare itself to other probes for device managment though.
    *
    * \ingroup US
    */
    //Be sure to check the isEqualTo() method if you expand this class to see if it needs work!
  class MitkUS_EXPORT USProbe : public itk::Object
    {
    public:
      mitkClassMacro(USProbe,itk::Object);
      itkNewMacro(Self);

      /**
      * \brief Compares this probe to another probe and returns true if they are equal in terms of name AND NAME ONLY
      *  be sure to sufficiently extend this method along with further capabilities probes.
      */
      bool IsEqualToProbe(mitk::USProbe::Pointer probe);


      //## getter and setter ##

      itkGetMacro(Name, std::string);
      itkSetMacro(Name, std::string);

    protected:
      USProbe();
      virtual ~USProbe();

      std::string m_Name;



    };
} // namespace mitk
#endif
