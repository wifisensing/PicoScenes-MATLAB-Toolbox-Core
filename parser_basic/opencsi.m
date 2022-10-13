function [bundle, bundleValidName] = opencsi(filePath)

[~,fileName,extension] = fileparts(filePath);
bundleValidName = matlab.lang.makeValidName(fileName);

disp(['Start parsing PicoScenes CSI file: ' fileName extension]);

cells = read_rxs_log(filePath);
try
    bundle = parseRXSBundle(cells);
    if numel(bundle) == 1
        bundle{1}.BundleName = bundleValidName;
    else
        if numel(bundle) == 2
            bundle{1}.BundleName = [bundleValidName '.backward'];
            bundle{2}.BundleName = [bundleValidName '.forward'];
        end
    end
    mergeAcrossCells = true;
catch
    warning('off','backtrace');
    warning(['The bundled parsing stage of [' fileName extension '] fails. The extracted data returns in the basic cell form. See the document <a href = "https://ps.zpj.io/matlab.html#structures-of-the-picoscenes-tx-and-rx-frames">The Raw Parsing & Bundled Parsing</a> for more information.'])
    warning('on','backtrace');
    bundle = cells;
    mergeAcrossCells = false;
end

if ~isempty(bundle) && mergeAcrossCells && exist('RXSBundle','class') && exist('RTRXSBundle','class')
    try
        bundle = cellfun(@(x) RXSBundle(x), bundle, 'UniformOutput', true);
        if numel(bundle) == 2
            bundle = RTRXSBundle(bundle);
        end
    catch
        warning('failed to convert raw bundles to RXSBundle. The extracted data returns in the merged format.');
    end
end

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