# Changelog of PicoScenes MATLAB Toolbox (PMT)

## Jan. 18, 2022
    * [Rxs-Parsing-Core] regular update

## Dec. 28, 2021
    * [Rxs-Parsing-Core] regular update

## Dec. 16, 2021
    * [Toolbox] Fix bug in startup.m; exclude .git from search path

## Dec. 14, 2021
    * [Rxs-Parsing-Core] regular update
    * [Parser] Fix bug in ExtraInfo parsing for QCA9300

## Dec. 9, 2021
    * [Parser] Fix typo error for MVMExtra.FTMClock

## Nov. 4, 2021
    * [Rxs-Parsing-Core] regular update

## Dec. 3, 2021
    * [Parser] fix bug in parseRXSBundle.m

## Nov. 26, 2021
    * [Rxs-Parsing-Core] decode ExtraInfo temperature

## Nov. 22, 2021
    * [Rxs-Parsing-Core] Fix bug in CSISegment v4

## Nov. 18, 2021
    * [Update] Remove old RXSParser during update

## Nov. 13, 2021
    * [Parser] Update to the latest Rxs-Parsing-Core structure; rename all MATLAB variable to CamelCase style

## Nov. 12, 2021
    * [Rxs-Parsing-Core] Apt to v4 CSI parser, add firmware version output

## Nov. 10, 2021
    * [Rxs-Parsing-Core] regular update, W/A firmware bug for CSI extraction in the VHT 160 MHz case

## Nov. 4, 2021
    * [Rxs-Parsing-Core] regular update
## Oct. 27, 2021
    * Optimize the auto-update logic
    * Fix bug in startup

## Oct. 25, 2021
    * [Rxs-Parsing-Core] regular update

## Oct. 24, 2021
    * [Rxs-Parsing-Core] regular update

## Oct. 9, 2021
    * [Rxs-Parsing-Core] regular update

## Oct. 7, 2021
    * [Rxs-Parsing-Core] Workaround the Intel 67 firmware bug

## Oct. 6, 2021
    * Mute unnecessary warnings during the launch
    * Performance improved for the Intel-MVM CSI data structure 
    * [Rxs-Parsing-Core] Add *skipPilotSubcarriers* for fromIWLMVM method
    * [Rxs-Parsing-Core] Workaround a firmware bug with HE/160MHz 


## Oct. 4, 2021
    * Fix bug in install_PicoScenes_MATLAB_Toolbox.m script
    * Add uninstallation for PicoScenes MATLAB Toolbox via *install_PicoScenes_MATLAB_Toolbox('uninstall')*
    * Add path display in startup.p
    * Add auto-update functionality to startup
    * Remove batch_loader_app from public release
    * Optimize backup strategy

## Oct. 3, 2021
    * [RxS-Parsing-Core] update parsing_core
    * Add changelog
    * Adding PMT update protocol