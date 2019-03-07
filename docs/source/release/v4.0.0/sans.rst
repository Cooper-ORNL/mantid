============
SANS Changes
============

.. contents:: Table of Contents
   :local:

New
###
* Added manual saving functionality to the new GUI.

- :ref:`SANSILLReduction <algm-SANSILLReduction>` performs SANS data reduction for ILL instruments D11, D22, D33.
- :ref:`SANSILLIntegration <algm-SANSILLIntegration>` performs integration of corrected SANS data to produce I(Q), I(Phi,Q) or I(Qx,Qy).

ISIS SANS Interface
-------------------

Improved
########
* Updated workspace naming scheme for new backend.
* Added shortcut keys to copy/paste/cut rows of data.
* Added shortcut keys to delete or add rows.
* Added tabbing support to table.
* Added error notifications on a row by row basis.
* Updated file adding to prefix the instrument name.
* Updated file finding to be able to find added runs without instrument name prefix.
* Updated GUI code so calculated merge scale and shift are shown after reduction.
* Removed instrument selection box. Instrument is now determined by user file.
* Automatically remembers last loaded user file.
* Added display of current save directory.
* Added a "process all" and "process selected" button to the batch table in place of "process" button.
* Added a load button to load selected workspaces without processing.
* Added save_can option to output unsubtracted can and sample workspaces.
* Added transmission sample and can data to XML and H5 files when provided.
* Added transmission sample/can to unsubtracted sample/can XML and H5 files when "Save Can" option selected.
* Can set PhiMin, PhiMax, and UseMirror mask options from the options column in the table
* Autocomplete for Sample shape column in the table.
* Can export table as a csv, which can be re-loaded as a batch file.
* File path to batch file will be added to your directories automatically upon loading.
* If loading of user file fails, user file field will remain empty to make it clear it has not be loaded successfully.
* Workspaces are centred upon loading.
* All limit strings in the user file (L/ ) are now space-separable to allow for uniform structure in the user file. For backwards compatibility, any string which was comma separable remains comma separable as well.
* QXY can accept simple logarithmic bins. E.g. L/QXY 0.05 1.0 0.05 /LOG in the user file.
* Added a sum runs directory selection button. This is separate from default save directory and only determines where summed runs are saved.
* The gui will remember which output mode (Memory, File, Both) you last used and set that as your default when on the SANS interface.
* The gui will check *save can* by default if it was checked when the gui was last used.
* Default adding mode is set to Event for all instruments except for LOQ, which defaults to Custom.
* Removed the show transmission check box. Transmission workspaces are always added to output files regardless of output mode, and are always added to ADS if "memory" or "both" output mode is selected.
* The old ISIS SANS interface has been renamed *ISIS SANS (Deprecated)* to signify that it is not under active development.
* The v2 experimental interface has been renamed *ISIS SANS* to signify that it is the main interface and is under active development.

Bug fixes
#########
* Fixed an issue where the run tab was difficult to select.
* Changed the geometry names from CylinderAxisUP, Cuboid and CylinderAxisAlong to Cylinder, FlatPlate and Disc.
* The GUI now correctly resets to default values when a new user file is loaded.
* The GUI no longer hangs whilst searching the archive for files.
* Updated the options and units displayed in wavelength and momentum range combo boxes.
* Fixed a bug which crashed the beam centre finder if a phi mask was set.
* Removed option to process in non-compatibility mode to avoid calculation issues.
* GUI can correctly read user files with variable step sizes, in /LOG and /LIN modes.
* Fixed occasional crash when entering data into table.
* Fixed error message when trying to load or process table with empty rows.
* Removed option to process in non-compatibility mode to avoid calculation issues.
* Default name for added runs has correct number of digits.
* RKH files no longer append to existing files, but overwrite instead.
* Reductions with event slices can save output files. However, transmission workspaces are not included in these files.

Improvements
############

- :ref:`Q1DWeighted <algm-Q1DWeighted>` now supports the option of asymmetric wedges for unisotropic scatterer.

Removed
#######

- Obsolete *SetupILLD33Reduction* algorithm was removed.


ORNL SANS
---------

Improvements
############

- ORNL HFIR SANS instruments have new geometries. The monitors have now a shape associated to them. Detector will move to the right position based on log values.


:ref:`Release 4.0.0 <v4.0.0>`
