/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUSProbe_h
#define mitkUSProbe_h

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkVector.h>

namespace mitk {
  /**Documentation
  * \brief Right now, the US Probe is only a fancy name for a string. Later, it could handle probe specific parameters
  *  like the current frequency etc. It is able to compare itself to other probes for device managment though.
  *
  * \ingroup US
  */
  //Be sure to check the isEqualTo() method if you expand this class to see if it needs work!
  class MITKUS_EXPORT USProbe : public itk::Object
  {
  public:
    mitkClassMacroItkParent(USProbe, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)
      mitkNewMacro1Param(Self, std::string);

    /**
    * \brief Struct to define a probe specific ultrasound image cropping.
    */
    typedef struct USProbeCropping_
    {
      unsigned int top;
      unsigned int bottom;
      unsigned int left;
      unsigned int right;

      USProbeCropping_()
        : top(0), bottom(0), left(0), right(0) { };

      USProbeCropping_(unsigned int top, unsigned int bottom, unsigned int left, unsigned int right)
        : top(top), bottom(bottom), left(left), right(right) { };
    }USProbeCropping;

    /**
    * \brief Sets the probe cropping.
    */
    void SetProbeCropping(unsigned int top, unsigned int bottom, unsigned int left, unsigned int right);
    USProbeCropping GetProbeCropping();

    /**
    * \brief Compares this probe to another probe and returns true if they are equal in terms of name AND NAME ONLY
    *  be sure to sufficiently extend this method along with further capabilities probes.
    */
    bool IsEqualToProbe(mitk::USProbe::Pointer probe);

    /**
    * \brief Sets a scanning depth of the probe and the associated spacing
    */
    void SetDepthAndSpacing(int depth, Vector3D spacing);

    /**
    * \brief Gets all scanning depths and the associates spacings of the probe as an std::map with depth as key (represented by an int) and
    *spacing as value (represented by a Vector3D)
    */
    std::map<int, Vector3D> GetDepthsAndSpacing();

    /**
    * \brief Sets a scanning depth of the probe with the default spacing (1,1,1). Exact spacing needs to be calibrated.
    */
    void SetDepth(int depth);

    /**
    * \brief Removes the given depth of the probe, if it exists
    */
    void RemoveDepth(int depthToRemove);

    /**
    * \ brief Sets the spacing associated to the given depth of the probe. Spacing needs to be calibrated.
    */
    void SetSpacingForGivenDepth(int givenDepth, Vector3D spacing);

    /**
    * \brief Returns the spacing that is associated to the given depth of the probe.
    *If spacing was not calibrated or if depth does not exist for this probe the default spacing (1,1,1) is returned.
    */
    Vector3D GetSpacingForGivenDepth(int givenDepth);

    /**
    * \brief Checks, whether the std::map m_DepthAndSpacings contains at least one depth element or not.
    * \return True, if the the std::map m_DepthAndSpacings does not contain at least one depth element, else false.
    */
    bool IsDepthAndSpacingEmpty();

    //## getter and setter ##

    itkGetMacro(Name, std::string);
    itkSetMacro(Name, std::string);
    itkGetMacro(CurrentDepth, double);
    itkSetMacro(CurrentDepth, double);

  protected:
    USProbe();
    USProbe(std::string identifier);
    ~USProbe() override;

    std::string m_Name;
    double m_CurrentDepth;

    // Map containing the depths and the associated spacings as an std::vector with depth as key and spacing as value
    std::map<int, Vector3D> m_DepthsAndSpacings;

    USProbeCropping m_Cropping;
  };
} // namespace mitk
#endif
