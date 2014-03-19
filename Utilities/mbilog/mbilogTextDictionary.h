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

#ifndef _MBILOG_TEXTDICTIONARY_H_
#define _MBILOG_TEXTDICTIONARY_H_

namespace mbilog {
  /** \brief This is a dictionary to replace long names of classes,
    *        modules, etc. to shorter versions in the console output. */
  static const char *replace[] =
      {
        ".cpp",                "",
        ".cxx",                "",
        ".txx",                "",
        ".h",                  "",
        ".hpp",                "",
        ".hxx",                "",
        ".c",                  "",

        "org.blueberry.",      "",
        "org.mitk.gui.qt.",    "",
        "org.mitk.",           "",

        "qmitk",               "",
        "mitk",                "",
        "berry",               "",
        "itk",                 "",
        "vtk",                 "",
        "qt",                  "",

        "object",              "obj",
        "factory",             "fac",
        "classes",             "cls",
        "plugin",              "plg",
        "widget",              "wdgt",
        "interface",           "itf",
        "service",             "svc",
        "register",            "reg",
        "perspective",         "prs",

        "assessor",            "ase",

        "atrophy",             "atr",

        "bias",                "bias",

        "field",               "fld",

        "multi",               "mlt",

        "contour",             "cntr",
        "tools",               "tls",
        "tool",                "tl",

        "application",        "app",
        "calculate",          "calc",
        "subtract",           "sub",
        "region",             "reg",
        "tumor",              "tum",
        "growing",            "grow",
        "segmentation",       "seg",
        "statistics",         "stat",

        "imaging",            "img",
        "image",              "img",

        "diffusion",          "dif",
        "registration",       "reg",
        "navigation",         "nav",

        "generation",         "gen",
        "generator",          "gen",

        "vector",             "vec",
        "gradient",           "grad",
        "flow",               "flow",

        "paint",              "pnt",
        "brush",              "brsh",
        "volumetry",          "vol",
        "volume",             "vol",
        "mapper",             "map",
        "filter",             "flt",
        "surface",            "sfc",
        "point",              "pnt",
        "organ",              "org",
        "multiple",           "mlt",

        "corrector",          "cor",
        "correction",         "cor",

        "batch",              "bat",

        "window",             "wnd",

        "advisor",            "adv",

        "editor",             "edt",

        "material",           "mat",

        "visualization",      "vis",

        "measurement",        "mes",

        "scene",              "scn",

        "serialization",      "ser",
        "deserializer",       "dser",
        "serializer",         "ser",

        "sandbox",            "sb",
        "texture",            "tex",
        "opengl",             "ogl",
        "vessel",             "vsl",
        "value",              "val",
        "analysis",           "ana",

        "patient",            "pat",
        "body",               "body",
        "diagnosis",          "diag",
        "mesh",               "mesh",
        "radial",             "rad",
        "simple",             "smp",
        "algorithms",         "alg",
        "controllers",        "con",
        "control",            "con",
        "interactive",        "ia",
        "interactions",       "ia",

        "processing",         "pro",
        "process",            "pro",

        "rendering",          "rnd",
        "renderer",           "rnd",
        "render",             "rnd",

        "datamanagement",     "data",
        "management",         "mng",
        "manager",            "mng",
        "data",               "data",

        "anatomy",            "ana",
        "neuro",              "neo",
        "automatic",          "auto",

        "optimizer",          "opt",
        "optimize",           "opt",

        "binary",             "bin",
        "liver",              "liv",
        "lymph",              "lym",
        "node",               "node",
        "tree",               "tree",
        "homogeneous",        "hmgn",
        "threshold",          "tsh",

     //   "shapebased",         "shp",
        "based",              "bsd",
        "shape",              "shp",

        "model",              "mdl",
        "extension",          "ext",
        "activator",          "act",

        "dicom",              "dicom",

        "browser",            "brwr",

        "viewer",             "view",
        "view",               "view",

        "finder",             "fnd",

        "indexer",            "idx",
        "index",              "idx",

        "rapid",              "rpd",

        "gui",                "gui",

        "slices",             "slc",
        "slice",              "slc",

        "about",              "abt",

        "interpolator",       "inp",

        "switcher",           "swh",

        "planning",           "plan",
        "planner",            "plan",
        "plane",              "pln",
        "plan",               "plan",

        "workbench",          "wrkbnc",
        "common",             "com",
        "resection",          "rsc",
        "translation",        "trnsl",
        "rotation",           "rot",
        "deformation",        "dfrm",
        "shader",             "shd",
        "repository",         "rep",
        "initializer",        "init",
        "dialog",             "dlg",
        "download",           "down",
        "upload",             "up",
        "core",               "core",
        "manual",             "man",
        "leaf",               "leaf",
        "internal",           "int",
        "external",           "ext",
        "platform",           "pltfm",
        "method",             "mthd",
        "pyramidal",          "prmdl",
        "tracking",           "trck",
        "track",              "trck",

        "bspline",            "bspl",
        "spline",             "spl",

        "create",             "crt",
        "erase",              "ers",

        "auto",               "auto",
        "crop",               "crop",
        "file",               "file",
        "io",                 "io",

        "2d",                 "2d",
        "3d",                 "3d",
        ".",                  "."
  };
}

#endif
