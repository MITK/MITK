# MITK Segmentation Task Lists {#MITKSegmentationTaskListsPage}

[TOC]

## Overview

MITK Segmentation Task Lists are a JSON-based file format defining a list of segmentation tasks.
Segmentation tasks consist at least of a path to a reference image and a unique result path.
The result path specifies where the final segmentation of the task is expected to be located once it is done.

Optional properties of a segmentation task include a task name and description as well as various degrees of start conditions for the segmentation like a label name, a list of suggested names and colors for new labels, a label set preset, or even a pre-segmentation to begin with.
The complete set of properties is specified further below in the file format specification.

MITK Segmentation Task Lists must be considered experimental at the moment and are prone to change without any prior warning.
They are currently supported only in the MITK FlowBench application where a dedicated widget for task navigation and management appears if an MITK Segmentation Task List has been opened.

## File format

MITK Segmentation Task Lists are JSON files containing a JSON object as root, which must contain the two mandatory properties `FileFormat` and `Version`:

~~~{.json}
{
  "FileFormat": "MITK Segmentation Task List",
  "Version": 1
}
~~~

We also recommend to specify an optional `Name` that is used in the application if present instead of the plain filename of the JSON file:

~~~{.json}
{
  "FileFormat": "MITK Segmentation Task List",
  "Version": 1,
  "Name": "My First Task List"
}
~~~

### Tasks

The root object must also contain a mandatory `Tasks` array containing JSON objects that specify the individual tasks of the task list.
A minimum task object must consist of `Image` and `Result` file paths.
`Image` is a path to the reference image for the task and `Result` is a path where the final segmentation is expected to be stored once it is done.
Paths can be absolute or relative to the MITK Segmentation Task List file.

~~~{.json}
{
  "FileFormat": "MITK Segmentation Task List",
  "Version": 1,
  "Tasks": [
    {
      "Image": "images/input.nrrd",
      "Result": "results/output.nrrd"
    }
  ]
}
~~~

In addition, tasks can define various optional properties that mainly specify the start conditions for a segmentation:

- `Name` (*string*): A name for the task.
- `Description` (*string*): A short description/definition of the task.
- `LabelName` (*string*): The name of the first label in a new segmentation that is created for the task on the fly.
- `LabelNameSuggestions` (*file path*): A Label Suggestions JSON file specifying names and optional colors that are suggested to the user for new labels in the segmentation.
- `Preset` (*file path*): A Label Set Preset XML file in MITK's .lsetp file format. The preset is applied to a new segmentation that is created for the task on the fly. We recommend to use the Segmentation plugin to create such label set preset files as described in its [user guide](@ref org_mitk_views_segmentationlabelpresets).
- `Segmentation` (*file path*): A pre-segmentation that a user can start with or should refine.
- `Dynamic` (*boolean*): In case `Image` refers to a dynamic (3d+t) image, specifies whether the segmentation should be static (*false*), i.e. equal for all time steps, or dynamic (*true*), i.e. individual for each time step.

### Task defaults / common properties

If a task list contains multiple tasks with common properties, they do not have to be specified for each and every task again and again.
Instead, the root object may contain an optional `Defaults` object that is identical in format to the tasks specified above.
There is one exception, though: A `Defaults` object must not contain a `Result` file path, since result files of tasks must be distinct by definition.
As the name indicates, default properties can still be overridden by individual tasks if they are specified explicitly.

### Example

The following example is a complete showcase of the properties and features listed above.
It defines four tasks, three of which refer to the same reference image so it is specified as default.

Remember that the only task property required to be distinct is `Result` so you are pretty free in your task design.
For simplicity, we chose to define tasks around organs for this example and named the tasks accordingly:

~~~{.json}
{
  "FileFormat": "MITK Segmentation Task List",
  "Version": 1,
  "Name": "Example Segmentation Task List",
  "Defaults": {
    "Image": "images/Pic3D.nrrd"
  },
  "Tasks": [
    {
      "Name": "Liver",
      "LabelName": "Liver",
      "LabelNameSuggestions": "suggestions/label_suggestions.json",
      "Description": "This task provides an image and label name suggestions for new labels. The segmentation will start with an empty label named Liver.",
      "Result": "results/liver.nrrd"
    },
    {
      "Name": "Kidneys",
      "Description": "This task provides an image and a label set preset that is applied to the new segmentation.",
      "Preset": "presets/kidneys.lsetp",
      "Result": "results/kidneys.nrrd"
    },
    {
      "Name": "Spleen",
      "Description": "This task provides an image and an initial (pre-)segmentation.",
      "Segmentation": "segmentations/spleen.nrrd",
      "Result": "results/spleen.nrrd"
    },
    {
      "Name": "Surprise",
      "Description": "And now for something completely different. This task overrides the default Image and starts with an empty static segmentation for a dynamic image.",
      "Image": "images/US4DCyl.nrrd",
      "Result": "results/US4DCyl.nrrd",
      "Dynamic": false
    }
  ]
}
~~~

## Label name suggestions

The `LabelNameSuggestions` property of segmentation tasks is supposed to reference a JSON file that consists of an array of objects with a mandatory `name` property and an optional `color` property.
For example:

~~~{.json}
[
  {
    "name": "Abdomen",
    "color": "red"
  },
  {
    "name": "Lung",
    "color": "#00ff00"
  },
  {
    "name": "Heart"
  },
  {
    "name": "Aortic Valve",
    "color": "CornflowerBlue"
  }
]
~~~
