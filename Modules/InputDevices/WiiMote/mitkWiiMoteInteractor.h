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
#ifndef MITK_WIIMOTEINTERACTOR_H
#define MITK_WIIMOTEINTERACTOR_H

// mitk
#include <mitkInteractor.h>
#include <mitkDataNode.h>
#include <mitkAction.h>
#include <mitkStateEvent.h>
#include <mitkGeometry3D.h>
#include <mitkWiiMoteAllDataEvent.h>
#include <mitkBaseRenderer.h>

// export macro
#include <MitkWiiMoteExports.h>

namespace mitk
{
  /**
  * \deprecatedSince{2014_03} mitk::WiiMoteInteractor is deprecated. Needs to be updated to the new interaction-framework.
  * Refer to \see DataInteractionPage for general information about the concept of the new implementation.
  */

class DEPRECATED() mitkWiiMote_EXPORT WiiMoteInteractor: public mitk::Interactor
{

public:

// SmartPointer macros
mitkClassMacro(WiiMoteInteractor, Interactor);
mitkNewMacro2Param(Self, const char*, DataNode*);

protected:

// SmartPointer conventions
WiiMoteInteractor(const char* type, DataNode* dataNode);
virtual ~WiiMoteInteractor();

// allow movement
virtual bool OnWiiMoteResetButton(Action* action, const mitk::StateEvent* event);

// movement
virtual bool OnWiiMoteInput(Action* action, const mitk::StateEvent* event);

// stopping movement
virtual bool OnWiiMoteReleaseButton(Action* action, const mitk::StateEvent* event);

private:

  mitk::Geometry3D* TransformCurrentDataInGeometry3D();

  // all movements are separated and fixed
  bool FixedRotationAndTranslation(const mitk::WiiMoteAllDataEvent* event);

  vnl_matrix_fixed<double, 4, 4> ComputeCurrentCameraPosition(vtkCamera* vtkCamera);

  // combined movements and different interaction modes
  bool DynamicRotationAndTranslation(Geometry3D* geometry);

  float m_OrientationX;
  float m_OrientationY;
  float m_OrientationZ;

  float m_xVelocity;
  float m_yVelocity;
  float m_zVelocity;

  float m_xValue;
  float m_yValue;
  float m_zValue;

  // refering to an angle around an axis
  // which is defined in the wiimote
  ScalarType m_xAngle;
  ScalarType m_yAngle;
  ScalarType m_zAngle;

  // modes
  bool m_InRotation;
  int m_TranslationMode;

  // default: 1 = relative to object
  // 2 = relative to camera view
  int m_SurfaceInteractionMode;

  // to reset the geometry
  mitk::Geometry3D::Pointer m_OriginalGeometry;

  mitk::BaseRenderer::Pointer m_BaseRenderer;
};

}
#endif // MITK_WIIMOTEINTERACTOR_H
