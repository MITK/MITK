#define MITK_WRAP_PACKAGE "mitkDataManagementPython"
//#include "wrap_MITKAlgorithms.cxx"
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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifdef CABLE_CONFIGURATION

namespace _cable_
{
  const char* const package = MITK_WRAP_PACKAGE;
  const char* const groups[] =
  {
    //"VtkAbstractTransform",
      "AbstractTransformGeometry",
      "AnnotationProperty",
      "ApplicationCursor",
      "BaseData",
      "BaseDataImplementation",
      "BaseProperty",
      "ClippingProperty",
      "ColorProperty",
      //"Common",
      "DataNode",
      "DataStorage",
      "DisplayGeometry",
      "EnumerationProperty",
      "GenericLookupTable",
      //"GenericProperty",
      "Geometry2D",
      "Geometry2DData",
      "Geometry3D",
      "GeometryData",
      "GroupTagProperty",
      "Image",
      "ImageDataItem",
      //"ItkMatirxHack",
      "LandmarkBasedCurvedGeometry",
      "LandmarkProjector",
      "LandmarkProjectorBasedCurvedGeometry",
      "LevelWindow",
      "LevelWindowManager",
      "LevelWindowPreset",
      "LevelWindowProperty",
      //Line",
      "LookupTable",
      //"LookupTables",
      "Material",
      //"MatrixConvert",
      "ModalityProperty",
      "NodePredicateAND",
      "NodePredicateBase",
      "NodePredicateCompositeBase",
    "NodePredicateData",
      "NodePredicateDataType",
      "NodePredicateDimension",
      "NodePredicateFirstLevel",
      "NodePredicateNOT",
      "NodePredicateOR",
      "NodePredicateProperty",
      "NodePredicateSource",
      "PlaneDecorationProperty",
      "PlaneGeometry",
      "PlaneOperation",
      //"PointOperation",
      "PointSet",
      //"Properties",
      "PropertyList",
      "ResliceMethodEnumProperty",
      "RotationOperation",
      "ShaderEnumProperty",
      "SlicedData",
      //"SlicedGeometry3D",
      "SmartPointerProperty",
      "StandaloneDataStorage",
      "StateTransitionOperation",
      "StringProperty",
      "Surface",
      "ThinPlateSplineCurvedGeometry",
      "TimeSlicedGeometry",
      "TransferFunction",
      "TransferFunctionProperty",
      //"Vector",
      "VtkInterpolationProperty",
      "VtkRepresentationProperty",
      "VtkResliceInterpolationProperty",
      "VtkScalarModeProperty",
      "VtkVolumeRenderingProperty",
      //"WeakPointer",
      "WeakPointerProperty",
  };
}
#endif
