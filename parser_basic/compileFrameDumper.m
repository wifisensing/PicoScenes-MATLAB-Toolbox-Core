function compileFrameDumper(skipExtraParam)
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
    
    try
        eval(['mex -silent -DBUILD_WITH_MEX CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3" -I../rxs_parsing_core MFrameDumper.cxx ../rxs_parsing_core/*.cxx ../rxs_parsing_core/preprocess/generated/*.cpp ' extraParam]);
        disp('Compilation done!');
    catch 
        warning('Exception caught! Use verbose mode to build again.');
        eval(['mex -v -DBUILD_WITH_MEX CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3" -I../rxs_parsing_core MFrameDumper.cxx ../rxs_parsing_core/*.cxx ../rxs_parsing_core/preprocess/generated/*.cpp ' extraParam])
    end
    
    cd(currentDir);
end

function checkRXSParsingCoreExists(currentDir)
    cd([fileparts(which(mfilename)) filesep '..' filesep 'rxs_parsing_core']);
    parserDir = pwd;
    allFiles = dir([pwd filesep '*.cxx']);
    if isempty(allFiles)
        cd (currentDir);
        error(['The [' parserDir '] directory is empty! That means you have NOT cloned the PicoScenes MATALB Toolbox RECURSIVELY. Please refer to page <a href = "https://gitlab.com/wifisensing/PicoScenes-MATLAB-Toolbox-Core">PicoScenes MATLAB Toolbox Core</a> on how to clone the toolbox correctly.']);
    end
end
