# MITK ROI {#MITKROIPage}

[TOC]

## Disclaimer

Until the MITK ROI file format is going to be officially announced in a 2024 release of MITK, the file format must be considered experimental and is prone to change without any prior warning.

## Overview

MITK ROI is a JSON-based file format defining a collection of region of interests (ROIs).

ROIs must have an ID (unsigned integer) and their shape is currently considered to be an axis-aligned bounding box.
Its bounds are defined by minimum and maximum index coordinates, typically relative to an image.
Custom properties of various known types can be optionally attached to a ROI.
A few of these properties are used by MITK to define the appearance of a rendered ROI, for example:

  - "color" (mitk::ColorProperty): Color/RGB triplet of the rendered ROI (default: white \[1.0, 1.0, 1.0\])
  - "opacity" (mitk::FloatProperty): Opacity of the rendered ROI (default: 100% \[1.0\])
  - "lineWidth" (mitk::FloatProperty): Line width of the egdes of the rendered ROI (default: 1px \[1.0\])

ROIs can be optionally time-resolved and define both coordinates and properties per time step, allowing for a dynamic appearance, position, and size over time.

ROIs also display a caption at their bottom-left corner (supporting multiple lines), that can be set once per MITK ROI file for all contained ROIs.
Placeholders enclosed by braces in the caption are substituted by their corresponding ROI property values at runtime.
The default caption is "{name} ({ID})", where ``{ID}`` is a special placeholder for the ID of a ROI (technically not a ROI property), and ``{name}`` refers to the ROI property "name" (typically an mitk::StringProperty).

Last but not least the reference (image) geometry of the ROIs in an MITK ROI file must be specified to be able to map all index coordinates to actual world coordinates.
A geometry is defined by an origin, the pixel/voxel spacing, a size, and optionally the number of time steps in case of a time-resolved MITK ROI file.

## File format

As all features are explained in the overview above, the JSON-based file format is defined here by two examples with minimal additional notes: one example for a static MITK ROI file and one example for a time-resolved MITK ROI file.

### Static MITK ROI file

This example contains two ROIs for detected tumors in an image with certain confidence.
Names and confidence values will be displayed in separate lines for each ROI.

~~~{.json}
{
  "FileFormat": "MITK ROI",
  "Version": 1,
  "Name": "Static example",
  "Caption": "{name}\nConfidence: {confidence}",
  "Geometry": {
    "Origin": [0, 0, 0],
    "Spacing": [1, 1, 3],
    "Size": [256, 256, 49]
  },
  "ROIs": [
    {
      "ID": 0,
      "Min": [4, 4, 1],
      "Max": [124, 124, 31],
      "Properties": {
        "StringProperty": {
          "name": "tumor",
          "comment": "Detected a tumor with 95% confidence.",
          "note": "Properties are grouped by their type to reduce verbosity."
        },
        "ColorProperty": {
          "color": [0, 1, 0]
        },
        "FloatProperty": {
          "confidence": 0.95
        }
      }
    },
    {
      "ID": 1,
      "Min": [132, 4, 1],
      "Max": [252, 60, 15],
      "Properties": {
        "StringProperty": {
          "name": "Another tumor",
          "comment": "Maybe another tumor (confidence only 25%)."
        },
        "ColorProperty": {
          "color": [1, 0, 0]
        },
        "FloatProperty": {
          "confidence": 0.25
        }
      }
    }
  ]
}

~~~

Further hints:

  - "FileFormat" ("MITK ROI"), "Version" (1), and "Geometry" are mandatory.
  - "Name" is optional. If not set, the file name is used by MITK instead.
  - ROIs are defined by JSON objects in the "ROIs" JSON array.
  - See the derived classes of mitk::BaseProperty for an overview of known property types.

### Time-resolved MITK ROI file

This example only contains a single ROI but it is defined for several time steps.
Fallbacks of time step properties to default properties are demonstrated as well.

~~~{.json}
{
  "FileFormat": "MITK ROI",
  "Version": 1,
  "Name": "Time-resolved example",
  "Geometry": {
    "Origin": [0, 0, 0],
    "Spacing": [1, 1, 3],
    "Size": [256, 256, 49],
    "TimeSteps": 3
  },
  "ROIs": [
    {
      "ID": 0,
      "Properties": {
        "ColorProperty": {
          "color": [1, 0, 0]
        },
        "StringProperty": {
          "name": "Color-changing ROI"
        }
      },
      "TimeSteps": [
        {
          "t": 0,
          "Min": [4, 4, 1],
          "Max": [124, 124, 31]
        },
        {
          "t": 2,
          "Min": [14, 14, 11],
          "Max": [121, 121, 28],
          "Properties": {
            "ColorProperty": {
              "color": [0, 1, 0]
            }
          }
        }
      ]
    }
  ]
}
~~~

Further hints:

  - The geometry defines 3 time steps.
  - The "Properties" directly in the ROI function as fallbacks, if they are not defined for a certain time step.
  - Time time step indices "t" are mandatory. The ROI is only present at time steps 0 and 2.
  - The ROI is red (fallback) at time step 0 and green at time step 2.
  - Its extents are larger at time step 0 than at time step 2.
