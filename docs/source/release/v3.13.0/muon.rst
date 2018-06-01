============
MuSR Changes
============

.. contents:: Table of Contents
   :local:
   
Interface
---------

Improvements
############

Bug fixes
#########

- Results table can now detect sequential fits.
- Fit options are not disabled after changing tabs.
- The run number is now updated before the periods, preventing irrelevant warningsfrom being produced.
- In single fit the workspace can be changed.

Algorithms
----------

New
###
- :ref:`ConvertFitFunctionForMuonTFAsymmetry <algm-ConvertFitFunctionForMuonTFAsymmetry>` has been added to help convert fitting functions for TF asymmetry fitting.

Improvements
############

Bug fixes
#########
- :ref:`MuonMaxent <algm-MuonMaxent>` and :ref:`PhaseQuad <algm-PhaseQuad>`  no longer include dead detectors (zero counts) when calculating the frequency spectrum.
- :ref:`RemoveExpDecay <algm-RemoveExpDecay>` will not alter data from a dead detectors (zero counts).
- :ref:`CalMuonDetectorPhases <algm-CalMuonDetectorPhases>` will give an error code for dead detectors (zero counts) in the phase table.

:ref:`Release 3.13.0 <v3.13.0>`
