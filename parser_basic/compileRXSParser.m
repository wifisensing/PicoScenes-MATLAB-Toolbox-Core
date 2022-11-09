function compileRXSParser(skipExtraParam)

if nargin == 0
    skipExtraParam = false;
end

disp('Compiling RXSParser.cxx, the MATLAB parser for PicoScenes .csi file ...');
currentDir = pwd;

cd(fileparts(which(mfilename))); % change the MATLAB working directory to the folder of this file.
cd ../rxs_parsing_core;
direcory = [pwd filesep '*.cxx'];
allFiles = dir(direcory);
[cppNum,~] = size(allFiles);
if cppNum <= 0
    cd (currentDir);
    error(['There are no files in directory ' pwd ',please go to <a href = "https://gitlab.com/wifisensing/rxs_parsing_core ">https://gitlab.com/wifisensing/rxs_parsing_core </a>to download files']);
end
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
