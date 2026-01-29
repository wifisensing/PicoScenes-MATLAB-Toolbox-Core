function compileRXSParser(skipExtraParam)
    if nargin == 0
        skipExtraParam = false;
    end
    
    currentDir = pwd;
    checkRXSParsingCoreExists(currentDir);    
    disp('Compiling the MATLAB parser for PicoScenes .csi file ...');

    cd(fileparts(which(mfilename))); 
    
    extraParam = '';
    if ~skipExtraParam && exist([pwd filesep '..' filesep '..' filesep 'utils'], 'dir')
        extraParam = fileread([pwd filesep '..' filesep '..' filesep 'utils' filesep 'compileRXSParserExtraParam']);
    end
    

    commonFlags = '-DBUILD_WITH_MEX -I../rxs_parsing_core';
    sources = 'RXSParser.cxx ../rxs_parsing_core/*.cxx ../rxs_parsing_core/preprocess/generated/*.cpp';
    
    if ispc
        % Windows 
        osSpecificFlags = '-DRXSPARSINGCORE_STATIC -DRXS_PARSING_CORE_STATIC -DRXS_PARSING_CORE_EXPORTS';
        cxxFlags = 'CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3"';
    else
        % Ubuntu/macOS 
        osSpecificFlags = '';
        cxxFlags = 'CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3"';
    end

    % 组合所有参数
    cmdParams = sprintf('%s %s %s %s %s', commonFlags, osSpecificFlags, cxxFlags, sources, extraParam);

    try
        eval(['mex -silent ' cmdParams]);
        disp('Compilation done!');
    catch 
        warning('Exception caught! Use verbose mode to build again.');
        eval(['mex -v ' cmdParams]);
    end
    
    cd(currentDir);
end

function checkRXSParsingCoreExists(currentDir)
    cd([fileparts(which(mfilename)) filesep '..' filesep 'rxs_parsing_core']);
    parserDir = pwd;
    allFiles = dir([pwd filesep '*.cxx']);
    if isempty(allFiles)
        cd (currentDir);
        error(['The [' parserDir '] directory is empty! That means you have NOT cloned the PicoScenes MATLAB Toolbox RECURSIVELY. Please refer to page <a href = "https://github.com/wifisensing/PicoScenes-MATLAB-Toolbox-Core">PicoScenes MATLAB Toolbox Core</a> on how to clone the toolbox correctly.']);
    end
end