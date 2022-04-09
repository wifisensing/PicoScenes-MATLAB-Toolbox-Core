function install_PicoScenes_MATLAB_Toolbox
%install_PicoScenes_MATLAB_Toolbox Install PicoScenes MATLAB Toolbox (PMT) into your MATLAB path for this and later session.

    startup;
    pathFile = [userpath filesep 'PicoScenes-MATLAB-Toolbox-Path.txt'];
    fid = fopen(pathFile);
    toolSetPath = fgetl(fid);
    fclose(fid);
    disp(['PicoScenes MATLAB Toolbox (PMT) at [' toolSetPath '] is installed!']);