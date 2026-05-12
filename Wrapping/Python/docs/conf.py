"""Sphinx configuration for the mitk Python wheel documentation."""

from __future__ import annotations

import importlib.metadata

# -- Project information -----------------------------------------------------

project = "mitk"
copyright = "German Cancer Research Center (DKFZ)"  # noqa: A001
author = "MITK developers"

try:
    release = importlib.metadata.version("mitk")
    version = ".".join(release.split(".")[:2])
except importlib.metadata.PackageNotFoundError:
    release = "unknown"
    version = "unknown"

# -- General configuration ---------------------------------------------------

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx.ext.intersphinx",
    "sphinx_autodoc_typehints",
    "myst_parser",
    "sphinx_copybutton",
]

source_suffix = {
    ".md": "markdown",
    ".rst": "restructuredtext",
}

exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- autodoc / autosummary ---------------------------------------------------

autosummary_generate = True
autodoc_default_options = {
    "members": True,
    "inherited-members": True,
    "show-inheritance": True,
    "undoc-members": False,
}
autodoc_typehints = "description"
autodoc_typehints_format = "short"
always_use_bars_union = True

# -- napoleon (Google-style docstrings) --------------------------------------

napoleon_google_docstring = True
napoleon_numpy_docstring = False
napoleon_use_admonition_for_examples = True
napoleon_use_admonition_for_notes = True
napoleon_use_param = True
napoleon_use_rtype = True
# napoleon_include_init_with_doc is intentionally left at its default
# (False). Setting it True duplicates the __init__ entry because the
# autosummary class template already lists __init__ via :special-members:.

# -- MyST settings -----------------------------------------------------------

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "fieldlist",
    "linkify",
    "smartquotes",
    "substitution",
    "tasklist",
]
myst_heading_anchors = 3

# -- intersphinx -------------------------------------------------------------

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
    "numpy": ("https://numpy.org/doc/stable/", None),
}

# -- HTML output -------------------------------------------------------------

html_theme = "sphinx_book_theme"
html_static_path = ["_static"]
html_title = f"mitk {release}"
html_theme_options = {
    "show_toc_level": 2,
    "use_download_button": False,
    "use_fullscreen_button": False,
    "home_page_in_toc": True,
    "extra_footer": (
        '<p>This is the Python documentation for the '
        '<a href="https://docs.mitk.org/latest/">Medical Imaging Interaction '
        'Toolkit (MITK)</a>. For the full C++ reference, the developer manual, '
        'and the user manual, see '
        '<a href="https://docs.mitk.org/latest/">docs.mitk.org/latest/</a>.</p>'
    ),
}

# -- copybutton --------------------------------------------------------------

copybutton_prompt_text = r">>> |\.\.\. |\$ "
copybutton_prompt_is_regexp = True
