function uninstall_PicoScenes_MATLAB_Toolbox
%uninstall_PicoScenes_MATLAB_Toolbox Unnstall PicoScenes MATLAB Toolbox (PMT) from your MATLAB path. Don't worry! No files is touched during uninstallation.

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

disp('PicoScenes MATLAB Toolbox (PMT) has been removed from the MATLAB search path.');