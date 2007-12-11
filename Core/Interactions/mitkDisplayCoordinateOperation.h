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


#ifndef MITKDISPLAYCOORDINATEOPERATION_H_HEADER_INCLUDED_C10E33D0
#define MITKDISPLAYCOORDINATEOPERATION_H_HEADER_INCLUDED_C10E33D0

#include "mitkCommon.h"
#include "mitkBaseRenderer.h"
#include "mitkVector.h"
#include "mitkOperation.h"

#define mitkGetMacro(name,type) \
  virtual type Get##name () \
  { \
  return this->m_##name; \
  }


namespace mitk {

  //##ModelId=3EF1CC2B03BD
  //##Documentation
  //## @brief Operation with informations necessary for operations of DisplayVectorInteractor
  //## @ingroup Undo
  class DisplayCoordinateOperation : public Operation
  {
  public:
    //##ModelId=3EF1D349027C
    DisplayCoordinateOperation(mitk::OperationType operationType, 
      mitk::BaseRenderer* renderer,
      const mitk::Point2D& startDisplayCoordinate, 
      const mitk::Point2D& lastDisplayCoordinate, 
      const mitk::Point2D& currentDisplayCoordinate
      );

    //##ModelId=3EF1CCBB03B0
    virtual ~DisplayCoordinateOperation();

    //##ModelId=3EF1D5CF0235
    mitk::BaseRenderer* GetRenderer();

    //##ModelId=3F0177090269
    mitkGetMacro(StartDisplayCoordinate, mitk::Point2D);
    //##ModelId=3F0177090288
    mitkGetMacro(LastDisplayCoordinate, mitk::Point2D);
    //##ModelId=3F01770902A7
    mitkGetMacro(CurrentDisplayCoordinate, mitk::Point2D);

    //##ModelId=3F01770902D6
    mitk::Vector2D GetLastToCurrentDisplayVector();
    //##ModelId=3F01770902D7
    mitk::Vector2D GetStartToCurrentDisplayVector();
    //##ModelId=3F01770902E6
    mitk::Vector2D GetStartToLastDisplayVector();


  private:
    //##ModelId=3EF1D2A00053
    mitk::BaseRenderer::Pointer m_Renderer;

    //##ModelId=3EF1CD2D03A0
    const mitk::Point2D m_StartDisplayCoordinate;
    //##ModelId=3F01770901DC
    const mitk::Point2D m_LastDisplayCoordinate;
    //##ModelId=3F01770901FB
    const mitk::Point2D m_CurrentDisplayCoordinate;
  };

}

#endif /* MITKDISPLAYCOORDINATEOPERATION_H_HEADER_INCLUDED_C10E33D0 */
