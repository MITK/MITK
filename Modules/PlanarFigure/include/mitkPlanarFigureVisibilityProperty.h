/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureVisibilityProperty_h
#define mitkPlanarFigureVisibilityProperty_h

#include <mitkEnumerationProperty.h>

#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief When a part of a planar figure is shown, e.g. its helper lines or
   *        its label.
   *
   * The values are "Always", "On hover", "When selected", "On hover or when
   * selected", "On hover while selected" and "Never". All but "Always" and
   * "Never" also show the part while the figure is being placed.
   *
   * \ingroup MitkPlanarFigureModule
   */
  class MITKPLANARFIGURE_EXPORT PlanarFigureVisibilityProperty : public EnumerationProperty
  {
  public:
    enum class Visibility : IdType
    {
      Always,
      OnHover,
      WhenSelected,
      OnHoverOrWhenSelected,
      OnHoverWhileSelected,
      Never
    };

    mitkClassMacro(PlanarFigureVisibilityProperty, EnumerationProperty);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    mitkNewMacro1Param(PlanarFigureVisibilityProperty, Visibility);

    Visibility GetVisibility() const;
    void SetVisibility(Visibility visibility);

    using BaseProperty::operator=;

  protected:
    PlanarFigureVisibilityProperty();
    explicit PlanarFigureVisibilityProperty(Visibility visibility);
    PlanarFigureVisibilityProperty(const PlanarFigureVisibilityProperty &other) = default;

    /** \brief Overridden as protected, since the values are fixed. */
    bool AddEnum(const std::string &name, const IdType &id) override;

    mitkCloneMacro(PlanarFigureVisibilityProperty);
  };
}

#endif
