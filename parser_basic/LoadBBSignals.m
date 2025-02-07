function LoadBBSignals(filePath, startRatio, endRatio)

    [~,fileName] = fileparts(filePath);
    validName = matlab.lang.makeValidName(fileName);
    bb_signal = loadBasebandSignalFile(filePath, startRatio, endRatio);
    assignin('base', validName, bb_signal);
    disp(['Loaded variable name: ' validName]);

end

function data = loadBasebandSignalFile(bbFilePath, startRatio, endRatio)
    fid = fopen(bbFilePath);
    fseek(fid, 0, 'eof');
    fileSize = ftell(fid);
    fseek(fid, 0, 'bof');

    fileHeader = fread(fid, 2, 'char=>char');
    bbFileVersion = fread(fid, 2, 'char=>char');
    if any(fileHeader' ~= 'BB') || any(bbFileVersion' ~= 'v2')
        error(' ** incompatible .bbsignals file format! **');
    end

    numDimensions = fread(fid, 1, 'uint8=>double');
    dimensions = ones(1, numDimensions);
    for i = 1: numDimensions
        dimensions(i) = fread(fid, 1, 'int64=>double');
    end
    isComplexMatrix = double(fread(fid, 1, 'char=>double') == 'C');
    typeChar = fread(fid, 1, 'char=>double');
    typeBits = fread(fid, 1, 'uint8=>double');
    majority = SignalStorageMajority(fread(fid, 1, 'uint8=>double'));
    currentPos = ftell(fid);

    disp(['BBSignals: [' bbFilePath ']']);
    disp(['Read range: [' num2str(startRatio) '-' num2str(endRatio) ']'])
    disp(['Signature: [bbVer=' bbFileVersion' ', complex=' num2str(isComplexMatrix) ', type=' char(typeChar) num2str(typeBits) ', major=' char(majority) ']']);
    rowBytes = (typeBits / 8) * 2^isComplexMatrix * prod(dimensions(2:end));
    rowsInFile = floor((fileSize - currentPos) / rowBytes);
    dimensions(1) = rowsInFile;
    disp(['Size: numDim=' num2str(numDimensions) ', dims=[' num2str(dimensions) '], fileSize=' num2str(fileSize/1e6) 'MB'])

    if majority == SignalStorageMajority.ColumnMajor && prod(dimensions(2:end)) > 1
        columnMajorDimensions = dimensions;
        dimensions(1) = prod(dimensions);
        dimensions(2) = 1;
        if startRatio ~= 0 || endRatio ~= 1
            warning('LoadBBSignals does not support partial loading for column-major storage with >=2 dimensions, forced to full-length loading ...');
            startRatio = 0;
            endRatio = 1;
        end
    end

    readRatio = endRatio - startRatio;
    totalLines2Read = floor(readRatio * dimensions(1));
    skipLines = floor(startRatio * dimensions(1));
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
        data = coder.nullcopy(zeros([totalLines2Read dimensions(2:end)], 'like', complex(0)));
    else
        data = coder.nullcopy(zeros([totalLines2Read dimensions(2:end)]));
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
 
        if isComplexMatrix
            temp = complex(temp(1:2:end), temp(2:2:end)); % slower but save memory
        end

        temp = permute(reshape(temp, flip([step dimensions(2:end)])), flip(1:numDimensions));

        % use more ':' to support more dimensions
        data(readLines + 1 : readLines + step, :, :, :, :, :) = temp;
        readLines = readLines + step;
    end

    if exist('columnMajorDimensions', 'var')
        data = reshape(data(:), columnMajorDimensions);
    end
end
