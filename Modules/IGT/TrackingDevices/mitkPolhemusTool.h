/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPolhemusTool_h
#define mitkPolhemusTool_h

#include <mitkPolhemusInterface.h>
#include <mitkTrackingTool.h>

namespace mitk
{
  class PolhemusTrackingDevice;
  /** Documentation:
  *   \brief  An object of this class represents a tool of a Polhemus tracking device.
  *           A tool has to be added to a tracking device which will then
  *           continuously update the tool coordinates.
  *   \ingroup IGT
  */
  class MITKIGT_EXPORT PolhemusTool : public TrackingTool
  {
  public:
    friend class PolhemusTrackingDevice;
    mitkClassMacro(PolhemusTool, TrackingTool);

    enum DistortionLevel
    {
      UNDEFINED, ///< Distortion level is not determined.
      NO_DISTORTION, ///< System operational with a good quality magnetic signal.
      MINOR_DISTORTION, ///< System operational with a marginal magnetic signal.
      SIGNIFICANT_DISTORTION ///< System operational with a poor magnetic signal.
    };

    /**
    * \brief Sets the port of the tool. (e.g. 1 for port "SENS 1" etc.)
    */
    virtual void SetToolPort(int _ToolPort);

    /**
    * \brief Sets the port of the tool. (e.g. 1 for port "SENS 1" etc.)
    */
    virtual int GetToolPort();

    /**
     * \brief Sets the distortion level by mapping the integer value (read from the response frame) to the corresponding enumeration literal.
     *
     * According to the Polhemus Liberty documentation:
     * - 0 means system operational with a good quality magnetic signal. No distortion.
     * - 1 means system operational with a marginal magnetic signal. Minor distortion.
     * - 2 means system operational with a poor magnetic signal. Significant distortion.
     *
     * \param level The distortion level represented as 0, 1 or 2.
     */
    void SetDistortionLevel(const int level);

    /**
     * \brief Returns the distortion level.
     * \return The distortion level. UNDEFINED, if distortion level is not determined.
     */
    DistortionLevel GetDistortionLevel() const;

  protected:
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)
    PolhemusTool();
    virtual ~PolhemusTool();

    //This is the port, on which the tool is connected. It is identical with the "ToolIdentifier" set on NavigationDataTools.
    //If tool is connected on port "SENS 2", the m_ToolPort is 2 etc.
    int m_ToolPort;

    DistortionLevel m_DistortionLevel;
  };
}//mitk
#endif
