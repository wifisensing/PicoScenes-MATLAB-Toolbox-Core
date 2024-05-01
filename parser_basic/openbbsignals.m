function openbbsignals(filePath, varargin)

if isempty(varargin)
    lines2Skip = 0;
    lines2Read = inf;
else
    lines2Skip = varargin{1};
    lines2Read = varargin{2};
end

[~,fileName] = fileparts(filePath);
validName = matlab.lang.makeValidName(fileName);
bb_signal = loadBasebandSignalFile(filePath, lines2Skip, lines2Read);
assignin('base', validName, bb_signal);
disp(['Loaded variable name: ' validName]);

end

function data = loadBasebandSignalFile(bbFilePath, skipLines, totalLines2Read)
    fid = fopen(bbFilePath);
    fseek(fid, 0, 'eof');
    fileSize = ftell(fid);
    fseek(fid, 0, 'bof');

    fileHeader = fread(fid, 2, 'char=>char');
    bbFileVersion = fread(fid, 2, 'char=>char');
    if any(fileHeader' ~= 'BB') || (any(bbFileVersion' ~= 'v1') && any(bbFileVersion' ~= 'v2'))
        error(' ** incompatible .bbsignals file format! **');
    end

    numDimensions = fread(fid, 1, 'uint8=>double');
    dimensions = ones(1, numDimensions);
    if bbFileVersion(2) - 48 == 1
        for i = 1: numDimensions
            dimensions(i) = fread(fid, 1, 'int32=>double');
         end
    elseif bbFileVersion(2) - 48  == 2
        for i = 1: numDimensions
            dimensions(i) = fread(fid, 1, 'int64=>double');
         end
    end
    isComplexMatrix = double(fread(fid, 1, 'char=>double') == 'C');
    typeChar = fread(fid, 1, 'char=>double');
    typeBits = fread(fid, 1, 'uint8=>double');
    majority = SignalStorageMajority(fread(fid, 1, 'uint8=>double'));
    currentPos = ftell(fid);

    disp(['BBSignals: [' bbFilePath ']']);
    disp(['Signature: [bbVer=' bbFileVersion' ', complex=' num2str(isComplexMatrix) ', type=' char(typeChar) num2str(typeBits) ', major=' char(majority) ']']);
    rowBytes = (typeBits / 8) * 2^isComplexMatrix * prod(dimensions(2:end));
    rowsInFile = floor((fileSize - currentPos) / rowBytes);
    dimensions(1) = rowsInFile;
    disp(['Size: numDim=' num2str(numDimensions) ', dims=[' num2str(dimensions) '], fileSize=' num2str(fileSize/1e6) 'MB'])

    if isinf(totalLines2Read)
        totalLines2Read = rowsInFile - skipLines;
    end
    skipBytes = skipLines * rowBytes;
    fseek(fid, skipBytes, 'cof');

    if typeChar == 'D'
        precision = 'float';
    elseif typeChar == 'F'
        precision = 'float';
    elseif typeChar =='U'
        precision = 'uint';
    elseif typeChar == 'I'
        precision = 'int';
    elseif typeChar == 'L'
        precision = 'uint';
    end
    precision = [precision num2str(typeBits)];
    precision = [precision '=>double'];

    if isComplexMatrix
        data = coder.nullcopy(zeros(totalLines2Read, 1, 'like', complex(0)));
    else
        data = coder.nullcopy(zeros(totalLines2Read, 1));
    end

    readLines = 0;
    stepLimit = 1e8;
    while readLines < totalLines2Read
        if readLines + stepLimit < totalLines2Read
            step = stepLimit;
        else
            step = totalLines2Read - readLines;
        end
        stepBytes2Read = step * 2^isComplexMatrix * prod(dimensions(2:end));
        temp = fread(fid, stepBytes2Read, precision);
        
        if typeChar == 'I' && typeBits == 16
            temp = temp / 32768;
        elseif typeChar == 'I' && typeBits == 8
            temp = temp / 256;
        end
    
        if isComplexMatrix
            temp = complex(temp(1:2:end), temp(2:2:end)); % slower but save memory
        end

        data(readLines + 1 : readLines + step) = temp;
        readLines = readLines + step;
    end
    

    if majority == SignalStorageMajority.ColumnMajor && dimensions(2) > 1
        data = reshape(data, dimensions);
    elseif majority == SignalStorageMajority.RowMajor && dimensions(2) > 1
        data = reshape(data, flip(dimensions));
        data = ipermute(data, numel(dimensions):-1:1);
    end
end