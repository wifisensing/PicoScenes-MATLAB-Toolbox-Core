function LoadBBSignals(filePath, startRatio, endRatio)

    [~,fileName] = fileparts(filePath);
    validName = matlab.lang.makeValidName(fileName);
    
    % Read file header to determine version
    fid = fopen(filePath);
    fileHeader = fread(fid, 2, 'char=>char');
    bbFileVersion = fread(fid, 2, 'char=>char');
    if ~strcmp(fileHeader', 'BB') || (~strcmp(bbFileVersion', 'v2') && ~strcmp(bbFileVersion', 'v3'))
        error(' ** incompatible .bbsignals file format! **');
    end
    
    if strcmp(bbFileVersion', 'v2')
        bb_signal = loadBasebandSignalFileV2(filePath, startRatio, endRatio);
    elseif strcmp(bbFileVersion', 'v3')
        bb_signal = loadBasebandSignalFileV3(filePath, startRatio, endRatio);
    else
        error(' ** unsupported .bbsignals file version! **');
    end
    
    assignin('base', validName, bb_signal);
    disp(['Loaded variable name: ' validName]);

end

function data = loadBasebandSignalFileV2(filePath, startRatio, endRatio)
    fid = fopen(filePath);
    fseek(fid, 0, 'eof');
    fileSize = ftell(fid);
    fseek(fid, 0, 'bof');

    % Skip version header
    fseek(fid, 4, 'bof');

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

    disp(['BBSignals V2: [' filePath ']']);
    disp(['Read range: [' num2str(startRatio) '-' num2str(endRatio) ']'])
    disp(['Signature: [bbVer=v2, complex=' num2str(isComplexMatrix) ', type=' char(typeChar) num2str(typeBits) ', major=' char(majority) ']']);
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

function data = loadBasebandSignalFileV3(bbFilePath, startRatio, endRatio)
    fid = fopen(bbFilePath);
    fseek(fid, 0, 'eof');
    fileSize = ftell(fid);
    fseek(fid, 0, 'bof');

    % Skip version header
    fseek(fid, 4, 'bof');

    % Read global header
    numDimensions = fread(fid, 1, 'uint8=>double');
    dimensions = ones(1, numDimensions);
    for i = 1: numDimensions
        dimensions(i) = fread(fid, 1, 'int64=>double');
    end
    isComplexMatrix = double(fread(fid, 1, 'char=>double') == 'C');
    typeChar = fread(fid, 1, 'char=>double');
    typeBits = fread(fid, 1, 'uint8=>double');

    disp(['BBSignals V3: [' bbFilePath ']']);
    disp(['Read range: [' num2str(startRatio) '-' num2str(endRatio) ']'])
    disp(['Signature: [bbVer=v3, complex=' num2str(isComplexMatrix) ', type=' char(typeChar) num2str(typeBits) ']']);
    disp(['Size: numDim=' num2str(numDimensions) ', dims=[' num2str(dimensions) '], fileSize=' num2str(fileSize/1e6) 'MB'])

    % Determine data type
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

    % Calculate total rows and block information
    totalRows = 0;
    blockInfo = struct('startPos', {}, 'numRows', {}, 'majority', {});
    currentPos = ftell(fid);

    while currentPos < fileSize
        % Read block header
        majorityChar = fread(fid, 1, 'char=>char');
        numRows = fread(fid, 1, 'uint64=>double');
        
        % Store block information
        blockInfo(end+1).startPos = currentPos + 9; % 9 = 1(char) + 8(uint64)
        blockInfo(end).numRows = numRows;
        blockInfo(end).majority = majorityChar;
        
        % Calculate block size and move to next block
        rowBytes = (typeBits / 8) * 2^isComplexMatrix * prod(dimensions(2:end));
        blockSize = numRows * rowBytes;
        currentPos = currentPos + 9 + blockSize;
        fseek(fid, currentPos, 'bof');
        
        totalRows = totalRows + numRows;
    end

    % Calculate read range
    readRatio = endRatio - startRatio;
    totalLines2Read = floor(readRatio * totalRows);
    skipLines = floor(startRatio * totalRows);
    
    % Initialize output data
    if isComplexMatrix
        data = coder.nullcopy(zeros([totalLines2Read dimensions(2:end)], 'like', complex(0)));
    else
        data = coder.nullcopy(zeros([totalLines2Read dimensions(2:end)]));
    end

    % Read data blocks
    currentRow = 0;
    readRow = 0;
    for i = 1:length(blockInfo)
        block = blockInfo(i);
        
        % Skip blocks before start position
        if currentRow + block.numRows <= skipLines
            currentRow = currentRow + block.numRows;
            continue;
        end
        
        if block.majority == 'R'
            % Calculate rows to read from this block
            if currentRow < skipLines
                rowsToSkip = skipLines - currentRow;
                rowsToRead = min(block.numRows - rowsToSkip, totalLines2Read - readRow);
                fseek(fid, block.startPos + rowsToSkip * (typeBits / 8) * 2^isComplexMatrix * prod(dimensions(2:end)), 'bof');
            else
                rowsToRead = min(block.numRows, totalLines2Read - readRow);
                fseek(fid, block.startPos, 'bof');
            end
            
            if rowsToRead <= 0
                break;
            end
            
            % Read block data
            value2Read = rowsToRead * 2^isComplexMatrix * prod(dimensions(2:end));
            temp = fread(fid, value2Read, precision);
            
            if isComplexMatrix
                temp = complex(temp(1:2:end), temp(2:2:end));
            end
            
            temp = permute(reshape(temp, flip([rowsToRead dimensions(2:end)])), flip(1:numDimensions));
        else
            % For column-major storage, we need to read the entire block
            % and then extract the rows we need
            fseek(fid, block.startPos, 'bof');
            
            % Read the entire block
            value2Read = block.numRows * 2^isComplexMatrix * prod(dimensions(2:end));
            temp = fread(fid, value2Read, precision);
            
            if isComplexMatrix
                temp = complex(temp(1:2:end), temp(2:2:end));
            end
            
            % Reshape according to column-major format
            temp = reshape(temp, [block.numRows dimensions(2:end)]);
            
            % Extract only the rows we need
            if currentRow < skipLines
                rowsToSkip = skipLines - currentRow;
                rowsToRead = min(block.numRows - rowsToSkip, totalLines2Read - readRow);
                temp = temp(rowsToSkip + 1:rowsToSkip + rowsToRead, :);
            else
                rowsToRead = min(block.numRows, totalLines2Read - readRow);
                temp = temp(1:rowsToRead, :);
            end
        end
        
        data(readRow + 1:readRow + rowsToRead, :, :, :, :, :) = temp;
        readRow = readRow + rowsToRead;
        currentRow = currentRow + block.numRows;
        
        if readRow >= totalLines2Read
            break;
        end
    end
end
