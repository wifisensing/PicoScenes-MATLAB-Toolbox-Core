function [bundle, bundleValidName] = opencsi(filePath)

[bundle, bundleValidName] = parseCSIFile(filePath);

assignin('base', bundleValidName, bundle);
assignin('base', 'latest', bundle);
disp(bundleValidName);

currentDir = pwd;
cd(fileparts(which(mfilename))); % change the MATLAB working directory to the folder of this file.
path2ProtocolHandler = [pwd filesep '..' filesep '..' filesep 'utils' filesep 'ProtocolHandler'];
if exist(path2ProtocolHandler, 'dir')
    protocolList = dir([path2ProtocolHandler filesep '*.m'])';
    for file = protocolList
        [~, mName] = fileparts(file.name);
        evalin('base', [mName '("' bundleValidName '")']);
    end
end
cd(currentDir);

if nargout == 0
    bundle = [];
end