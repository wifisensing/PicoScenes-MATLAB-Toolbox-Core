#include "ModularPicoScenesFrame.hxx"
#ifdef CUSTOM_HEADER_MAPPING_EXISTS
    #include "CustomHeaderMapping.hxx"
#endif

#include "MatlabDataArray.hpp"
#include "mexAdapter.hpp"
  
/**
 * This code performs the following 3 things:
 * 1. invokes 'rxs_parsing_core' to parse the raw RXS data entry
 * 2. further translate the result C++ struct into MATLAB structArray
 * 3. return the MATLAB structArray to MATLAB
 * 
 * The portal of this code is 'MexFunction' at the bottom.
 * 
 */

template <typename ValueType>
matlab::data::TypedArray<ValueType> createScalarMxArray(const ValueType &value) {
    matlab::data::ArrayFactory factory;
    return factory.createArray<ValueType>({1, 1}, {value});
}

template <typename TargetType, typename SourceType>
matlab::data::TypedArray<std::complex<TargetType>> copyComplexData2MxArray(const std::complex<SourceType> sourceArray[], uint32_t numElements, uint32_t ndim, const uint32_t *dims) {
    matlab::data::ArrayFactory factory;
    
    std::vector<size_t> dimensions(dims, dims + ndim);
    auto targetArray = factory.createArray<std::complex<TargetType>>(dimensions);

    std::transform(sourceArray, sourceArray + numElements, targetArray.begin(),
                   [](const std::complex<SourceType> &value) -> std::complex<TargetType> {
                       return std::complex<TargetType>(
                           static_cast<TargetType>(value.real()),
                           static_cast<TargetType>(value.imag())
                       );
                   });

    return targetArray;
}

template <typename TargetType, typename SourceType>
matlab::data::TypedArray<TargetType> copyData2MxArray(const SourceType sourceArray[], uint32_t numElements, uint32_t ndim, const uint32_t *dims) {
    matlab::data::ArrayFactory factory;

    std::vector<size_t> dimensions(dims, dims + ndim);
    auto targetArray = factory.createArray<TargetType>(dimensions);
    std::copy(sourceArray, sourceArray + numElements, targetArray.begin());

    return targetArray;
}

template <typename TargetType, typename SourceType>
matlab::data::TypedArray<std::complex<TargetType>> copyComplexData2MxArray(const std::complex<SourceType> sourceArray[], uint32_t numElements) {
    matlab::data::ArrayFactory factory;

    auto targetArray = factory.createArray<std::complex<TargetType>>({numElements, 1});
    for (uint32_t index = 0; index < numElements; ++index) {
        targetArray[index] = std::complex<TargetType>(
            static_cast<TargetType>(sourceArray[index].real()),
            static_cast<TargetType>(sourceArray[index].imag())
        );
    }

    return targetArray;
}

template <typename TargetType, typename SourceType, bool columnMajor = false>
matlab::data::TypedArray<TargetType> copyData2MxArray(const SourceType sourceArray[], uint32_t numElements) {
    matlab::data::ArrayFactory factory;

    auto targetArray = factory.createArray<TargetType>({columnMajor ? numElements : 1, columnMajor ? 1 : numElements});

    for (uint32_t index = 0; index < numElements; index++) {
        targetArray[index] = static_cast<TargetType>(sourceArray[index]);
    }

    return targetArray;
}

matlab::data::TypedArray<std::complex<double>> convertBasebandSignal2MxArray(const BasebandSignalSegment &bbSegment) {
    using namespace matlab::data;
    ArrayFactory factory;

    auto bbArray = factory.createArray<std::complex<double>>({static_cast<std::size_t>(bbSegment.getSignals().dimensions[0]), static_cast<std::size_t>(bbSegment.getSignals().dimensions[1])});
    auto numElements = bbSegment.getSignals().array.size();

    for (uint32_t index = 0; index < numElements; index++) {
        bbArray[index] = std::complex<double>(bbSegment.getSignals().array[index].real(), bbSegment.getSignals().array[index].imag());
    }

    return bbArray;
}


matlab::data::StructArray convertStandardHeader2MxArray(const ieee80211_mac_frame_header &standardHeader) {
    using namespace matlab::data;
    ArrayFactory factory;

    StructArray standardHeaderArray = factory.createStructArray({1, 1}, {"ControlField", "Addr1", "Addr2", "Addr3", "Fragment", "Sequence"});
    StructArray controlField = factory.createStructArray({1, 1}, {"Version", "Type", "SubType", "ToDS", "FromDS", "MoreFrags", "Retry", "PowerManagement", "More", "Protected", "Order"});
    
    controlField[0]["Version"] = createScalarMxArray(standardHeader.fc.version);
    controlField[0]["Type"] = createScalarMxArray(standardHeader.fc.type);
    controlField[0]["SubType"] = createScalarMxArray(standardHeader.fc.subtype);
    controlField[0]["ToDS"] = createScalarMxArray(standardHeader.fc.toDS);
    controlField[0]["FromDS"] = createScalarMxArray(standardHeader.fc.fromDS);
    controlField[0]["MoreFrags"] = createScalarMxArray(standardHeader.fc.moreFrags);
    controlField[0]["Retry"] = createScalarMxArray(standardHeader.fc.retry);
    controlField[0]["PowerManagement"] = createScalarMxArray(standardHeader.fc.power_mgmt);
    controlField[0]["More"] = createScalarMxArray(standardHeader.fc.more);
    controlField[0]["Protected"] = createScalarMxArray(standardHeader.fc.protect);
    controlField[0]["Order"] = createScalarMxArray(standardHeader.fc.order);
    
    standardHeaderArray[0]["ControlField"] = controlField;
    standardHeaderArray[0]["Addr1"] = factory.createArray<uint8_t>({1, 6}, standardHeader.addr1.begin(), standardHeader.addr1.end());
    standardHeaderArray[0]["Addr2"] = factory.createArray<uint8_t>({1, 6}, standardHeader.addr2.begin(), standardHeader.addr2.end());
    standardHeaderArray[0]["Addr3"] = factory.createArray<uint8_t>({1, 6}, standardHeader.addr3.begin(), standardHeader.addr3.end());

    standardHeaderArray[0]["Fragment"] = createScalarMxArray(standardHeader.frag);
    standardHeaderArray[0]["Sequence"] = createScalarMxArray(standardHeader.seq);

    return standardHeaderArray;
}

matlab::data::StructArray convertPicoScenesHeader2MxArray(const PicoScenesFrameHeader &header) {
    using namespace matlab::data;
    ArrayFactory factory;

    StructArray picoScenesFrameHeaderArray = factory.createStructArray({1, 1}, {"MagicValue", "Version", "DeviceType", "FrameType", "TaskId", "TxId"});
    picoScenesFrameHeaderArray[0]["MagicValue"] = createScalarMxArray(header.magicValue);
    picoScenesFrameHeaderArray[0]["Version"] = createScalarMxArray(header.version);
    picoScenesFrameHeaderArray[0]["DeviceType"] = createScalarMxArray(static_cast<uint16_t>(header.deviceType));
    picoScenesFrameHeaderArray[0]["FrameType"] = createScalarMxArray(header.frameType);
    picoScenesFrameHeaderArray[0]["TaskId"] = createScalarMxArray(header.taskId);
    picoScenesFrameHeaderArray[0]["TxId"] = createScalarMxArray(header.txId);

    return picoScenesFrameHeaderArray;
}

matlab::data::StructArray convertRxSBasic2MxArray(const RxSBasicSegment &basicSegment) {
    using namespace matlab::data;
    ArrayFactory factory;

    const auto& basic = basicSegment.getBasic();
    StructArray rxsArray = factory.createStructArray({1, 1}, { 
        "SegmentVersion", "DeviceType", "Timestamp", "SystemTime", "CenterFreq", "CenterFreq2", 
        "ControlFreq", "CBW", "PacketFormat", "PacketCBW", "GI", "MCS", "NumSTS", 
        "NumESS", "NumRx", "NoiseFloor", "RSSI", 
        "RSSI1", "RSSI2", "RSSI3", "RSSI4", 
        "RSSI5", "RSSI6", "RSSI7", "RSSI8"
    });

    rxsArray[0]["SegmentVersion"] = createScalarMxArray(basicSegment.segmentVersionId);    
    rxsArray[0]["DeviceType"] = createScalarMxArray(basic.deviceType);
    rxsArray[0]["Timestamp"] = createScalarMxArray(basic.tstamp);
    rxsArray[0]["SystemTime"] = createScalarMxArray(basic.systemTime);
    rxsArray[0]["CenterFreq"] = createScalarMxArray(basic.centerFreq);
    rxsArray[0]["CenterFreq2"] = createScalarMxArray(basic.centerFreq2);
    rxsArray[0]["ControlFreq"] = createScalarMxArray(basic.controlFreq);
    rxsArray[0]["CBW"] = createScalarMxArray(basic.cbw);
    rxsArray[0]["PacketFormat"] = createScalarMxArray(basic.packetFormat);
    rxsArray[0]["PacketCBW"] = createScalarMxArray(basic.pkt_cbw);
    rxsArray[0]["GI"] = createScalarMxArray(basic.guardInterval);
    rxsArray[0]["MCS"] = createScalarMxArray(basic.mcs);
    rxsArray[0]["NumSTS"] = createScalarMxArray(basic.numSTS);
    rxsArray[0]["NumESS"] = createScalarMxArray(basic.numESS);
    rxsArray[0]["NumRx"] = createScalarMxArray(basic.numRx);
    rxsArray[0]["NoiseFloor"] = createScalarMxArray(basic.noiseFloor);
    rxsArray[0]["RSSI"] = createScalarMxArray(basic.rssi);
    rxsArray[0]["RSSI1"] = createScalarMxArray(basic.rssi_ctl0);
    rxsArray[0]["RSSI2"] = createScalarMxArray(basic.rssi_ctl1);
    rxsArray[0]["RSSI3"] = createScalarMxArray(basic.rssi_ctl2);
    rxsArray[0]["RSSI4"] = createScalarMxArray(basic.rssi_ctl3);
    rxsArray[0]["RSSI5"] = createScalarMxArray(basic.rssi_ctl4);
    rxsArray[0]["RSSI6"] = createScalarMxArray(basic.rssi_ctl5);
    rxsArray[0]["RSSI7"] = createScalarMxArray(basic.rssi_ctl6);
    rxsArray[0]["RSSI8"] = createScalarMxArray(basic.rssi_ctl7);

    return rxsArray;
}


matlab::data::StructArray convertExtraInfo2MxArray(const ExtraInfoSegment &eiSegment) {
    using namespace matlab::data;
    ArrayFactory factory;

    const ExtraInfo& ei = eiSegment.getExtraInfo();
    std::vector<std::pair<std::string, matlab::data::Array>> fields;
    fields.emplace_back("SegmentVersion", factory.createScalar(eiSegment.segmentVersionId));
    fields.emplace_back("FeatureCode", factory.createScalar(ei.featureCode));
    fields.emplace_back("HasLength", factory.createScalar(ei.hasLength));
    fields.emplace_back("HasVersion", factory.createScalar(ei.hasVersion));
    fields.emplace_back("HasMacAddr_cur", factory.createScalar(ei.hasMacAddr_cur));
    fields.emplace_back("HasMacAddr_rom", factory.createScalar(ei.hasMacAddr_rom));
    fields.emplace_back("HasChansel", factory.createScalar(ei.hasChansel));
    fields.emplace_back("HasBMode", factory.createScalar(ei.hasBMode));
    fields.emplace_back("HasEVM", factory.createScalar(ei.hasEVM));
    fields.emplace_back("HasTxChainMask", factory.createScalar(ei.hasTxChainMask));
    fields.emplace_back("HasRxChainMask", factory.createScalar(ei.hasRxChainMask));
    fields.emplace_back("HasTxpower", factory.createScalar(ei.hasTxpower));
    fields.emplace_back("HasCF", factory.createScalar(ei.hasCF));
    fields.emplace_back("HasCFO", factory.createScalar(ei.hasCFO));
    fields.emplace_back("HasSFO", factory.createScalar(ei.hasSFO));
    fields.emplace_back("HasTxTSF", factory.createScalar(ei.hasTxTSF));
    fields.emplace_back("HasLastHwTxTSF", factory.createScalar(ei.hasLastHWTxTSF));
    fields.emplace_back("HasChannelFlags", factory.createScalar(ei.hasChannelFlags));
    fields.emplace_back("HasTxNess", factory.createScalar(ei.hasTxNess));
    fields.emplace_back("HasTuningPolicy", factory.createScalar(ei.hasTuningPolicy));
    fields.emplace_back("HasPLLRate", factory.createScalar(ei.hasPLLRate));
    fields.emplace_back("HasPLLClkSel", factory.createScalar(ei.hasPLLClkSel));
    fields.emplace_back("HasPLLRefDiv", factory.createScalar(ei.hasPLLRefDiv));
    fields.emplace_back("HasAGC", factory.createScalar(ei.hasAGC));
    fields.emplace_back("HasAntennaSelection", factory.createScalar(ei.hasAntennaSelection));
    fields.emplace_back("HasSamplingRate", factory.createScalar(ei.hasSamplingRate));
    fields.emplace_back("HasTemperature", factory.createScalar(ei.hasTemperature));

    if (ei.hasLength) {
        fields.emplace_back("Length", factory.createScalar(ei.length));
    }
    if (ei.hasVersion) {
        fields.emplace_back("Version", factory.createScalar(ei.version));
    }
    if (ei.hasMacAddr_cur) {
        TypedArray<uint8_t> macaddr_cur = factory.createArray<uint8_t>({1, 6}, ei.macaddr_cur, ei.macaddr_cur + 6);
        fields.emplace_back("MACAddressCurrent", macaddr_cur);
    }
    if (ei.hasMacAddr_rom) {
        TypedArray<uint8_t> macaddr_rom = factory.createArray<uint8_t>({1, 6}, ei.macaddr_rom, ei.macaddr_rom + 6);
        fields.emplace_back("MACAddressROM", macaddr_rom);
    }
    if (ei.hasChansel) {
        fields.emplace_back("CHANSEL", factory.createScalar(ei.chansel));
    }
    if (ei.hasBMode) {
        fields.emplace_back("BMode", factory.createScalar(ei.bmode));
    }
    if (ei.hasEVM) {
        TypedArray<int8_t> evmMxArray = factory.createArray<int8_t>({1, 20}, ei.evm, ei.evm + 20);
        fields.emplace_back("EVM", evmMxArray);
    }
    if (ei.hasTxChainMask) {
        fields.emplace_back("TxChainMask", factory.createScalar(ei.txChainMask));
    }
    if (ei.hasRxChainMask) {
        fields.emplace_back("RxChainMask", factory.createScalar(ei.rxChainMask));
    }
    if (ei.hasTxpower) {
        fields.emplace_back("TxPower", factory.createScalar(ei.txpower));
    }
    if (ei.hasCF) {
        fields.emplace_back("CF", factory.createScalar(ei.cf));
    }
    if (ei.hasTxTSF) {
        fields.emplace_back("TxTSF", factory.createScalar(ei.txTSF));
    }
    if (ei.hasLastHWTxTSF) {
        fields.emplace_back("LastTXTSF", factory.createScalar(ei.lastHwTxTSF));
    }
    if (ei.hasChannelFlags) {
        fields.emplace_back("ChannelFlags", factory.createScalar(ei.channelFlags));
    }
    if (ei.hasTxNess) {
        fields.emplace_back("TXNESS", factory.createScalar(ei.tx_ness));
    }
    if (ei.hasTuningPolicy) {
        fields.emplace_back("TuningPolicy", factory.createScalar(static_cast<uint8_t>(ei.tuningPolicy)));
    }
    if (ei.hasPLLRate) {
        fields.emplace_back("PLLRate", factory.createScalar(ei.pll_rate));
    }
    if (ei.hasPLLClkSel) {
        fields.emplace_back("PLLClockSelect", factory.createScalar(ei.pll_clock_select));
    }
    if (ei.hasPLLRefDiv) {
        fields.emplace_back("PLLRefDiv", factory.createScalar(ei.pll_refdiv));
    }
    if (ei.hasAGC) {
        fields.emplace_back("AGC", factory.createScalar(ei.agc));
    }
    if (ei.hasAntennaSelection) {
        TypedArray<uint8_t> antSelMxArray = factory.createArray<uint8_t>({1, 3}, ei.ant_sel, ei.ant_sel + 3);
        fields.emplace_back("ANTSEL", antSelMxArray);
    }
    if (ei.hasSamplingRate) {
        fields.emplace_back("SF", factory.createScalar(ei.samplingRate));
    }
    if (ei.hasCFO) {
        fields.emplace_back("CFO", factory.createScalar(static_cast<double>(ei.cfo)));
    }
    if (ei.hasSFO) {
        fields.emplace_back("SFO", factory.createScalar(static_cast<double>(ei.sfo)));
    }
    if (ei.hasTemperature) {
        fields.emplace_back("Temperature", factory.createScalar(static_cast<double>(ei.temperature)));
    }

    std::vector<std::string> fieldNames;
    for (const auto &field : fields) {
        fieldNames.push_back(field.first);
    }

    StructArray mxExtraInfo = factory.createStructArray({1, 1}, fieldNames);
    for (const auto &field : fields) {
        mxExtraInfo[0][field.first] = field.second;
    }

    return mxExtraInfo;
}

matlab::data::StructArray convertCSISegment2MxArray(const CSISegment &csiSegment) {
    using namespace matlab::data;
    ArrayFactory factory;

    StructArray groupCell = factory.createStructArray({1, 1}, {
        "SegmentVersion", "DeviceType", "FirmwareVersion", "PacketFormat", "CBW", 
        "CarrierFreq", "CarrierFreq2", "IsMerged", "SamplingRate", 
        "SubcarrierBandwidth", "NumTones", "NumTx", "NumRx", 
        "NumESS", "NumCSI", "ANTSEL", "CSI", 
        "Mag", "Phase", "SubcarrierIndex", "TimingOffsets", 
        "PhaseSlope", "PhaseIntercept"
    });

    const auto &csi = csiSegment.getCSI();
    
    groupCell[0]["SegmentVersion"] = createScalarMxArray(static_cast<double>(csiSegment.segmentVersionId));
    groupCell[0]["DeviceType"] = createScalarMxArray(static_cast<double>(csi->deviceType));
    groupCell[0]["FirmwareVersion"] = createScalarMxArray(static_cast<double>(csi->firmwareVersion));
    groupCell[0]["PacketFormat"] = createScalarMxArray(static_cast<double>(csi->packetFormat));
    groupCell[0]["CBW"] = createScalarMxArray(static_cast<double>(csi->cbw));
    groupCell[0]["CarrierFreq"] = createScalarMxArray(static_cast<double>(csi->carrierFreq));
    groupCell[0]["CarrierFreq2"] = createScalarMxArray(static_cast<double>(csi->carrierFreq2));
    groupCell[0]["IsMerged"] = createScalarMxArray(static_cast<double>(csi->isMerged));
    groupCell[0]["SamplingRate"] = createScalarMxArray(static_cast<double>(csi->samplingRate));
    groupCell[0]["SubcarrierBandwidth"] = createScalarMxArray(static_cast<double>(csi->subcarrierBandwidth));
    groupCell[0]["NumTones"] = createScalarMxArray(csi->dimensions.numTones);
    groupCell[0]["NumTx"] = createScalarMxArray(csi->dimensions.numTx);
    groupCell[0]["NumRx"] = createScalarMxArray(csi->dimensions.numRx);
    groupCell[0]["NumESS"] = createScalarMxArray(csi->dimensions.numESS);
    groupCell[0]["NumCSI"] = createScalarMxArray(csi->dimensions.numCSI);
    groupCell[0]["ANTSEL"] = createScalarMxArray(csi->antSel);

    std::vector<uint32_t> csiDataDimensions {csi->dimensions.numTones, 
                                            static_cast<uint32_t>(csi->dimensions.numTx + csi->dimensions.numESS), 
                                            csi->dimensions.numRx, 
                                            csi->dimensions.numCSI};

    auto CSIData = copyComplexData2MxArray<double, float>(&csi->CSIArray.array[0], csi->CSIArray.array.size(), csiDataDimensions.size(), csiDataDimensions.data());
    groupCell[0]["CSI"] = CSIData;

    auto magData = copyData2MxArray<double, float>(&csi->magnitudeArray.array[0], csi->magnitudeArray.array.size(), csiDataDimensions.size(), csiDataDimensions.data());
    groupCell[0]["Mag"] = magData;

    auto phaseData = copyData2MxArray<double, float>(&csi->phaseArray.array[0], csi->phaseArray.array.size(), csiDataDimensions.size(), csiDataDimensions.data());
    groupCell[0]["Phase"] = phaseData;

    auto indexData = copyData2MxArray<int16_t, int16_t>(&csi->subcarrierIndices[0], csi->subcarrierIndices.size());
    groupCell[0]["SubcarrierIndex"] = indexData;

    auto timingOffsetData = copyData2MxArray<uint32_t, uint32_t>(csi->timingOffsets.data(), csi->timingOffsets.size());
    groupCell[0]["TimingOffsets"] = timingOffsetData;

    if (!csi->phaseSlope.dimensions.empty()) {
        auto phaseSlopeData = copyData2MxArray<double, float>(&csi->phaseSlope.array[0], csi->phaseSlope.array.size(), 3, csiDataDimensions.data() + 1);
        groupCell[0]["PhaseSlope"] = phaseSlopeData;
    } else {
        groupCell[0]["PhaseSlope"] = factory.createArray<double>({0, 0}); // 空值
    }

    if (!csi->phaseIntercept.dimensions.empty()) {
        auto phaseInterceptData = copyData2MxArray<double, float>(&csi->phaseIntercept.array[0], csi->phaseIntercept.array.size(), 3, csiDataDimensions.data() + 1);
        groupCell[0]["PhaseIntercept"] = phaseInterceptData;
    } else {
        groupCell[0]["PhaseIntercept"] = factory.createArray<double>({0, 0}); // 空值
    }

    return groupCell;
}

matlab::data::StructArray convertFrameSegmentViaDynamicInterpretation(const AbstractPicoScenesFrameSegment &segment) {
    using namespace matlab::data;
    ArrayFactory factory;

    const auto dynamicType = DynamicContentTypeDictionary::getInstance()->queryType(segment.segmentName, segment.segmentVersionId);
    if (!dynamicType) {
        return factory.createStructArray({1, 1}, {});
    }

    std::vector<std::string> fieldNames;
    for (const auto &field : dynamicType->fields) {
        fieldNames.push_back(field.fieldName);
    }   

    StructArray segmentStruct = factory.createStructArray({1, 1}, fieldNames);

    for (const auto &field : dynamicType->fields) {
        const auto &fieldName = field.fieldName;  // 字段名
        const auto &fieldType = field.fieldType;  // 字段类型
        const auto &arraySize = field.arraySize;  // 数组大小

        switch (fieldType) {
            case DynamicContentFieldPrimitiveType::Int8:
                segmentStruct[0][fieldName] = copyData2MxArray<double, int8_t>(
                    segment.getDynamicInterpreter().getFieldPointer<int8_t>(fieldName), arraySize);
                break;
            case DynamicContentFieldPrimitiveType::Uint8:
                segmentStruct[0][fieldName] = copyData2MxArray<double, uint8_t>(
                    segment.getDynamicInterpreter().getFieldPointer<uint8_t>(fieldName), arraySize);
                break;
            case DynamicContentFieldPrimitiveType::Int16:
                segmentStruct[0][fieldName] = copyData2MxArray<double, int16_t>(
                    segment.getDynamicInterpreter().getFieldPointer<int16_t>(fieldName), arraySize);
                break;
            case DynamicContentFieldPrimitiveType::Uint16:
                segmentStruct[0][fieldName] = copyData2MxArray<double, uint16_t>(
                    segment.getDynamicInterpreter().getFieldPointer<uint16_t>(fieldName), arraySize);
                break;
            case DynamicContentFieldPrimitiveType::Int32:
                segmentStruct[0][fieldName] = copyData2MxArray<double, int32_t>(
                    segment.getDynamicInterpreter().getFieldPointer<int32_t>(fieldName), arraySize);
                break;
            case DynamicContentFieldPrimitiveType::Uint32:
                segmentStruct[0][fieldName] = copyData2MxArray<double, uint32_t>(
                    segment.getDynamicInterpreter().getFieldPointer<uint32_t>(fieldName), arraySize);
                break;
            case DynamicContentFieldPrimitiveType::Int64:
                segmentStruct[0][fieldName] = copyData2MxArray<double, int64_t>(
                    segment.getDynamicInterpreter().getFieldPointer<int64_t>(fieldName), arraySize);
                break;
            case DynamicContentFieldPrimitiveType::Uint64:
                segmentStruct[0][fieldName] = copyData2MxArray<double, uint64_t>(
                    segment.getDynamicInterpreter().getFieldPointer<uint64_t>(fieldName), arraySize);
                break;
            case DynamicContentFieldPrimitiveType::Single:
                segmentStruct[0][fieldName] = copyData2MxArray<double, float>(
                    segment.getDynamicInterpreter().getFieldPointer<float>(fieldName), arraySize);
                break;
            case DynamicContentFieldPrimitiveType::Double:
                segmentStruct[0][fieldName] = copyData2MxArray<double, double>(
                    segment.getDynamicInterpreter().getFieldPointer<double>(fieldName), arraySize);
                break;
            default:
                break;
        }
    }

    return segmentStruct;
}

matlab::data::StructArray convertSDRExtra2MxArray(const SDRExtraSegment &sdrExtraSegment) {
    using namespace matlab::data;
    ArrayFactory factory;

    StructArray sdrExtraArray = factory.createStructArray({1, 1}, {
        "SegmentVersion", "ScramblerInit", "PacketStartInternal", "PreciseRxSampleIndex", 
        "PreciseRxTime", "LastTxTime", "SignalInputTime", 
        "SignalDecodingTime", "SIGEVM", "InitialCFO", 
        "ResidualCFO", "DecodingDelay"
    });

    const SDRExtra& sdrExtra = sdrExtraSegment.getSdrExtra();
    sdrExtraArray[0]["SegmentVersion"] = createScalarMxArray(sdrExtraSegment.segmentVersionId);
    sdrExtraArray[0]["ScramblerInit"] = createScalarMxArray(sdrExtra.scramblerInit);
    sdrExtraArray[0]["PacketStartInternal"] = createScalarMxArray(sdrExtra.packetStartInternal);
    sdrExtraArray[0]["PreciseRxSampleIndex"] = createScalarMxArray(sdrExtra.hardwareRxSamplingIndex);
    sdrExtraArray[0]["PreciseRxTime"] = createScalarMxArray(sdrExtra.preciseRxTime);
    sdrExtraArray[0]["LastTxTime"] = createScalarMxArray(sdrExtra.lastTxTime);
    sdrExtraArray[0]["SignalInputTime"] = createScalarMxArray(sdrExtra.signalInputSystemTime);
    sdrExtraArray[0]["SignalDecodingTime"] = createScalarMxArray(sdrExtra.signalDecodeSystemTime);
    sdrExtraArray[0]["SIGEVM"] = createScalarMxArray(sdrExtra.sigEVM);
    sdrExtraArray[0]["InitialCFO"] = createScalarMxArray(sdrExtra.initialCFO);
    sdrExtraArray[0]["ResidualCFO"] = createScalarMxArray(sdrExtra.residualCFO);
    sdrExtraArray[0]["DecodingDelay"] = createScalarMxArray(sdrExtra.signalDecodeSystemTime - sdrExtra.signalInputSystemTime);

    return sdrExtraArray;
}

void convertPicoScenesFrame2Struct(ModularPicoScenesRxFrame &frame, matlab::data::StructArray &outCell, int index) {
    using namespace matlab::data;
    ArrayFactory factory;
    outCell[index]["StandardHeader"] = convertStandardHeader2MxArray(frame.standardHeader);
    outCell[index]["RxSBasic"] = convertRxSBasic2MxArray(*frame.rxSBasicSegment);
    outCell[index]["RxExtraInfo"] = convertExtraInfo2MxArray(*frame.rxExtraInfoSegment);
    if (frame.mvmExtraSegment) {
        outCell[index]["MVMExtra"] = convertFrameSegmentViaDynamicInterpretation(*frame.mvmExtraSegment);
    }
    if (frame.sdrExtraSegment) {
        outCell[index]["SDRExtra"] = convertSDRExtra2MxArray(*frame.sdrExtraSegment);
    }
    outCell[index]["CSI"] = convertCSISegment2MxArray(*frame.csiSegment);
    if (frame.PicoScenesHeader) {
        outCell[index]["PicoScenesHeader"] = convertPicoScenesHeader2MxArray(*frame.PicoScenesHeader);
    } else {
        outCell[index]["PicoScenesHeader"] = factory.createStructArray({1, 1}, {});
    }
    if (frame.txExtraInfoSegment) {
        outCell[index]["TxExtraInfo"] = convertExtraInfo2MxArray(*frame.txExtraInfoSegment);
    } else {
        outCell[index]["TxExtraInfo"] = factory.createStructArray({1, 1}, {});
    }
    std::vector<std::string> txSegmentNames;
    for (const auto &txSegment : frame.txUnknownSegments) {
        txSegmentNames.push_back(txSegment.first);
    }
    StructArray txForeignSegments = factory.createStructArray({1, 1}, txSegmentNames);
    for (const auto &txSegment : frame.txUnknownSegments) {
        txForeignSegments[0][txSegment.first] = convertFrameSegmentViaDynamicInterpretation(*txSegment.second);
    }
    outCell[index]["TxForeignSegments"] = txForeignSegments;

    std::vector<std::string> rxSegmentNames;
    for (const auto &rxSegment : frame.rxUnknownSegments) {
        rxSegmentNames.push_back(rxSegment.first);
    }
    StructArray rxForeignSegments = factory.createStructArray({1, 1}, rxSegmentNames);
    for (const auto &rxSegment : frame.rxUnknownSegments) {
        rxForeignSegments[0][rxSegment.first] = convertFrameSegmentViaDynamicInterpretation(*rxSegment.second);
    }
    outCell[index]["RxForeignSegments"] = rxForeignSegments;

    if (frame.legacyCSISegment) {
        outCell[index]["LegacyCSI"] = convertCSISegment2MxArray(*frame.legacyCSISegment);
    }
    if (frame.basebandSignalSegment) {
        outCell[index]["BasebandSignals"] = convertBasebandSignal2MxArray(*frame.basebandSignalSegment);
    }
    if (!frame.mpdus.empty()) {
        outCell[index]["MPDU"] = copyData2MxArray<uint8_t, uint8_t, true>(frame.mpdus[0].data(), frame.mpdus[0].size());
    } else {
        outCell[index]["MPDU"] = copyData2MxArray<uint8_t, uint8_t, true>(nullptr, 0);
    }
}

std::vector<std::string> getPicoScenesFrameStructField(ModularPicoScenesRxFrame &frame) {
    std::vector<std::string> fields = {
        "StandardHeader", 
        "RxSBasic", 
        "RxExtraInfo", 
        "CSI", 
        "PicoScenesHeader", 
        "TxExtraInfo", 
        "TxForeignSegments", 
        "RxForeignSegments",  
        "MPDU"
    };
    if (frame.mvmExtraSegment) {
        fields.emplace_back("MVMExtra");
    }
    if (frame.sdrExtraSegment) {
        fields.emplace_back("SDRExtra");
    }
    if (frame.legacyCSISegment) {
        fields.emplace_back("LegacyCSI");
    }
    if (frame.basebandSignalSegment) {
        fields.emplace_back("BasebandSignals");
    }

    return fields;
}

using namespace matlab::data;
using matlab::mex::ArgumentList;

class MexFunction : public matlab::mex::Function {
public:
    void operator()(ArgumentList outputs, ArgumentList inputs) {
        checkArguments(outputs, inputs);

        TypedArray<uint8_t> inputArray = std::move(inputs[0]);
        const uint8_t* inBytes = &(*inputArray.begin());
        size_t bufferLength = inputArray.getNumberOfElements();
        
        IntelMVMParsedCSIHeader::registerDefaultMVMHeaderInterpretation();
#ifdef CUSTOM_HEADER_MAPPING_EXISTS
        CustomHeaderMapping::registerPrivateInterpretations();
#endif
        if (auto frame = ModularPicoScenesRxFrame::fromBuffer(inBytes, bufferLength, true)) {
            ArrayFactory factory;
            auto frameFields = getPicoScenesFrameStructField(*frame);
            StructArray result = factory.createStructArray({1, 1}, frameFields); 

            if (std::any_of(frame->payloadSegments.cbegin(), frame->payloadSegments.cend(),
                            [](const std::shared_ptr<PayloadSegment>& payloadSegment) {
                                return payloadSegment->getPayloadData().payloadDescription == "EchoProbeReplyCSI" ||
                                       payloadSegment->getPayloadData().payloadDescription == "EchoProbeReplyFull";
                            })) {
                result = factory.createStructArray({2, 1}, frameFields); 
            }

            convertPicoScenesFrame2Struct(*frame, result, 0);
            if (auto echoProbeCSIPayloadIt = std::find_if(frame->payloadSegments.cbegin(), frame->payloadSegments.cend(),
                                                          [](const std::shared_ptr<PayloadSegment>& payloadSegment) {
                                                              return payloadSegment->getPayloadData().payloadDescription == "EchoProbeReplyCSI";
                                                          });
                echoProbeCSIPayloadIt != frame->payloadSegments.cend()) {

                convertPicoScenesFrame2Struct(*frame, result, 1); // 伪造 ACK 帧
                const auto& csiPayload = (*echoProbeCSIPayloadIt)->getPayloadData().payloadData;
                auto txCSISegment = CSISegment(csiPayload.data(), csiPayload.size());
                txCSISegment.getCSI()->removeCSDAndInterpolateCSI();
                auto rxCSIGroups = convertCSISegment2MxArray(txCSISegment);
                result[1]["CSI"] = rxCSIGroups;
            }

            if (auto echoProbeFullPacketIt = std::find_if(frame->payloadSegments.cbegin(), frame->payloadSegments.cend(),
                                                          [](const std::shared_ptr<PayloadSegment>& payloadSegment) {
                                                              return payloadSegment->getPayloadData().payloadDescription == "EchoProbeReplyFull";
                                                          });
                echoProbeFullPacketIt != frame->payloadSegments.cend()) {
                const auto& rxFrameBuffer = (*echoProbeFullPacketIt)->getPayloadData().payloadData;
                if (auto initiatingFrame = ModularPicoScenesRxFrame::fromBuffer(rxFrameBuffer.data(), rxFrameBuffer.size(), true)) {
                    convertPicoScenesFrame2Struct(*initiatingFrame, result, 1);
                }
            }

            outputs[0] = std::move(result);
        } else {
            ArrayFactory factory;
            outputs[0] = factory.createStructArray({1, 1}, {});
        }
    }

private:

    void checkArguments(ArgumentList& outputs, ArgumentList& inputs) {
        if (inputs.size() != 1) {
            throw std::invalid_argument("Wi-Fi Sensing Toolbox:read_csi:nrhs - One input required.");
        }
        if (outputs.size() != 1) {
            throw std::invalid_argument("Wi-Fi Sensing Toolbox:read_csi:nlhs - One output required.");
        }
        if (inputs[0].getType() != matlab::data::ArrayType::UINT8) {
            throw std::invalid_argument("Wi-Fi Sensing Toolbox:read_csi:notBytes - Input must be a uint8 array.");
        }
    }
};