/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

__kernel void ckUsedLines(
  __global unsigned short* dDest, // output buffer
  float partMult,
  unsigned int inputL,
  unsigned int inputS,
  unsigned int outputL,
  unsigned int outputS,
  float totalSamples_i // parameters
)
{
  // get thread identifier
  unsigned int globalPosX = get_global_id(0);
  unsigned int globalPosY = get_global_id(1);

  // terminate non-valid threads
  if ( globalPosX < outputL && globalPosY < outputS)
  {
    float l_i = (float)globalPosX / outputL * inputL;
    float s_i = (float)globalPosY / (float)outputS * totalSamples_i;

    float part = partMult * s_i;
    if (part < 1)
      part = 1;

    unsigned short maxLine = min((l_i + part) + 1, (float)inputL);
    unsigned short minLine = max((l_i - part), 0.0f);

    dDest[globalPosY * 3 * outputL + 3 * globalPosX] = (maxLine - minLine); //usedLines
    dDest[globalPosY * 3 * outputL + 3 * globalPosX + 1] = minLine; //minLine
    dDest[globalPosY * 3 * outputL + 3 * globalPosX + 2] = maxLine; //maxLine
  }
}

__kernel void ckUsedLines_g(
  __global unsigned short* dDest, // output buffer
  __global float* elementHeights,
  __global float* elementPositions,
  float sin_deg,
  float probeRadius,
  unsigned int inputL,
  unsigned int inputS,
  unsigned int outputL,
  unsigned int outputS,
  float horizontalExtent,
  float verticalExtent
)
{
  // get thread identifier
  unsigned int globalPosX = get_global_id(0);
  unsigned int globalPosY = get_global_id(1);

  float x_sensor_pos = 0;
  float y_sensor_pos = 0;
  float center_to_sensor_a = 0;
  float center_to_sensor_b = 0;
  float center_to_sensor_c = 0;
  float distance_to_sensor_direction = 0;
  float distance_sensor_target = 0;

  // terminate non-valid threads
  if ( globalPosX < outputL && globalPosY < outputS)
  {
    float x_cm = (float)globalPosX / outputL * horizontalExtent; // x*Xspacing
    float y_cm = (float)globalPosY / (float)outputS * verticalExtent; // y*Yspacing

    int maxLine = inputL;
    int minLine = 0;

    float x_center_pos = horizontalExtent / 2.0;
    float y_center_pos = probeRadius;

    for (int l_s = minLine; l_s <= inputL; l_s += 1)
    {
      x_sensor_pos = elementPositions[l_s];
      y_sensor_pos = elementHeights[l_s];

      distance_sensor_target = sqrt((x_cm - x_sensor_pos)*(x_cm - x_sensor_pos) + (y_cm - y_sensor_pos)*(y_cm - y_sensor_pos));

      // solving line equation
      center_to_sensor_a = y_sensor_pos - y_center_pos;
      center_to_sensor_b = x_center_pos - x_sensor_pos;
      center_to_sensor_c = -(center_to_sensor_a * x_center_pos + center_to_sensor_b * y_center_pos);
      distance_to_sensor_direction = fabs((center_to_sensor_a * x_cm + center_to_sensor_b * y_cm + center_to_sensor_c)) / (sqrt(center_to_sensor_a*center_to_sensor_a + center_to_sensor_b*center_to_sensor_b));

      if (distance_to_sensor_direction < (sin_deg*distance_sensor_target))
      {
        minLine = l_s;
        break;
      }
    }

    for (int l_s = maxLine - 1; l_s > minLine; l_s -= 1) // start with maxline-1 otherwise elementPositions[] will go out of range
    {
      x_sensor_pos = elementPositions[l_s];
      y_sensor_pos = elementHeights[l_s];

      distance_sensor_target = sqrt((x_cm - x_sensor_pos)*(x_cm - x_sensor_pos) + (y_cm - y_sensor_pos)*(y_cm - y_sensor_pos));

      // solving line equation
      center_to_sensor_a = y_sensor_pos - y_center_pos;
      center_to_sensor_b = x_center_pos - x_sensor_pos;
      center_to_sensor_c = -(center_to_sensor_a * x_center_pos + center_to_sensor_b * y_center_pos);
      distance_to_sensor_direction = fabs((center_to_sensor_a * x_cm + center_to_sensor_b * y_cm + center_to_sensor_c)) / (sqrt(center_to_sensor_a*center_to_sensor_a + center_to_sensor_b*center_to_sensor_b));

      if (distance_to_sensor_direction < sin_deg*distance_sensor_target)
      {
        maxLine = l_s;
        break;
      }
    }

    dDest[globalPosY * 3 * outputL + 3 * globalPosX] = (maxLine - minLine); //usedLines
    dDest[globalPosY * 3 * outputL + 3 * globalPosX + 1] = minLine; //minLine
    dDest[globalPosY * 3 * outputL + 3 * globalPosX + 2] = maxLine; //maxLine
  }
}
