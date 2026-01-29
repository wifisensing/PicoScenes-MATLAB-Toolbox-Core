function compileRXSParser(skipExtraParam)
    if nargin == 0
        skipExtraParam = false;
    end
    
    currentDir = pwd;
    checkRXSParsingCoreExists(currentDir);    
    disp('Compiling the MATLAB parser for PicoScenes .csi file ...');
    
    % 切换到脚本所在目录
    cd(fileparts(which(mfilename))); 
    
    extraParam = '';
    if ~skipExtraParam && exist([pwd filesep '..' filesep '..' filesep 'utils'], 'dir')
        extraParam = fileread([pwd filesep '..' filesep '..' filesep 'utils' filesep 'compileRXSParserExtraParam']);
    end
    
    % --- 核心修改部分：根据 OS 构建参数 ---
    commonFlags = '-DBUILD_WITH_MEX -I../rxs_parsing_core';
    sources = 'RXSParser.cxx ../rxs_parsing_core/*.cxx ../rxs_parsing_core/preprocess/generated/*.cpp';
    
    if ispc
        % Windows 专用宏定义
        osSpecificFlags = '-DRXSPARSINGCORE_STATIC -DRXS_PARSING_CORE_STATIC -DRXS_PARSING_CORE_EXPORTS';
        % MSVC 默认支持 C++20 往往需要 /std:c++20，但 mex 内部会自动映射
        cxxFlags = 'CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3"';
    else
        % Ubuntu/macOS 
        osSpecificFlags = '';
        cxxFlags = 'CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3"';
    end

    cmdBase = sprintf('mex %s %s %s %s %s', commonFlags, osSpecificFlags, cxxFlags, sources, extraParam);

    try
        fprintf('Running: %s\n', ['mex -silent ' cmdBase]);
        eval(['mex -silent ' cmdBase]);
        disp('Compilation done!');
    catch 
        warning('Exception caught! Use verbose mode to build again.');
        eval(['mex -v ' cmdBase]);
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