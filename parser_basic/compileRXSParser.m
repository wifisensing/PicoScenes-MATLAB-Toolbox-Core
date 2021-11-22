function compileRXSParser

disp('Compiling RXSParser.cxx, the MATLAB parser for PicoScenes .csi file ...');
currentDir = pwd;
cd(fileparts(which(mfilename))); % change the MATLAB working directory to the folder of this file.
try
    mex -silent -DBUILD_WITH_MEX CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3" RXSParser.cxx ../rxs_parsing_core/*.cxx ../rxs_parsing_core/preprocessor/generated/*.cpp
catch 
    warning('Exception caught! Use verbose mode to build again.');
    mex -v -DBUILD_WITH_MEX CXXFLAGS="$CXXFLAGS -std=c++2a -Wno-attributes -O3" RXSParser.cxx ../rxs_parsing_core/*.cxx ../rxs_parsing_core/preprocessor/generated/*.cpp
end

cd(currentDir);