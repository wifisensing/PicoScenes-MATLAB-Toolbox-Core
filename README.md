# PicoScenes MATLAB Toolbox Core part (PMT-Core)

## What is this?

This repo holds the open-sourced part of the larger PicoSenes MATLAB Toolbox (PMT).

## How to obtain? CLONE this repo, RECURSIVELY!

This repo depends on the *RXS-Parsing-Core* library wit git submodule mechanism. You should clone this repo with **--recursive** option like

```git clone https://gitlab.com/wifisensing/PicoScenes-MATLAB-Toolbox-Core --recursive```

**Never download! Direct download results in missing the RXS-Parsing-Core library.**

## Remember to delete parser_basic/RXSParser.mexXXXX file everytime after upgrade!!!

    PMT-Core does not auto-build parser_basic/RXSParser.mexXXXX when the C++ code changes. To make the .mex file sync with the C++ code, delete the .mex file and let PMT-Core to rebuild it.

## Documentation?

The document of PMT is at <https://ps.zpj.io/matlab.html>.

## No Issue Tracker for this repo

All PicoScenes related issues should be raised in the dedicated [PicoScenes Issue Tracker](https://gitlab.com/wifisensing/picoscenes-issue-tracker/-/issues).
