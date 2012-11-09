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

#include "mitkTransferFunctionInitializer.h"

static const char* presetNames[] = {  "choose an internal transferfunction preset",
                              "CT Generic",
                              "CT Black & White",
                              "CT Thorax large",
                              "CT Thorax small",
                              "CT Bone",
                              "CT Bone (with Gradient)",
                              "CT Cardiac",
                              "MR Generic" };

static int numPresetNames = sizeof( presetNames ) / sizeof( char * );

namespace mitk
{

  TransferFunctionInitializer::TransferFunctionInitializer(mitk::TransferFunction::Pointer transferFunction)
{
  if (transferFunction.IsNotNull())
  {
    m_transferFunction = transferFunction;
  }
}

TransferFunctionInitializer::~TransferFunctionInitializer()
{

}

void TransferFunctionInitializer::GetPresetNames(std::vector<std::string>& names)
{
  for (int i = 0; i < numPresetNames; ++i)
  {
    names.push_back(presetNames[i]);
  }
}

void TransferFunctionInitializer::SetTransferFunction(mitk::TransferFunction::Pointer transferFunction)
{
  m_transferFunction = transferFunction;
}

void TransferFunctionInitializer::SetTransferFunctionMode( int mode )
{
  if(m_transferFunction)
  {
    m_Mode = mode;
    this->InitTransferFunctionMode();
  }
}

void TransferFunctionInitializer::RemoveAllPoints()
{
  m_transferFunction->GetScalarOpacityFunction()->RemoveAllPoints();
  m_transferFunction->GetColorTransferFunction()->RemoveAllPoints();
  m_transferFunction->GetGradientOpacityFunction()->RemoveAllPoints();
}

void TransferFunctionInitializer::SetModified()
{
  m_transferFunction->GetScalarOpacityFunction()->Modified();
  m_transferFunction->GetColorTransferFunction()->Modified();
  m_transferFunction->GetGradientOpacityFunction()->Modified();
}

void TransferFunctionInitializer::InitTransferFunctionMode()
{
  if(m_transferFunction)
  {
    this->RemoveAllPoints();
    switch( this->m_Mode )
    {
      case TF_CT_DEFAULT:
        this->SetCtDefaultMode();
        break;

      case TF_CT_BLACK_WHITE:
        this->SetCtBlackWhiteMode();
        break;

      case TF_CT_THORAX_LARGE:
        this->SetCtThoraxLargeMode();
        break;

      case TF_CT_THORAX_SMALL:
        this->SetCtThoraxSmallMode();
        break;

      case TF_CT_BONE:
        this->SetCtBoneMode();
        break;

      case TF_CT_BONE_GRADIENT:
        this->SetCtBoneGradientMode();
        break;

      case TF_CT_CARDIAC:
        this->SetCtCardiacMode();
        break;

      case TF_MR_GENERIC:
        this->SetMrGenericMode();
        break;

      default:
        itkExceptionMacro(<< "No Mode set!");
        break;
    }

    this->SetModified();
  } else {
    itkExceptionMacro(<< "No Transferfunction set!");
  }
}

void TransferFunctionInitializer::SetCtDefaultMode()
{
  // grayvalue->opacity
  m_transferFunction->GetScalarOpacityFunction()->AddPoint(132.108911,0.000000);
  m_transferFunction->GetScalarOpacityFunction()->AddPoint(197.063492,0.041333);
  m_transferFunction->GetScalarOpacityFunction()->AddPoint(1087.917989,0.700000);

  // gradient at grayvalue->opacity
  m_transferFunction->GetGradientOpacityFunction()->AddPoint(560.695000,1.000000);

  // grayvalue->color
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint(176.881890,0.650980,0.000000,0.000000);
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint(239.427822,0.933333,0.000000,0.000000);
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint(301.973753,1.000000,0.800000,0.062745);
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint(495.866142,1.000000,0.905882,0.666667);
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint(677.249344,1.000000,0.882353,0.215686);
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint(808.595801,1.000000,1.000000,1.000000);
}

void TransferFunctionInitializer::SetCtBlackWhiteMode()
{
  //Set Opacity
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 135.063521, 0.0 );
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 948.137931, 1.0 );


  //Set Color
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 122.088929, 0.352941, 0.352941, 0.352941);
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 372.931034, 1.000000, 1.000000, 1.000000 );

  //Set Gradient
  m_transferFunction->GetGradientOpacityFunction()->Initialize();
  m_transferFunction->GetGradientOpacityFunction()->AddPoint( 560.695000, 1);

}

void TransferFunctionInitializer::SetCtThoraxLargeMode()
{
  // grayvalue->opacity

    m_transferFunction->GetScalarOpacityFunction()->AddPoint(76.721239,0.000000);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(139.524336,0.000000);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(274.458333,0.228650);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(638.420139,0.721763);


  // gradient at grayvalue->opacity
    m_transferFunction->GetGradientOpacityFunction()->AddPoint(560.695000,1.000000);

  // grayvalue->color
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 85.382743,0.478431,0.000000,0.000000);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 198.201327,0.933333,0.000000,0.000000);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 254.610619,1.000000,0.800000,0.062745);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 336.0907085,1.000000,0.905882,0.666667);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 630.672566,1.000000,1.000000,1.000000);
}

void TransferFunctionInitializer::SetCtThoraxSmallMode()
{
  // grayvalue->opacity
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(147.216912,0.000000);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(274.458333,0.228650);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(430.330882,0.675532);


  // gradient at grayvalue->opacity
    m_transferFunction->GetGradientOpacityFunction()->AddPoint(560.695000,1.000000);


  // grayvalue->color
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 129.607774,0.478431,0.000000,0.000000);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 213.812721,0.933333,0.000000,0.000000);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 348.540636,1.000000,0.800000,0.062745);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 500.419118,1.000000,0.898039,0.776471);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 579.268382,1.000000,1.000000,1.000000);
}

void TransferFunctionInitializer::SetCtBoneMode()
{
  // grayvalue->opacity
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(126.413793,0.000000);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(178.312160,0.014663);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(247.509982,0.000000);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(1013.010889,1.000000);

  // gradient at grayvalue->opacity
    m_transferFunction->GetGradientOpacityFunction()->AddPoint(485.377495,1.000000);

  // grayvalue->color
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 312.382940,1.000000,0.564706,0.274510);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 455.103448,1.000000,0.945098,0.768627);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 623.773140,1.000000,0.800000,0.333333);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 796.767695,1.000000,0.901961,0.815686);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 930.838475,1.000000,1.000000,1.000000);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint(1073.558984,1.000000,0.839216,0.423529);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint(1220.604356,1.000000,0.772549,0.490196);

}

void TransferFunctionInitializer::SetCtBoneGradientMode()
{
  // grayvalue->opacity
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(126.413793,0.000000);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(186.961887,0.146628);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(247.509982,0.000000);
    m_transferFunction->GetScalarOpacityFunction()->AddPoint(1013.010889,1.000000);

  // gradient at grayvalue->opacity
    m_transferFunction->GetGradientOpacityFunction()->AddPoint(22.617060,0.000000);
    m_transferFunction->GetGradientOpacityFunction()->AddPoint(65.865699,1.000000);

  // grayvalue->color
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 312.382940,1.000000,0.564706,0.274510);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 455.103448,1.000000,0.945098,0.768627);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 623.773140,1.000000,0.800000,0.333333);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 796.767695,1.000000,0.901961,0.815686);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 930.838475,1.000000,1.000000,1.000000);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint(1073.558984,1.000000,0.839216,0.423529);
    m_transferFunction->GetColorTransferFunction()->AddRGBPoint(1220.604356,1.000000,0.772549,0.490196);
}

void TransferFunctionInitializer::SetCtCardiacMode()
{
  //Set Opacity
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 150.246824, 0.000000 );
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 179.974592, 0.202346);
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 276.589837, 0.000000);
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 781.961887, 1.000000);

  //Set Color
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 395.500907,1.000000,0.000000,0.000000);
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 410.364791,1.000000,0.749020,0.000000);
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 484.684211,1.000000,0.878431,0.662745);
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 588.731397,1.000000,0.784314,0.482353);

  //Set Gradient
  m_transferFunction->GetGradientOpacityFunction()->AddPoint( 246.862069, 0.215827 );
}

void TransferFunctionInitializer::SetMrGenericMode()
{
  //Set Opacity
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 0, 0 );
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 20, 0 );
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 40, 0.15 );
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 120, 0.3 );
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 220, 0.375 );
  m_transferFunction->GetScalarOpacityFunction()->AddPoint( 1024, 0.5);

  //Set Color
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 0, 0, 0, 0 );
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 20, 0.168627, 0, 0 );
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 40, 0.403922, 0.145098, 0.0784314  );
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 120, 0.780392, 0.607843, 0.380392  );
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 220, 0.847059, 0.835294, 0.788235  );
  m_transferFunction->GetColorTransferFunction()->AddRGBPoint( 1024, 1, 1, 1  );

  //Set Gradient
  m_transferFunction->GetGradientOpacityFunction()->AddPoint( 0, 1 );
  m_transferFunction->GetGradientOpacityFunction()->AddPoint( 255, 1);
}

mitk::TransferFunction::Pointer TransferFunctionInitializer::GetTransferFunction()
{
  return m_transferFunction;
}
} // namespace
