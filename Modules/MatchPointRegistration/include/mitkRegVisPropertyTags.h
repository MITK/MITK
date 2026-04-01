/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkRegVisPropertyTags_h
#define mitkRegVisPropertyTags_h

// MITK
#include <MitkMatchPointRegistrationExports.h>

/**
 * \brief Property tag constants for registration visualization and evaluation in MITK.
 *
 * These string constants define the data node property keys that control how
 * MatchPoint registrations are visualized (grid, glyph, points) and how
 * registration evaluations are rendered (blend, checkerboard, wipe, contour).
 *
 * \sa mitk::RegVisHelper, mitk::RegEvaluationMapper2D, mitk::MITKRegistrationWrapperMapperBase
 */
namespace mitk
{
/** \brief Property key controlling whether a registration grid visualization is shown. */
const char* const nodeProp_RegVisGrid = "matchpoint.RegVis.Grid";
/** \brief Property key controlling whether a glyph-based registration visualization is shown. */
const char* const nodeProp_RegVisGlyph = "matchpoint.RegVis.Glyph";
/** \brief Property key controlling whether a point-based registration visualization is shown. */
const char* const nodeProp_RegVisPoints = "matchpoint.RegVis.Points";
/** \brief Property key specifying the mapping direction for registration visualization (direct or inverse). */
const char* const nodeProp_RegVisDirection = "matchpoint.RegVis.Direction";
/** \brief Property key for the field-of-view size of the registration visualization. */
const char* const nodeProp_RegVisFOVSize = "matchpoint.RegVis.FOV.size";
/** \brief Property key for the field-of-view origin of the registration visualization. */
const char* const nodeProp_RegVisFOVOrigin = "matchpoint.RegVis.FOV.origin";
/** \brief Property key for the field-of-view spacing of the registration visualization. */
const char* const nodeProp_RegVisFOVSpacing = "matchpoint.RegVis.FOV.spacing";
/** \brief Property key for the first row of the field-of-view orientation matrix. */
const char* const nodeProp_RegVisFOVOrientation1 = "matchpoint.RegVis.FOV.orientation.row.1";
/** \brief Property key for the second row of the field-of-view orientation matrix. */
const char* const nodeProp_RegVisFOVOrientation2 = "matchpoint.RegVis.FOV.orientation.row.2";
/** \brief Property key for the third row of the field-of-view orientation matrix. */
const char* const nodeProp_RegVisFOVOrientation3 = "matchpoint.RegVis.FOV.orientation.row.3";
/** \brief Property key for the grid frequency (number of grid lines) in registration visualization. */
const char* const nodeProp_RegVisGridFrequence = "matchpoint.RegVis.Grid.Frequence";
/** \brief Property key controlling whether the undeformed start grid is shown alongside the deformed grid. */
const char* const nodeProp_RegVisGridShowStart = "matchpoint.RegVis.Grid.ShowStart";
/** \brief Property key for the color style used in registration visualization (uniform or vector-magnitude-based). */
const char* const nodeProp_RegVisColorStyle = "matchpoint.RegVis.ColorStyle";
/** \brief Property key for the color of the undeformed start grid. */
const char* const nodeProp_RegVisGridStartColor = "matchpoint.RegVis.Grid.StartColor";
/** \brief Property key for the uniform color used in registration visualization. */
const char* const nodeProp_RegVisColorUni = "matchpoint.RegVis.Color.uni";
/** \brief Property key for the first color value in the color transfer function. */
const char* const nodeProp_RegVisColor1Value = "matchpoint.RegVis.Color.1.value";
/** \brief Property key for the first magnitude threshold in the color transfer function. */
const char* const nodeProp_RegVisColor1Magnitude = "matchpoint.RegVis.Color.1.magnitude";
/** \brief Property key for the second color value in the color transfer function. */
const char* const nodeProp_RegVisColor2Value = "matchpoint.RegVis.Color.2.value";
/** \brief Property key for the second magnitude threshold in the color transfer function. */
const char* const nodeProp_RegVisColor2Magnitude = "matchpoint.RegVis.Color.2.magnitude";
/** \brief Property key for the third color value in the color transfer function. */
const char* const nodeProp_RegVisColor3Value = "matchpoint.RegVis.Color.3.value";
/** \brief Property key for the third magnitude threshold in the color transfer function. */
const char* const nodeProp_RegVisColor3Magnitude = "matchpoint.RegVis.Color.3.magnitude";
/** \brief Property key for the fourth color value in the color transfer function. */
const char* const nodeProp_RegVisColor4Value = "matchpoint.RegVis.Color.4.value";
/** \brief Property key for the fourth magnitude threshold in the color transfer function. */
const char* const nodeProp_RegVisColor4Magnitude = "matchpoint.RegVis.Color.4.magnitude";
/** \brief Property key controlling whether colors are interpolated between transfer function control points. */
const char* const nodeProp_RegVisColorInterpolate = "matchpoint.RegVis.ColorInterpolate";
/** \brief Property key for the registration evaluation visualization style (Blend, Checkerboard, Wipe, etc.). */
const char* const nodeProp_RegEvalStyle = "matchpoint.RegEval.Style";
/** \brief Property key for the blend factor used in Blend evaluation style. */
const char* const nodeProp_RegEvalBlendFactor = "matchpoint.RegEval.BlendFactor";
/** \brief Property key for the number of checkers used in Checkerboard evaluation style. */
const char* const nodeProp_RegEvalCheckerCount = "matchpoint.RegEval.CheckerCount";
/** \brief Property key for the wipe style used in Wipe evaluation mode (Cross, Horizontal, Vertical). */
const char* const nodeProp_RegEvalWipeStyle = "matchpoint.RegEval.WipeStyle";
/** \brief Property key controlling whether target contour overlay is shown in evaluation. */
const char* const nodeProp_RegEvalTargetContour = "matchpoint.RegEval.TargetContour";
/** \brief Property key for the current crosshair position used in evaluation rendering. */
const char* const nodeProp_RegEvalCurrentPosition = "matchpoint.RegEval.CurrentPosition";
}


#endif
