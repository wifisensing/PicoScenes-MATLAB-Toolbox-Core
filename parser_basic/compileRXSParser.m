function compileRXSParser(skipExtraParam)
    if nargin == 0
        skipExtraParam = false;
    end
    
    checkRXSParsingCoreExists;    
    disp('Compiling RXSParser.cxx, the MATLAB parser for PicoScenes .csi file ...');
    currentDir = pwd;
    cd(fileparts(which(mfilename))); 
    
    extraParam = '';
    if ~skipExtraParam && exist([pwd filesep '..' filesep '..' filesep 'utils'], 'dir')
        extraParam = fileread([pwd filesep '..' filesep '..' filesep 'utils' filesep 'compileRXSParserExtraParam']);
    end
    
    try
        eval(['mex -silent -DBUILD_WITH_MEX CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3" -I../rxs_parsing_core RXSParser.cxx ../rxs_parsing_core/*.cxx ../rxs_parsing_core/preprocess/generated/*.cpp ' extraParam]);
        disp('Compilation done!');
    catch 
        warning('Exception caught! Use verbose mode to build again.');
        eval(['mex -v -DBUILD_WITH_MEX CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3" -I../rxs_parsing_core RXSParser.cxx ../rxs_parsing_core/*.cxx ../rxs_parsing_core/preprocess/generated/*.cpp ' extraParam])
    end
    
    cd(currentDir);
end

function checkRXSParsingCoreExists
    cd([fileparts(which(mfilename)) filesep '..' filesep 'rxs_parsing_core']);
    parserDir = pwd;
    allFiles = dir([pwd filesep '*.cxx']);
    if isempty(allFiles)
        cd (currentDir);
        error(['The [' parserDir '] directory is empty! That means you have NOT clone the PicoScenes MATALB Toolbox RECURSIVELY. Please refer to page <a href = "https://gitlab.com/wifisensing/PicoScenes-MATLAB-Toolbox-Core">PicoScenes MATLAB Toolbox Core</a> on how to clone the toolbox.']);
    end
end