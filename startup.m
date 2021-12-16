function startup()
%STARTUP Add the directory containing this script to MATLAB path automatically.
%
% This script will perform the installation in the first-time use.
% After that, every time MATLAB boots up, the directory containing this script
% is added to path automatically.
    
    path2ThisScript = which(mfilename);
    toolSetPath = fileparts(path2ThisScript);
    originPath = pwd;
    isInFatherDirectory = false;
   
    % disp(['Current startup.m file path: ' path2ThisScript]);
    cd (toolSetPath);
    cd ../;
    if isfolder('SDR') && isfolder('@RXSBundle') && isfolder('@RTRXSBundle')
        toolSetPath = pwd;
        isInFatherDirectory = true;
    end
    
    if isfolder('PicoScenes-MATLAB-Toolbox-Core')
        cd PicoScenes-MATLAB-Toolbox-Core/;
    elseif isfolder('MATLAB')
        cd MATLAB/;
    end

    if strcmp(userpath, toolSetPath) == true
        isInUserPath = true;
    else
        isInUserPath = false;
    end
    
    if isInUserPath == false
        if isempty(userpath) == true % Create /Documents/MATLAB directory if not existent.
            if ispc == true
                dirname = [getenv('USERPROFILE') '/Documents/MATLAB'];
                if ~exist(dirname, 'dir')
                    mkdir(dirname);
                end
            else
                dirname = [getenv('HOME') '/Documents/MATLAB'];
                if ~exist(dirname, 'dir')
                    mkdir(dirname);
                end
            end
            userpath('reset');
        end
        
        copyfile(path2ThisScript, userpath);
        writePath(toolSetPath,isInFatherDirectory);
        toolSetPath = readToolSetPath();
        addpath(genpathExcludeGit(toolSetPath));
    end
    
    if isInUserPath == true
        [toolSetPath,isInFatherDirectory] = readToolSetPath();
        disp(['PicoScenes MATLAB Toolbox [' toolSetPath '] added to the MATLAB search path.']);
        addpath(genpathExcludeGit(toolSetPath));

        if(strcmp(isInFatherDirectory,'true'))
            startupScriptPathInToolSet = [readToolSetPath filesep 'PicoScenes-MATLAB-Toolbox-Core' filesep 'startup.m'];
        else
            startupScriptPathInToolSet = [readToolSetPath filesep 'startup.m'];
        end
        copyfile(startupScriptPathInToolSet, userpath);
       
    end
    cd (originPath);
    
end

function writePath(toolSetPath,isInFatherDirectory)
    targetPath = [userpath filesep 'PicoScenes-MATLAB-Toolbox-Path.txt'];
    fid = fopen(targetPath,'wt');
    fprintf(fid, '%s\n', toolSetPath);
    if(isInFatherDirectory)
        fprintf(fid,'%s','true');
    else
        fprintf(fid,'%s','false');
    end
    fclose(fid);
end

function [toolSetPath,isInFatherDirectory] = readToolSetPath()
    targetPath = [userpath filesep 'PicoScenes-MATLAB-Toolbox-Path.txt'];
    fid = fopen(targetPath,'r');
    toolSetPath = fgetl(fid);
    isInFatherDirectory = fgetl(fid);
    fclose(fid);
end

function gitFreePath = genpathExcludeGit(toolboxPath)
    allpath = genpath(toolboxPath);
    splitPath = split(allpath, pathsep);
    hasPattern = contains(splitPath, '.git');
    excluded = splitPath(~hasPattern);
    gitFreePath = char(strjoin(excluded, pathsep));
end
