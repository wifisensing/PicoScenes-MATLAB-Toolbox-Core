% Call this script once to install ar_matlab into MATLAB Path.
function install_PicoScenes_MATLAB_Toolbox(uninstall)

if nargin == 0 
    startup;
    path2ThisScript = which(mfilename);
    toolSetPath = fileparts(path2ThisScript);
    dot_verison_content = fileread([toolSetPath filesep '.version']);
    lines = splitlines(dot_verison_content);
    current_version_string = lines{1};
    if which('package_n_install_batchloader')
        package_n_install_batchloader;
    end
    disp(['PicoScenes MATLAB Toolbox [' toolSetPath ' ver.' current_version_string '] installed successfully!']);
elseif strcmp('uninstall', uninstall)
    userpath('reset');
    
    pathFile = [userpath filesep 'PicoScenes-MATLAB-Toolbox-Path.txt'];
    if exist(pathFile, 'file')
        delete(pathFile);
    end
    
    startupFile = [userpath filesep 'startup.p'];
    if exist(startupFile, 'file')
        delete(startupFile);
    end
    
    startupMFile = [userpath filesep 'startup.m'];
    if exist(startupMFile, 'file')
        delete(startupMFile);
    end
    
    a = matlab.apputil.getInstalledAppInfo;
    if ~isempty(a)
        searchResult = cell2mat(strfind({a.id}, 'CSIFileBatchLoader'));
        for i = 1:numel(searchResult)
            if searchResult(i)
                matlab.apputil.uninstall(a(i).id);
            end
        end
    end
    
    disp('PicoScenes MATLAB Toolbox has been removed from the MATLAB search path.');
end