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


#include "mitkIsoLevelsGenerator.h"

mitk::IsoDoseLevelSet::Pointer mitk::GenerateIsoLevels_Virtuos()
{
  mitk::IsoDoseLevelSet::Pointer levelSet = mitk::IsoDoseLevelSet::New();

  mitk::IsoDoseLevel::ColorType color;
  color[0] = 0.0;
  color[1] = 0.0;
  color[2] = 0.4;
  mitk::IsoDoseLevel::Pointer level = mitk::IsoDoseLevel::New(0.01,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 0.0;
  color[1] = 0.2;
  color[2] = 0.8;
  level = mitk::IsoDoseLevel::New(0.1,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 0.0;
  color[1] = 0.4;
  color[2] = 1.0;
  level = mitk::IsoDoseLevel::New(0.2,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 0.0;
  color[1] = 0.7;
  color[2] = 1.0;
  level = mitk::IsoDoseLevel::New(0.3,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 0.0;
  color[1] = 0.7;
  color[2] = 0.6;
  level = mitk::IsoDoseLevel::New(0.4,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 0.0;
  color[1] = 1.0;
  color[2] = 0.3;
  level = mitk::IsoDoseLevel::New(0.5,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 1.0;
  color[2] = 0.6;
  level = mitk::IsoDoseLevel::New(0.6,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 1.0;
  color[2] = 0.0;
  level = mitk::IsoDoseLevel::New(0.7,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 0.8;
  color[2] = 0.0;
  level = mitk::IsoDoseLevel::New(0.8,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 0.5;
  color[2] = 0.0;
  level = mitk::IsoDoseLevel::New(0.9,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 0.4;
  color[2] = 0.0;
  level = mitk::IsoDoseLevel::New(0.95,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 0.2;
  color[2] = 0.0;
  level = mitk::IsoDoseLevel::New(1.0,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 0.0;
  color[2] = 0.3;
  level = mitk::IsoDoseLevel::New(1.07,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 0.0;
  color[2] = 0.4;
  level = mitk::IsoDoseLevel::New(1.1,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 0.4;
  color[2] = 0.4;
  level = mitk::IsoDoseLevel::New(1.2,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 1.0;
  color[1] = 0.7;
  color[2] = 0.7;
  level = mitk::IsoDoseLevel::New(1.3,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 0.8;
  color[1] = 0.6;
  color[2] = 0.6;
  level = mitk::IsoDoseLevel::New(1.4,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  color[0] = 0.65;
  color[1] = 0.4;
  color[2] = 0.4;
  level = mitk::IsoDoseLevel::New(1.5,color,true,true);
  levelSet->SetIsoDoseLevel(level);

  return levelSet;
}
