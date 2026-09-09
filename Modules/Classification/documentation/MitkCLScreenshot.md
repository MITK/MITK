# MitkCLScreenshot {#MitkCLScreenshotPage}

[TOC]

## Overview

MitkCLScreenshot renders one or more images into an off-screen 2D render window and saves every slice as a PNG file. All given files are loaded into a common data storage and displayed together, so segmentations can be overlaid on an intensity image. Images that MITK recognizes as binary get a distinct color from a fixed palette of ten colors; other images are rendered with their default appearance. The app is intended for creating quick visual checks of a segmentation, for example in batch processing pipelines.

Related apps: [MitkCLGlobalImageFeatures](@ref MitkCLGlobalImageFeaturesPage) can create similar screenshots of the analysed image and mask as a side product of feature extraction. This app is built with the CMake option `BUILD_ClassificationCmdApps` and is not part of the MITK Workbench installer.

## Usage

```bash
MitkCLScreenshot -i <image1;image2;...> -o <output prefix>
```

### Required arguments

| Argument | Short | Type | Description |
|----------|-------|------|-------------|
| `--image` | `-i` | Image | One or more image files separated by semicolons. All files are rendered together. |
| `--output` | `-o` | File | Prefix of the PNG files that are written, see Details. |

### Optional arguments

| Argument | Short | Type | Default | Description |
|----------|-------|------|---------|-------------|
| `--help` | `-h` | Flag | | Show the help text and exit. |

## Details

### Rendering

The images are loaded in the given order and added to a data storage that is shown in a single 2D render window with the default view direction (axial). The view is initialized to the bounding box of all loaded data. The render window is resized to 512 x 512 pixels and every screenshot is rendered with a magnification of 3, so the PNG files are 1536 x 1536 pixels.

A node is treated as a segmentation if its `binary` property is true. This property is set automatically when a node is created from an image that MITK detects as binary: a single-component image whose voxels take at most two distinct values (for example 0 and 1, but any two values qualify). Segmentations receive the color, hovering color and selected color of the palette entry `k mod 10`, where `k` counts the segmentations in the order of the input list: red, yellow, green, blue, orange, violet, turquoise, bright green, dark orange, pink.

### Output files

One PNG file is written per slice of the render window's slice navigation, that is for every slice of the combined bounding geometry along the view direction. The file names are `<prefix>screenshot_step-<n>.png` with `n` starting at 0. The prefix is used verbatim, no separator is inserted: `-o out/case01_` yields `out/case01_screenshot_step-0.png`, whereas `-o out/` yields `out/screenshot_step-0.png`. The directory is not created.

### Requirements and exit behaviour

The app creates a Qt application object and an OpenGL render window, so a Qt platform (a display or an offscreen platform plugin) must be available. 3D+t images are rendered at their first time step; the slice loop does not iterate over time.

The app returns 0 after writing the screenshots. Missing required arguments print the help text and return 1. Errors such as unreadable files raise exceptions that are not caught; the process terminates with the platform-specific code for an unhandled exception.

## Examples

### Screenshots of a single image

```bash
MitkCLScreenshot -i ct.nrrd -o shots/ct_
```

Writes one file per axial slice of `ct.nrrd` to `shots/ct_screenshot_step-0.png`, `shots/ct_screenshot_step-1.png`, and so on. The directory `shots` must exist.

### Overlay two segmentations on an image

```bash
MitkCLScreenshot -i "ct.nrrd;liver.nrrd;tumor.nrrd" -o shots/case01_
```

Renders the CT with the liver segmentation in red and the tumor segmentation in yellow. Quote the list so that the shell does not interpret the semicolons.

### Screenshots into the current directory

```bash
MitkCLScreenshot -i mr.nrrd -o ./
```

Writes `screenshot_step-<n>.png` into the current directory.
