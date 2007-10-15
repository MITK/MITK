/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#include "ipSegmentation.h"


/**
 * Input: one contour (start and end point identical). array of floats. [x1 y1 x2 y2 ...]
 * Output: two contours (start and end point not neccessarily identical)
 * cutCoords: two points that define the cut (must not [should not be] be identical)
 *                                                                     
 *              S.....O
 *            .        .
 *           .           .
 *         .               .
 *        .            ------C2      S: Start of contour (two times in the array)
 *       .      -------      .       C1: cut point 1
 *      C1------             .       C2: cut point 2
 *        .                  O
 *         .               ..
 *           .          ..
 *            .      ..
 *              . ..
 *               O
 */
void ipMITKSegmentationSplitContour( float *contour, int sizeContour, float *cutCoords, float *part1, int &size1, float *part2, int &size2 )
{
  int cut1, cut2, i=0;
  bool cutReached;
  size1 = 0;
  size2 = 0;

  do {
    part1[2*size1] = contour[2*i];
    part1[2*size1+1] = contour[2*i+1];
    i++;
    size1++;
    cutReached = ( (cutCoords[0]==contour[2*i] && cutCoords[1]==contour[2*i+1]) ||
             (cutCoords[2]==contour[2*i] && cutCoords[3]==contour[2*i+1])    );
  } while (!cutReached);
  cut1 = i;
  part1[2*size1] = contour[2*i];
  part1[2*size1+1] = contour[2*i+1];
  size1++;
  
  do {
    part2[2*size2] = contour[2*i];
    part2[2*size2+1] = contour[2*i+1];
    i++;
    size2++;
    cutReached = ( (cutCoords[0]==contour[2*i] && cutCoords[1]==contour[2*i+1]) ||
             (cutCoords[2]==contour[2*i] && cutCoords[3]==contour[2*i+1])    );
  } while (!cutReached);
  cut2 = i;
  part2[2*size2] = contour[2*i];
  part2[2*size2+1] = contour[2*i+1];
  size2++;

  do {
    part1[2*size1] = contour[2*i];
    part1[2*size1+1] = contour[2*i+1];
    i++;
    size1++;
  } while (i<sizeContour);
}


/*
 * This is taken from the FAQ of comp.graphics.algorithms
 *
 * http://www.exaflop.org/docs/cgafaq/cga2.html
 *
 * This FAQ references:
 *
 *   Graphics Gems IV, Paul S. Heckbert (ed.), Academic Press 1994, ISBN 0-12-336155-9
 *   Computational Geometry in C (2nd Ed.), Joseph O'Rourke, Cambridge University Press 1998, ISBN 0-521-64010-5
 *   An Introduction to Ray Tracing, Andrew Glassner (ed.), Academic Press 1989, ISBN 0-12-286160-4
 *
*/
bool ipMITKSegmentationIsInsideContour( float *contour, int sizeContour, float x, float y )
{

  int i, j;
  bool res = false;
  for (i=0, j=sizeContour-1; i<sizeContour; j=i++) {
    if ((((contour[2*i+1]<=y) && (y<contour[2*j+1])) ||  ((contour[2*j+1]<=y) && (y<contour[2*i+1]))) &&
      (x < (contour[2*j] - contour[2*i]) * (y - contour[2*i+1]) / (contour[2*j+1] - contour[2*i+1]) + contour[2*i]))

      res = !res;
  }
  return res;
}

