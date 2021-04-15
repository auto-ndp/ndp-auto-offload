# Automatic and transparent offloading of computation to near-data processing units

Code used in my PhD project at the University of Edinburgh.

# Folders

 - `memspeed` - Measuring memory access latency and speed across tested devices
 - `ndoffloadsim` - Scheduling simulation of the impact of offloading subtasks to storage (and other kinds of) nodes
 - `pin-NearMAP` - a PIN tool

# Third-party symlinks setup

To use this repo, you should set up symlinks at:
 - `third-party/pin3.18` pointing at a Intel PIN 3.18 linux install root (with subdirs doc, extras, source, ...)
 - `third-party/spec2017` pointing at a SPEC 2017 linux install root (with subdirs benchspec, bin, config, tools, ...)
