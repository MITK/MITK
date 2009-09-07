TEMPLATE = lib
TARGET = ipFunc
LANGUAGE = C
CONFIG += warn_on release

OBJECTS_DIR = obj

INCLUDEPATH = ../../DataStructures

unix {
	CONFIG += staticlib
}

win32 {
	INCLUDEPATH += ../../DataStructures/ipPic
}

HEADERS		= gradient.h \
		  ipAdministrative.h \
		  ipArithmeticAndLogical.h \
		  mitkIpFunc.h \
		  mitkIpFuncP.h \
		  ipGeometrical.h \
		  ipLocal.h \
		  ipMorphological.h \
		  ipPointOperators.h \
		  ipStatistical.h

SOURCES		= AddC.c \
		  AddI.c \
		  AddSl.c \
		  And.c \
		  Border.c \
		  BorderX.c \
		  Box.c \
		  Box2d.c \
		  Box3d.c \
		  Canny.c \
		  Close.c \
		  CompressM.c \
		  Conv.c \
		  Convert.c \
		  CopyTags.c \
		  Curtosis.c \
		  CurtosisR.c \
		  Dila.c \
		  DivC.c \
		  DivI.c \
		  DrawPoly.c \
		  Equal.c \
		  Ero.c \
		  Error.c \
		  Exp.c \
		  ExtT.c \
		  ExtrC.c \
		  ExtrR.c \
		  ExtrROI.c \
		  Extrema.c \
		  FillArea.c \
		  Frame.c \
		  GaussF.c \
		  Grad.c \
		  Grav.c \
		  Hist.c \
		  Histo.c \
		  HitMiss.c \
		  HitMissI.c \
		  Inertia.c \
		  Inv.c \
		  LN.c \
		  Label.c \
		  Laplace.c \
		  LevWind.c \
		  Log.c \
		  MakePicName.c \
		  Malloc.c \
		  Mean.c \
		  MeanC.c \
		  MeanF.c \
		  MeanR.c \
		  MeanROI.c \
		  Median.c \
		  Morph.c \
		  MultC.c \
		  MultI.c \
		  Norm.c \
		  NormXY.c \
		  Not.c \
		  OpCl.c \
		  Open.c \
		  Or.c \
		  Pot.c \
		  Range.c \
		  Rank.c \
		  Refl.c \
		  RegGrow.c \
		  Roberts.c \
		  Rotate.c \
		  SDev.c \
		  SDevC.c \
		  SDevR.c \
		  SDevROI.c \
		  ScBl.c \
		  ScFact.c \
		  ScNN.c \
		  Scale.c \
		  SelInv.c \
		  SelMM.c \
		  Select.c \
		  SetErrno.c \
		  SetTag.c \
		  Shp.c \
		  Skewness.c \
		  SkewnessR.c \
		  Sobel.c \
		  Sqrt.c \
		  SubC.c \
		  SubI.c \
		  Thresh.c \
		  Transpose.c \
		  Var.c \
		  VarC.c \
		  VarR.c \
		  VarROI.c \
		  Window.c \
		  WindowR.c \
		  Xchange.c \
		  ZeroCr.c \
		  _BorderX.c \
		  _DrawPoly.c \
		  ipGetANew.c

