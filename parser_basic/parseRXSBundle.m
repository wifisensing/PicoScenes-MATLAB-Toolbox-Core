function [rxsBundles] = parseRXSBundle(rxsBundleFilePathOrRxsCells, maxRxSLogNumber)

    if nargin < 2
        maxRxSLogNumber = intmax;
    end

    if ischar(rxsBundleFilePathOrRxsCells)
        rxs_cells = read_rxs_log(rxsBundleFilePathOrRxsCells, maxRxSLogNumber);
    elseif iscell(rxsBundleFilePathOrRxsCells)
        rxs_cells = rxsBundleFilePathOrRxsCells;
    end

    if isempty(rxs_cells)
        rxsBundles = [];
        return;
    end

    inner_size = cellfun(@(x) numel(x), rxs_cells);
    outlierIndex = inner_size ~= median(inner_size);
    rxs_cells(outlierIndex) = [];
    rxs_struct_array = [rxs_cells{:}]';
    rxsBundles = cell(1, size(rxs_struct_array, 2));

    for i = 1:size(rxs_struct_array, 2)
        rxsBundles{i} = CombineRXSStructs(rxs_struct_array(:, i));
    end

end

function [rxs_struct] = CombineRXSStructs (rxs_struct_array)
    originRxSBasic = [rxs_struct_array.RxSBasic].';
    originRxExtraInfo = [rxs_struct_array.RxExtraInfo].';
    originCSI = [rxs_struct_array.CSI].';
    originStandardHeader = [rxs_struct_array.StandardHeader].';
    originPicoScenesHeader = [rxs_struct_array.PicoScenesHeader].';
    for i=2:size(rxs_struct_array)
        if ~isfield(rxs_struct_array(i).TxExtraInfo, 'HasLength')
            rxs_struct_array(i).TxExtraInfo = rxs_struct_array(i - 1).TxExtraInfo;
        end
    end
    originTxExtraInfo = [rxs_struct_array.TxExtraInfo].';
    
    if isfield(rxs_struct_array, 'BasebandSignals')
        PilotCSI = {rxs_struct_array.PilotCSI}.';
        LegacyCSI = {rxs_struct_array.LegacyCSI}.';
        BasebandSignals = {rxs_struct_array.BasebandSignals}.';
        PreEQSymbols = {rxs_struct_array.PreEQSymbols}.';
    else
        PilotCSI = {};
        LegacyCSI = {};
        BasebandSignals = {};
        PreEQSymbols = {};
    end

    mergedRxSBasic = combineRxSBasic(originRxSBasic);
    mergedRxExtraInfo = combineExtraInfoSlots(originRxExtraInfo);
    mergedCSI = combineCSI(originCSI);
    mergedHeader = combineStandardHeader(originStandardHeader);
    headerCombined = mergedHeader;
    
    if ~isempty(fieldnames(originPicoScenesHeader))
        mergedPSHeader = combinePicoScenesHeader(originPicoScenesHeader);
        headerCombined.DeviceType = mergedPSHeader.DeviceType;
        headerCombined.FrameType = mergedPSHeader.FrameType;
        headerCombined.TaskId = mergedPSHeader.TaskId;
        headerCombined.TxId = mergedPSHeader.TxId;
    else
        mergedPSHeader = [];
        headerCombined.DeviceType = repmat(mergedCSI.DeviceType(1), numel(rxs_struct_array), 1);
        headerCombined.FrameType = zeros(numel(rxs_struct_array), 1, 'uint16');
        headerCombined.TaskId = zeros(numel(rxs_struct_array), 1, 'uint16');
        headerCombined.TxId = zeros(numel(rxs_struct_array), 1, 'uint16');
    end
    
    if ~isempty(fieldnames(originTxExtraInfo))
        mergedTxExtraInfo = combineExtraInfoSlots(originTxExtraInfo);
    else
        mergedTxExtraInfo = [];
    end

    if isfield(rxs_struct_array, 'MVMExtra')
        meregedMVMExtra = combineMVMExtraSegment([rxs_struct_array.MVMExtra].');
    else
        meregedMVMExtra = [];
    end

    if isfield(rxs_struct_array, 'DPASRequest')
        mergedDPASRequests = combineDPASRequestSegment([rxs_struct_array.DPASRequest].');
    else
        mergedDPASRequests = [];
    end

    Channel = mergedCSI;
    Channel = rmfield(Channel, {'CSI', 'Mag', 'Phase', 'SubcarrierIndex'});

    rxs_struct.Header = headerCombined;
    rxs_struct.Basic = mergedRxSBasic;
    rxs_struct.RxExtraInfo = mergedRxExtraInfo;
    rxs_struct.TxExtraInfo = mergedTxExtraInfo;
    rxs_struct.Channel = Channel;
    rxs_struct.MVMExtra = meregedMVMExtra;
    rxs_struct.DPASRequest = mergedDPASRequests;
    rxs_struct.CSI = mergedCSI.CSI;
    rxs_struct.Mag = mergedCSI.Mag;
    rxs_struct.Phase = mergedCSI.Phase;
    rxs_struct.SubcarrierIndex = mergedCSI.SubcarrierIndex;
    rxs_struct.Baseband.PilotCSI = PilotCSI;
    rxs_struct.Baseband.LegacyCSI = LegacyCSI;
    rxs_struct.Baseband.BasebandSignals = BasebandSignals;
    rxs_struct.Baseband.PreEQSymbols = PreEQSymbols;
end

function RxSBasic = combineRxSBasic(originRxSBasic)
    confirmArray = [
                "DeviceType";
                "Timestamp";
                "CenterFreq";
                "ControlFreq";
                "CBW";
                "PacketFormat";
                "PacketCBW";
                "GI";
                "MCS";
                "NumSTS";
                "NumESS";
                "NumRx";
                "NoiseFloor";
                "RSSI";
                "RSSI1";
                "RSSI2";
                "RSSI3";
                ];

    for i = 1:size(confirmArray, 1)
        RxSBasic.(confirmArray(i)) = [originRxSBasic.(confirmArray(i))]';
    end

end

function CSI = combineCSI(originCSI)
    confirmArray = [
                "DeviceType";
                "PacketFormat";
                "CBW";
                "CarrierFreq";
                "SamplingRate";
                "SubcarrierBandwidth";
                "NumTones";
                "NumTx";
                "NumRx";
                "NumESS";
                "ANTSEL";
                "CSI";
                "Mag";
                "Phase";
                "SubcarrierIndex";
                ];

    for i = 1:size(confirmArray, 1)
        singleField = originCSI.(confirmArray(i));

        if isscalar(singleField)
            CSI.(confirmArray(i)) = [originCSI.(confirmArray(i))].';
        elseif isvector(singleField) && isrow(singleField)
            CSI.(confirmArray(i)) = cell2mat({originCSI.(confirmArray(i))}');
        elseif isvector(singleField) && iscolumn(singleField)
            CSI.(confirmArray(i)) = cell2mat({originCSI.(confirmArray(i))});
        end

    end
    
    if isempty(CSI.Mag) || isempty(CSI.Phase)
        CSI.Mag = abs(CSI.CSI);
        CSI.Phase = angle(CSI.CSI);
    end

end

function StandardHeader = combineStandardHeader(originHeader)
    confirmArray = [
                "Addr1";
                "Addr2";
                "Addr3";
                "Fragment";
                "Sequence";
                ];

    for i = 1:size(confirmArray, 1)
        singleField = originHeader.(confirmArray(i));

        if isscalar(singleField)
            StandardHeader.(confirmArray(i)) = [originHeader.(confirmArray(i))].';
        elseif isvector(singleField) && isrow(singleField)
            StandardHeader.(confirmArray(i)) = cell2mat({originHeader.(confirmArray(i))}');
        elseif isvector(singleField) && iscol(singleField)
            StandardHeader.(confirmArray(i)) = cell2mat({originHeader.(confirmArray(i))});
        end

    end

end

function PSHeader = combinePicoScenesHeader(originHeader)
    confirmArray = [
                "MagicValue";
                "Version";
                "DeviceType";
                "FrameType";
                "TaskId";
                "TxId";
                ];

    for i = 1:size(confirmArray, 1)
        singleField = originHeader.(confirmArray(i));

        if isscalar(singleField)
            PSHeader.(confirmArray(i)) = [originHeader.(confirmArray(i))].';
        elseif isvector(singleField) && isrow(singleField)
            PSHeader.(confirmArray(i)) = cell2mat({originHeader.(confirmArray(i))}');
        elseif isvector(singleField) && iscol(singleField)
            PSHeader.(confirmArray(i)) = cell2mat({originHeader.(confirmArray(i))});
        end

    end

end

function extraInfoBundle = combineExtraInfoSlots(ExtraInfos)

    if isfield(ExtraInfos, 'Version')
        extraInfoBundle.Version = [ExtraInfos.Version]';
    end

    if isfield(ExtraInfos, 'CHANSEL')
        extraInfoBundle.CHANSEL = [ExtraInfos.CHANSEL]';
    end

    if isfield(ExtraInfos, 'BMode')
        txbmode = [ExtraInfos.BMode]';
        extraInfoBundle.txbmode = txbmode;
    end

    if isfield(ExtraInfos, 'MACAddressROM')
        extraInfoBundle.MACAddressROM = reshape(dec2hex([ExtraInfos.MACAddressROM]')', 6, [])';
    end

    if isfield(ExtraInfos, 'MACAddressCurrent')
        extraInfoBundle.MACAddressCurrent = reshape(dec2hex([ExtraInfos.MACAddressCurrent]')', 6, [])';
    end

    if isfield(ExtraInfos, 'EVM')
        extraInfoBundle.EVM = reshape([ExtraInfos.EVM], 18, [])';
    end

    if isfield(ExtraInfos, 'TxChainMask')
        extraInfoBundle.TxChainMask = [ExtraInfos.TxChainMask]';
    end

    if isfield(ExtraInfos, 'RxChainMask')
        extraInfoBundle.RxChainMask = [ExtraInfos.RxChainMask]';
    end

    if isfield(ExtraInfos, 'TxPower')
        extraInfoBundle.TxPower = [ExtraInfos.TxPower]';
    end

    if isfield(ExtraInfos, 'CF')
        extraInfoBundle.CF = [ExtraInfos.CF]';

        if isfield(ExtraInfos, 'CHANSEL') && isfield(ExtraInfos, 'BMode') && exist('chansel2Freq', 'file') == 2
            extraInfoBundle.CF = round(chansel2Freq(double(extraInfoBundle.CHANSEL), double(extraInfoBundle.BMode)));
        end

    end

    if isfield(ExtraInfos, 'SF')
        extraInfoBundle.SF = [ExtraInfos.SF]';
    end

    if isfield(ExtraInfos, 'TxTSF')
        extraInfoBundle.TxTSF = [ExtraInfos.TxTSF]';
    end

    if isfield(ExtraInfos, 'LastTxTSF')
        extraInfoBundle.LastTxTSF = [ExtraInfos.LastTxTSF]';
    end

    if isfield(ExtraInfos, 'ChannelFlags')
        extraInfoBundle.ChannelMode = bitand([ExtraInfos.ChannelFlags]', 62); % 62 = 63 - 1, which removes the HT_5G bit
    end

    if isfield(ExtraInfos, 'TXNESS')
        extraInfoBundle.TXNESS = [ExtraInfos.TXNESS]';
    end

    if isfield(ExtraInfos, 'TuningPolicy')
        extraInfoBundle.TuningPolicy = [ExtraInfos.TuningPolicy]';
    end

    if isfield(ExtraInfos, 'PLLRate')
        extraInfoBundle.PLLRate = [ExtraInfos.PLLRate]';
    end

    if isfield(ExtraInfos, 'PLLClockSelect')
        extraInfoBundle.PLLClockSelect = [ExtraInfos.PLLClockSelect]';
    end

    if isfield(ExtraInfos, 'PLLRefDiv')
        extraInfoBundle.PLLRefDiv = [ExtraInfos.PLLRefDiv]';
    end

    if isfield(ExtraInfos, 'AGC')
        extraInfoBundle.AGC = [ExtraInfos.AGC]';
    end

    if isfield(ExtraInfos, 'ANTSEL')
        extraInfoBundle.ANTSEL = reshape([ExtraInfos.ANTSEL], 3, [])';
    end
    
    if isfield(ExtraInfos, 'CFO')
        extraInfoBundle.CFO = [ExtraInfos.CFO]';
    end
    
    if isfield(ExtraInfos, 'SFO')
        extraInfoBundle.SFO = [ExtraInfos.SFO]';
    end

    if isfield(ExtraInfos, 'Temperature')
        extraInfoBundle.Temperature = [ExtraInfos.Temperature]';
    end
end

function NVMExtra = combineMVMExtraSegment(originalNVMExtra)
    confirmArray = [
                "FTMClock";
                "MuClock";
                "RateNFlags";
                ];

    for i = 1:size(confirmArray, 1)
        singleField = originalNVMExtra.(confirmArray(i));

        if isscalar(singleField)
            NVMExtra.(confirmArray(i)) = [originalNVMExtra.(confirmArray(i))].';
        elseif isvector(singleField) && isrow(singleField)
            NVMExtra.(confirmArray(i)) = cell2mat({originalNVMExtra.(confirmArray(i))}');
        elseif isvector(singleField) && iscol(singleField)
            NVMExtra.(confirmArray(i)) = cell2mat({originalNVMExtra.(confirmArray(i))});
        end

    end

end

function dpasRequest = combineDPASRequestSegment(originalDPASRequest)
    confirmArray = [
                "BatchId";
                "BatchLength";
                "Sequence";
                "Interval";
                "Step";
                ];

    for i = 1:size(confirmArray, 1)
        singleField = originalDPASRequest.(confirmArray(i));

        if isscalar(singleField)
            dpasRequest.(confirmArray(i)) = [originalDPASRequest.(confirmArray(i))].';
        elseif isvector(singleField) && isrow(singleField)
            dpasRequest.(confirmArray(i)) = cell2mat({originalDPASRequest.(confirmArray(i))}');
        elseif isvector(singleField) && iscol(singleField)
            dpasRequest.(confirmArray(i)) = cell2mat({originalDPASRequest.(confirmArray(i))});
        end

    end

end