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


#ifndef MITKDISPLAYCOORDINATEOPERATION_H_HEADER_INCLUDED_C10E33D0
#define MITKDISPLAYCOORDINATEOPERATION_H_HEADER_INCLUDED_C10E33D0

#include <MitkCoreExports.h>
#include "mitkBaseRenderer.h"
#include "mitkNumericTypes.h"
#include "mitkOperation.h"
#include <mitkWeakPointer.h>

#define mitkGetMacro(name,type) \
  virtual type Get##name () \
  { \
  return this->m_##name; \
  }


namespace mitk {
// TODO Legacy , no longer necessary when after migrating all DisplayInteractions to new Interactions.
// Coordinate supplier can probably also be removed then.

  //##Documentation
  //## @brief Operation with informations necessary for operations of DisplayVectorInteractor
  //## @ingroup Undo
  class MITK_CORE_EXPORT DisplayCoordinateOperation : public Operation
  {
  public:
    DisplayCoordinateOperation(mitk::OperationType operationType,
      mitk::BaseRenderer* renderer,
      const mitk::Point2D& startDisplayCoordinate,
      const mitk::Point2D& lastDisplayCoordinate,
      const mitk::Point2D& currentDisplayCoordinate
      );

DisplayCoordinateOperation(mitk::OperationType operationType,
      mitk::BaseRenderer* renderer,
      const mitk::Point2D& startDisplayCoordinate,
      const mitk::Point2D& lastDisplayCoordinate,
      const mitk::Point2D& currentDisplayCoordinate,
      const mitk::Point2D& startCoordinateInMM
      );

    virtual ~DisplayCoordinateOperation();

    mitk::BaseRenderer* GetRenderer();

    mitkGetMacro(StartDisplayCoordinate, mitk::Point2D);
    mitkGetMacro(LastDisplayCoordinate, mitk::Point2D);
    mitkGetMacro(CurrentDisplayCoordinate, mitk::Point2D);
    mitkGetMacro(StartCoordinateInMM, mitk::Point2D);

    mitk::Vector2D GetLastToCurrentDisplayVector();
    mitk::Vector2D GetStartToCurrentDisplayVector();
    mitk::Vector2D GetStartToLastDisplayVector();


  private:
    mitk::WeakPointer< mitk::BaseRenderer > m_Renderer;

    const mitk::Point2D m_StartDisplayCoordinate;
    const mitk::Point2D m_LastDisplayCoordinate;
    const mitk::Point2D m_CurrentDisplayCoordinate;
    const mitk::Point2D m_StartCoordinateInMM;
  };

}

#endif /* MITKDISPLAYCOORDINATEOPERATION_H_HEADER_INCLUDED_C10E33D0 */
