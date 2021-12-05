% Call this script once to install ar_matlab into MATLAB Path.
function install_PicoScenes_MATLAB_Toolbox(uninstall)

if nargin == 0 
    startup;
    pathFile = [userpath filesep 'PicoScenes-MATLAB-Toolbox-Path.txt'];
    fid = fopen(pathFile);
    toolSetPath = fgetl(fid);
    fclose(fid);
    disp(['PicoScenes MATLAB Toolbox [' toolSetPath '] installed successfully!']);
elseif strcmp('uninstall', uninstall)
    userpath('reset');
    
    pathFile = [userpath filesep 'PicoScenes-MATLAB-Toolbox-Path.txt'];
    if exist(pathFile, 'file')
        delete(pathFile);
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