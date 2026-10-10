# BlueParrot XCP documentation source

This directory contains the Sphinx source for the CXCP documentation. The
project-level introduction, supported build paths, repository map, and license
are in the [root README](../README.md).

Build the HTML documentation on Windows from the repository root:

```powershell
Push-Location docs
.\make.bat html
Pop-Location
```

The generated site is written to `docs/build/html`. The Sphinx reference
structure begins in `index.rst`; it covers building supplied targets,
integrating an XCP slave, configuration, application hooks, and the public
interface.

The Doxygen configuration is retained in `doxyfile` for an optional local API
site. It is not invoked by Sphinx:

```powershell
Push-Location docs
doxygen doxyfile
Pop-Location
```
