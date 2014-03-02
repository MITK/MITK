INCLUDES = -I../ipPic

EXTRA_DIST = libipFunc.dsp ipFunc.dsp ipFunc.dox

bin_PROGRAMS = ipFunc
lib_LIBRARIES = libipFunc.a
pkginclude_HEADERS = mitkIpFunc.h
noinst_HEADERS =		\
      gradient.h               	\
      ipArithmeticAndLogical.h 	\
      mitkIpFuncP.h                	\
      ipGeometrical.h          	\
      ipLocal.h                	\
      ipMorphological.h        	\
      ipPointOperators.h       	\
      ipStatistical.h            \
      ipAdministrative.h

ipFunc_LDADD = ./libipFunc.a ../ipPic/libipPic.a

ipFunc_SOURCES = \
	ipFunc.c

libipFunc_a_SOURCES = \
      AddC.c          \
      AddI.c          \
      AddSl.c         \
      And.c           \
      Border.c        \
      BorderX.c       \
      _BorderX.c      \
      Box.c           \
      Box2d.c         \
      Box3d.c         \
      Canny.c         \
      Close.c         \
      CompressM.c     \
      Conv.c          \
      Convert.c       \
      CopyTags.c      \
      Curtosis.c      \
      CurtosisR.c     \
      Dila.c          \
      DivC.c          \
      DivI.c          \
      _DrawPoly.c     \
      DrawPoly.c      \
      Ero.c           \
      Equal.c         \
      Error.c         \
      Exp.c           \
      Extrema.c       \
      ExtrC.c         \
      ExtrROI.c       \
      ExtrR.c         \
      ExtT.c          \
      Frame.c         \
      GaussF.c        \
      Grad.c          \
      Grav.c          \
      Hist.c          \
      Histo.c		\
      HitMiss.c       \
      HitMissI.c      \
      Inertia.c       \
      Inv.c           \
      LN.c            \
      Label.c         \
      Laplace.c       \
      LevWind.c       \
      Log.c           \
      Mean.c          \
      MeanC.c         \
      MeanF.c         \
      MeanROI.c       \
      MeanR.c         \
      Median.c        \
      Morph.c         \
      MultC.c         \
      MultI.c         \
      Norm.c          \
      NormXY.c        \
      Not.c           \
      Open.c          \
      OpCl.c          \
      Or.c            \
      Malloc.c        \
      Pot.c           \
      Range.c         \
      Rank.c          \
      Refl.c          \
      RegGrow.c       \
      Roberts.c       \
      Rotate.c        \
      Scale.c         \
      ScNN.c          \
      ScFact.c        \
      ScBl.c          \
      SDev.c          \
      SDevC.c         \
      SDevROI.c       \
      SDevR.c         \
      Select.c        \
      SelInv.c        \
      SelMM.c         \
      SetErrno.c      \
      SetTag.c        \
      Skewness.c      \
      SkewnessR.c     \
      Shp.c           \
      Sobel.c         \
      Sqrt.c          \
      SubC.c          \
      SubI.c          \
      Thresh.c        \
      Transpose.c     \
      Var.c           \
      VarC.c          \
      VarROI.c        \
      VarR.c          \
      Window.c        \
      WindowR.c       \
      Xchange.c       \
      ZeroCr.c        \
      ipGetANew.c     \
      FillArea.c      \
      MakePicName.c
