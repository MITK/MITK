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

#include "mitkPlaneCutFilter.h"

#include <mitkImageAccessByItk.h>
#include <mitkLine.h>

#define roundf(x) ((x - floor(x)) > 0.5f ? ceil(x) : floor(x))

void mitk::PlaneCutFilter::GenerateData()
{
  if (!this->m_Plane)
  {
    return;
  }

  InputImageType *input = const_cast<InputImageType*>(this->GetInput());

  if (!input)
  {
    return;
  }

  //Allocate output.
  OutputImageType *output = this->GetOutput();

  output->Initialize(input);
  output->SetImportVolume(input->GetData());

  //Do the intersection.
  AccessByItk_2(output, _computeIntersection, this->m_Plane, input->GetGeometry());
}

mitk::PlaneCutFilter::PlaneCutFilter()
  : m_BackgroundLevel(0.0f), m_Plane(0), m_FillMode(FILL)
{
}

mitk::PlaneCutFilter::~PlaneCutFilter()
{
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::PlaneCutFilter::_computeIntersection(itk::Image<TPixel, VImageDimension> *image, const PlaneGeometry *plane, const Geometry3D *geometry)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;

  const typename ImageType::RegionType &image_region = image->GetLargestPossibleRegion();
  const typename ImageType::SizeValueType
      max_x = image_region.GetSize(0ul),
      max_y = image_region.GetSize(1ul),
      max_z = image_region.GetSize(2ul),
      img_size = max_x * max_y;
  TPixel *data = image->GetBufferPointer();
  Point3D p1, p2;

  //TODO: Better solution required!
  TPixel casted_background_level = static_cast<TPixel>(this->m_BackgroundLevel);

  p1[0] = 0;
  p2[0] = max_x - 1ul;

  if (FILL == this->m_FillMode)
  {
    for (unsigned long z = 0ul; z < max_z; ++z)
    {
      p1[2] = z;
      p2[2] = z;

      for (unsigned long y = 0ul; y < max_y; ++y)
      {
        p1[1] = y;
        p2[1] = y;

        Point3D p1_t, p2_t;

        geometry->IndexToWorld(p1, p1_t);
        geometry->IndexToWorld(p2, p2_t);

        if (plane->IsAbove(p1_t))
        {
          if (plane->IsAbove(p2_t))
          {
            if (0.0f == this->m_BackgroundLevel)
            {
              memset(&data[(y * max_x) + (z * img_size)], 0, max_x * sizeof(TPixel));
            }
            else
            {
              TPixel *subdata = &data[(y * max_x) + (z * img_size)];

              for (unsigned long x = 0; x < max_x; ++x)
              {
                subdata[x] = casted_background_level;
              }
            }
          }
          else
          {
            Point3D intersection;
            Line3D line;

            line.SetPoints(p1_t, p2_t);
            plane->IntersectionPoint(line, intersection);
            geometry->WorldToIndex(intersection, intersection);

            if (0.0f == this->m_BackgroundLevel)
            {
              memset(&data[(y * max_x) + (z * img_size)], 0, (static_cast<unsigned long>(roundf(intersection[0])) + 1u) * sizeof(TPixel));
            }
            else
            {

              TPixel *subdata = &data[(y * max_x) + (z * img_size)];
              const unsigned long x_size = static_cast<unsigned long>(roundf(intersection[0])) + 1u;

              for (unsigned long x = 0; x < x_size; ++x)
              {
                subdata[x] = casted_background_level;
              }
            }
          }
        }
        else if (plane->IsAbove(p2_t))
        {
          Point3D intersection;
          Line3D line;

          line.SetPoints(p1_t, p2_t);
          plane->IntersectionPoint(line, intersection);
          geometry->WorldToIndex(intersection, intersection);

          if (0.0f == this->m_BackgroundLevel)
          {
            unsigned long x = static_cast<unsigned long>(roundf(intersection[0]));

            memset(&data[x + (y * max_x) + (z * img_size)], 0, (max_x - x) * sizeof(TPixel));
          }
          else
          {
            unsigned long x = static_cast<unsigned long>(roundf(intersection[0]));
            TPixel *subdata = &data[x + (y * max_x) + (z * img_size)];
            const unsigned long x_size = max_x - x;

            for (x = 0; x < x_size; ++x)
            {
              subdata[x] = casted_background_level;
            }
          }
        }
      }
    }
  }
  else
  {
    for (unsigned long z = 0ul; z < max_z; ++z)
    {
      p1[2] = z;
      p2[2] = z;

      for (unsigned long y = 0ul; y < max_y; ++y)
      {
        p1[1] = y;
        p2[1] = y;

        Point3D p1_t, p2_t;

        geometry->IndexToWorld(p1, p1_t);
        geometry->IndexToWorld(p2, p2_t);

        if (!plane->IsAbove(p1_t))
        {
          if (!plane->IsAbove(p2_t))
          {
            if (0.0f == this->m_BackgroundLevel)
            {
              memset(&data[(y * max_x) + (z * img_size)], 0, max_x * sizeof(TPixel));
            }
            else
            {
              TPixel *subdata = &data[(y * max_x) + (z * img_size)];

              for (unsigned long x = 0; x < max_x; ++x)
              {
                subdata[x] = casted_background_level;
              }
            }
          }
          else
          {
            Point3D intersection;
            Line3D line;

            line.SetPoints(p1_t, p2_t);
            plane->IntersectionPoint(line, intersection);
            geometry->WorldToIndex(intersection, intersection);

            if (0.0f == this->m_BackgroundLevel)
            {
              memset(&data[(y * max_x) + (z * img_size)], 0, (static_cast<unsigned long>(roundf(intersection[0])) + 1u) * sizeof(TPixel));
            }
            else
            {

              TPixel *subdata = &data[(y * max_x) + (z * img_size)];
              const unsigned long x_size = static_cast<unsigned long>(roundf(intersection[0])) + 1u;

              for (unsigned long x = 0; x < x_size; ++x)
              {
                subdata[x] = casted_background_level;
              }
            }
          }
        }
        else if (!plane->IsAbove(p2_t))
        {
          Point3D intersection;
          Line3D line;

          line.SetPoints(p1_t, p2_t);
          plane->IntersectionPoint(line, intersection);
          geometry->WorldToIndex(intersection, intersection);

          if (0.0f == this->m_BackgroundLevel)
          {
            unsigned long x = static_cast<unsigned long>(roundf(intersection[0]));

            memset(&data[x + (y * max_x) + (z * img_size)], 0, (max_x - x) * sizeof(TPixel));
          }
          else
          {
            unsigned long x = static_cast<unsigned long>(roundf(intersection[0]));
            TPixel *subdata = &data[x + (y * max_x) + (z * img_size)];
            const unsigned long x_size = max_x - x;

            for (x = 0; x < x_size; ++x)
            {
              subdata[x] = casted_background_level;
            }
          }
        }
      }
    }
  }
}
