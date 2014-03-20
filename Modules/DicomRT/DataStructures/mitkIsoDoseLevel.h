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


#ifndef _MITK_DOSE_ISO_LEVEL_H_
#define _MITK_DOSE_ISO_LEVEL_H_

#include <itkRGBPixel.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

#include "mitkCommon.h"
#include "mitkDoseValueType.h"
#include "MitkDicomRTExports.h"

namespace mitk
{

  /**
  \brief Stores values needed for the representation/visualization of dose iso levels.
  *
  * The dose iso level is defined in the topology of a dose distribution by the dose value
  * that indicates the lower boundary of the iso level. The upper boundary is implicitly defined
  * by the next IsoDoseLevel greater (higher dose value) than the current iso level.
  * Color and the visibility options are used to indicate the visualization style.
  */
  class MitkDicomRT_EXPORT IsoDoseLevel: public itk::Object
  {
  public:
    typedef ::itk::RGBPixel<float> ColorType;
    typedef DoseValueRel DoseValueType;

    mitkClassMacro(IsoDoseLevel, itk::Object);
    itkNewMacro(Self);
    mitkNewMacro4Param(Self,DoseValueType, ColorType, bool, bool);

    /** Checks if current dose iso level instances is greater
     according to the dose values.*/
    bool operator> ( const IsoDoseLevel& right ) const;
    /** Checks if current dose iso level instances is lesser
     according to the dose values.*/
    bool operator< ( const IsoDoseLevel& right ) const;

    bool operator == ( const IsoDoseLevel& right) const;

    itkSetMacro(DoseValue,DoseValueType);
    itkGetConstMacro(DoseValue,DoseValueType);

    itkSetMacro(Color,ColorType);
    itkGetConstMacro(Color,ColorType);

    itkSetMacro(VisibleIsoLine,bool);
    itkGetConstMacro(VisibleIsoLine,bool);
    itkBooleanMacro(VisibleIsoLine);

    itkSetMacro(VisibleColorWash,bool);
    itkGetConstMacro(VisibleColorWash,bool);
    itkBooleanMacro(VisibleColorWash);

  protected:
    IsoDoseLevel();
    IsoDoseLevel(const IsoDoseLevel & other);
    IsoDoseLevel(const DoseValueType & value, const ColorType& color, bool visibleIsoLine = true, bool visibleColorWash = true );
    virtual ~IsoDoseLevel();

    mitkCloneMacro(IsoDoseLevel);

    void PrintSelf(std::ostream &os, itk::Indent indent) const;

  private:
    /** Relative dose value and lower boundary of the iso level instance
     */
    DoseValueType m_DoseValue;

    /** RGB color code that should be used for the iso level.*/
    ColorType m_Color;

    /** indicates if an iso line should be shown for the iso level
     (the lower boundary indicated by m_DoseValue)*/
    bool m_VisibleIsoLine;

    /** indicates if a color wash should be shown for the iso level.*/
    bool m_VisibleColorWash;

    /** Not implemented on purpose*/
    IsoDoseLevel& operator = (const IsoDoseLevel& source);
  };

} // namespace mitk

#endif //_MITK_DOSE_ISO_LEVEL_H_
