# PicoScenes MATLAB Toolbox Core part (PMT-Core)

## What is this?

This repo holds the open-sourced part of the larger PicoScenes MATLAB Toolbox (PMT).

## How to obtain? No Download! Git clone this repo, RECURSIVELY!

The [RXS-Parsing-Core](https://github.com/wifisensing/RXS-Parsing-Core) library is *submoduled* in this repo. Direct download doesn't download the submodule automatically. The correct way to get the repo is to **git-clone the repo with --recursive option** like

```git clone https://github.com/wifisensing/PicoScenes-MATLAB-Toolbox-Core --recursive```

## Delete parser_basic/RXSParser.mexXXXX file every time after upgrade!

PMT-Core does not auto-build parser_basic/RXSParser.mexXXXX when the C++ code changes. To make the .mex file sync with the C++ code, delete the .mex file and let PMT-Core to rebuild it.

## Documentation?

The document of PMT-Core is at <https://ps.zpj.io/matlab.html>.

## No Issue Tracker for this repo

All PicoScenes related issues should be raised in the dedicated [PicoScenes Issue Tracker](https://github.com/wifisensing/PicoScenes-Issue-Tracker/issues).