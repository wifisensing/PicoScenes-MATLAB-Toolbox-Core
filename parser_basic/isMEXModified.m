function result = isMEXModified

originPath = pwd;
targetHashStr = '';
path2Script = which(mfilename);
scriptDirectory = fileparts(path2Script);
cd(scriptDirectory);

hashStr = outputHash('RXSParser.cxx');
targetHashStr = strcat(targetHashStr,hashStr);

cd ../rxs_parsing_core/;
CXXFileList = dir('*.cxx');
numCXXFile = length(CXXFileList);
for i = 1 : numCXXFile
    cxxHashStr = outputHash(CXXFileList(i).name);
    targetHashStr = strcat(targetHashStr,cxxHashStr);
end

HXXFileList = dir('*.hxx');
numHXXFile= length(HXXFileList);
for i = 1 : numHXXFile
    
    hxxHashStr = outputHash(HXXFileList(i).name);
    targetHashStr = strcat(targetHashStr,hxxHashStr);
end

cd interpolationAndCSDRemoval/generated/;
genFileList = dir;
numGenFile = length(genFileList);
for i = 1 : numGenFile
    if strcmp(genFileList(i).name,'.') || strcmp(genFileList(i).name,'..')
        continue;
    end
   
    genHashStr = outputHash(genFileList(i).name);
    targetHashStr = strcat(targetHashStr,genHashStr);
end

finalHashStr = mlreportgen.utils.hash(targetHashStr);

cd(scriptDirectory);
mexFilePath = [userpath filesep 'mex.flag'];
if isfile(mexFilePath)
    mexFileID = fopen(mexFilePath,'r');
    originHashStr = fgetl(mexFileID);
    fclose(mexFileID);
    if strcmp(originHashStr,finalHashStr)
        result = false;
    else
        writeMEXFile(mexFilePath,finalHashStr);
        result = true;
    end
else
   writeMEXFile(mexFilePath,finalHashStr);
   result = false;

end
cd(originPath);
return;
end

function hashResult = outputHash(path)

inputFileID = fopen(path,'r');
inputFileContent = fread(inputFileID,'char=>char');
fclose(inputFileID);
hashResult = mlreportgen.utils.hash(inputFileContent);
end

function writeMEXFile(path,content)

mexFileID = fopen(path,'wt');
fprintf(mexFileID,'%s',content);
fclose(mexFileID);
end
