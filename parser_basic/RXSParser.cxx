#include "mex.h"
#include "ModularPicoScenesFrame.hxx"
#ifdef CUSTOM_HEADER_MAPPING_EXISTS
    #include "CustomHeaderMapping.hxx"
#endif

/**
 * This code performs the following 3 things:
 * 1. invokes 'rxs_parsing_core' to parse the raw RXS data entry
 * 2. further translate the result C++ struct into MATLAB mxArray struct
 * 3. return the MATLAB mxArray to MATLAB
 * 
 * The portal of this code is 'mexFunction' at the bottom.
 * 
 */

template <typename ValueType>
mxArray *createScalarMxArray(const ValueType &value) {
    mxClassID classId = mxDOUBLE_CLASS;
    mxArray *valueArray;
    if constexpr (std::is_same<ValueType, double>::value) {
        classId = mxDOUBLE_CLASS;
    } else if constexpr (std::is_same<ValueType, float>::value) {
        classId = mxSINGLE_CLASS;
    } else if constexpr (std::is_same<ValueType, uint64_t>::value) {
        classId = mxUINT64_CLASS;
    } else if constexpr (std::is_same<ValueType, uint32_t>::value) {
        classId = mxUINT32_CLASS;
    } else if constexpr (std::is_same<ValueType, uint16_t>::value) {
        classId = mxUINT16_CLASS;
    } else if constexpr (std::is_same<ValueType, uint8_t>::value) {
        classId = mxUINT8_CLASS;
    } else if constexpr (std::is_same<ValueType, bool>::value) {
        classId = mxUINT8_CLASS;
    } else if constexpr (std::is_same<ValueType, int64_t>::value) {
        classId = mxINT64_CLASS;
    } else if constexpr (std::is_same<ValueType, int32_t>::value) {
        classId = mxINT32_CLASS;
    } else if constexpr (std::is_same<ValueType, int16_t>::value) {
        classId = mxINT16_CLASS;
    } else if constexpr (std::is_same<ValueType, int8_t>::value) {
        classId = mxINT8_CLASS;
    }

    valueArray = mxCreateNumericMatrix(1, 1, classId, mxREAL);
    auto *ptrR = (ValueType *)mxGetPr(valueArray);
    *ptrR = value;
    return valueArray;
}

template <typename TargetType, typename SourceType>
mxArray *copyComplexData2MxArray(const std::complex<SourceType> sourceArray[], mwSize numElements, mwSize ndim, const mwSize * dims) {
    mxArray *targetArray = nullptr;
    if constexpr (std::is_same<TargetType, double>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxDOUBLE_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, float>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxSINGLE_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, uint64_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxUINT64_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, uint32_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxUINT32_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, uint16_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxUINT16_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, uint8_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxUINT8_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, int64_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxINT64_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, int32_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxINT32_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, int16_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxINT16_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, int8_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxINT8_CLASS, mxCOMPLEX);
    }

    TargetType *realValue, *imagValue;
    realValue = (TargetType *)mxMalloc(numElements * sizeof(TargetType));
    imagValue = (TargetType *)mxMalloc(numElements * sizeof(TargetType));
    for (uint32_t index = 0; index < numElements; index++) {
        realValue[index] = static_cast<TargetType>(sourceArray[index].real());
        imagValue[index] = static_cast<TargetType>(sourceArray[index].imag());
    }
    mxSetPr(targetArray, (double *)realValue);
    mxSetPi(targetArray, (double *)imagValue);

    return targetArray;
}

template <typename TargetType, typename SourceType, bool columnMajor = false>
mxArray *copyData2MxArray(const SourceType sourceArray[], mwSize numElements, mwSize ndim, const mwSize * dims) {
    mxArray *targetArray = nullptr;
    if constexpr (std::is_same<TargetType, double>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxDOUBLE_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, float>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxSINGLE_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, uint64_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxUINT64_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, uint32_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxUINT32_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, uint16_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxUINT16_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, uint8_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxUINT8_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, int64_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxINT64_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, int32_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxINT32_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, int16_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxINT16_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, int8_t>::value) {
        targetArray = mxCreateNumericArray(ndim, dims, mxINT8_CLASS, mxREAL);
    }

    TargetType *realValue;
    realValue = (TargetType *)mxMalloc(numElements * sizeof(TargetType));
    for (uint32_t index = 0; index < numElements; index++) {
        realValue[index] = static_cast<TargetType>(sourceArray[index]);
    }
    mxSetPr(targetArray, (double *)realValue);

    return targetArray;
}

template <typename TargetType, typename SourceType>
mxArray *copyComplexData2MxArray(const std::complex<SourceType> sourceArray[], uint32_t numElements) {
    mxArray *targetArray = nullptr;
    if constexpr (std::is_same<TargetType, double>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxDOUBLE_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, float>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxSINGLE_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, uint64_t>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxUINT64_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, uint32_t>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxUINT32_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, uint16_t>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxUINT16_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, uint8_t>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxUINT8_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, int64_t>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxINT64_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, int32_t>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxINT32_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, int16_t>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxINT16_CLASS, mxCOMPLEX);
    } else if constexpr (std::is_same<TargetType, int8_t>::value) {
        targetArray = mxCreateNumericMatrix(1, numElements, mxINT8_CLASS, mxCOMPLEX);
    }

    TargetType *realValue, *imagValue;
    realValue = (TargetType *)mxMalloc(numElements * sizeof(TargetType));
    imagValue = (TargetType *)mxMalloc(numElements * sizeof(TargetType));
    for (uint32_t index = 0; index < numElements; index++) {
        realValue[index] = static_cast<TargetType>(sourceArray[index].real());
        imagValue[index] = static_cast<TargetType>(sourceArray[index].imag());
    }
    mxSetPr(targetArray, (double *)realValue);
    mxSetPi(targetArray, (double *)imagValue);

    return targetArray;
}

template <typename TargetType, typename SourceType, bool columnMajor = false>
mxArray *copyData2MxArray(const SourceType sourceArray[], uint32_t numElements) {
    mxArray *targetArray = nullptr;
    if constexpr (std::is_same<TargetType, double>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxDOUBLE_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, float>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxSINGLE_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, uint64_t>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxUINT64_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, uint32_t>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxUINT32_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, uint16_t>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxUINT16_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, uint8_t>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxUINT8_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, int64_t>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxINT64_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, int32_t>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxINT32_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, int16_t>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxINT16_CLASS, mxREAL);
    } else if constexpr (std::is_same<TargetType, int8_t>::value) {
        targetArray = mxCreateNumericMatrix(columnMajor ? numElements : 1, columnMajor ? 1 : numElements, mxINT8_CLASS, mxREAL);
    }

    TargetType *realValue;
    realValue = (TargetType *)mxMalloc(numElements * sizeof(TargetType));
    for (uint32_t index = 0; index < numElements; index++) {
        realValue[index] = static_cast<TargetType>(sourceArray[index]);
    }
    mxSetPr(targetArray, (double *)realValue);

    return targetArray;
}

mxArray *convertBasebandSignal2MxArray(const BasebandSignalSegment &bbSegment) {
    auto bbArray = mxCreateNumericMatrix(bbSegment.getSignals().dimensions[0], bbSegment.getSignals().dimensions[1], mxDOUBLE_CLASS, mxCOMPLEX);

    auto numElements = bbSegment.getSignals().array.size();
    auto realValue = (double *)mxMalloc(numElements * sizeof(double));
    auto imagValue = (double *)mxMalloc(numElements * sizeof(double));

    for (uint32_t index = 0; index < numElements; index++) {
        realValue[index] = static_cast<double>(bbSegment.getSignals().array[index].real());
        imagValue[index] = static_cast<double>(bbSegment.getSignals().array[index].imag());
    }
    
    mxSetPr(bbArray, (double *)realValue);
    mxSetPi(bbArray, (double *)imagValue);

    return bbArray;
}

mxArray *convertStandardHeader2MxArray(const ieee80211_mac_frame_header &standardHeader) {
    auto *standardHeaderArray = mxCreateStructMatrix(1, 1, 0, NULL);

    // add Control Field
    auto *controlField = mxCreateStructMatrix(1, 1, 0, NULL);
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "Version"), createScalarMxArray(standardHeader.fc.version));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "Type"), createScalarMxArray(standardHeader.fc.type));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "SubType"), createScalarMxArray(standardHeader.fc.subtype));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "ToDS"), createScalarMxArray(standardHeader.fc.toDS));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "FromDS"), createScalarMxArray(standardHeader.fc.fromDS));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "MoreFrags"), createScalarMxArray(standardHeader.fc.moreFrags));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "Retry"), createScalarMxArray(standardHeader.fc.retry));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "PowerManagement"), createScalarMxArray(standardHeader.fc.power_mgmt));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "More"), createScalarMxArray(standardHeader.fc.more));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "Protected"), createScalarMxArray(standardHeader.fc.protect));
    mxSetFieldByNumber(controlField, 0, mxAddField(controlField, "Order"), createScalarMxArray(standardHeader.fc.order));
    mxSetFieldByNumber(standardHeaderArray, 0, mxAddField(standardHeaderArray, "ControlField"), controlField);

    // add 3 MAC addresses
    mxArray *addr1 = mxCreateNumericMatrix(1, 6, mxUINT8_CLASS, mxREAL);
    mxArray *addr2 = mxCreateNumericMatrix(1, 6, mxUINT8_CLASS, mxREAL);
    mxArray *addr3 = mxCreateNumericMatrix(1, 6, mxUINT8_CLASS, mxREAL);
    std::copy(std::begin(standardHeader.addr1), std::end(standardHeader.addr1), (uint8_T *)mxGetPr(addr1));
    std::copy(std::begin(standardHeader.addr2), std::end(standardHeader.addr2), (uint8_T *)mxGetPr(addr2));
    std::copy(std::begin(standardHeader.addr3), std::end(standardHeader.addr3), (uint8_T *)mxGetPr(addr3));
    mxSetFieldByNumber(standardHeaderArray, 0, mxAddField(standardHeaderArray, "Addr1"), addr1);
    mxSetFieldByNumber(standardHeaderArray, 0, mxAddField(standardHeaderArray, "Addr2"), addr2);
    mxSetFieldByNumber(standardHeaderArray, 0, mxAddField(standardHeaderArray, "Addr3"), addr3);

    // add frag and sequence
    mxSetFieldByNumber(standardHeaderArray, 0, mxAddField(standardHeaderArray, "Fragment"), createScalarMxArray(standardHeader.frag));
    mxSetFieldByNumber(standardHeaderArray, 0, mxAddField(standardHeaderArray, "Sequence"), createScalarMxArray(standardHeader.seq));

    return standardHeaderArray;
}

mxArray *convertPicoScenesHeader2MxArray(const PicoScenesFrameHeader &header) {
    auto *PicoScenesFrameHeaderArray = mxCreateStructMatrix(1, 1, 0, NULL);

    // add frag and sequence
    mxSetFieldByNumber(PicoScenesFrameHeaderArray, 0, mxAddField(PicoScenesFrameHeaderArray, "MagicValue"), createScalarMxArray(header.magicValue));
    mxSetFieldByNumber(PicoScenesFrameHeaderArray, 0, mxAddField(PicoScenesFrameHeaderArray, "Version"), createScalarMxArray(header.version));
    mxSetFieldByNumber(PicoScenesFrameHeaderArray, 0, mxAddField(PicoScenesFrameHeaderArray, "DeviceType"), createScalarMxArray(uint16_t(header.deviceType)));
    mxSetFieldByNumber(PicoScenesFrameHeaderArray, 0, mxAddField(PicoScenesFrameHeaderArray, "FrameType"), createScalarMxArray(header.frameType));
    mxSetFieldByNumber(PicoScenesFrameHeaderArray, 0, mxAddField(PicoScenesFrameHeaderArray, "TaskId"), createScalarMxArray(header.taskId));
    mxSetFieldByNumber(PicoScenesFrameHeaderArray, 0, mxAddField(PicoScenesFrameHeaderArray, "TxId"), createScalarMxArray(header.txId));

    return PicoScenesFrameHeaderArray;
}

mxArray *convertRxSBasic2MxArray(const RxSBasic &basic) {
    auto *rxsArray = mxCreateStructMatrix(1, 1, 0, NULL);
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "DeviceType"), createScalarMxArray(basic.deviceType));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "Timestamp"), createScalarMxArray(basic.tstamp));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "SystemTime"), createScalarMxArray(basic.systemTime));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "CenterFreq"), createScalarMxArray(basic.centerFreq));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "CenterFreq2"), createScalarMxArray(basic.centerFreq2));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "ControlFreq"), createScalarMxArray(basic.controlFreq));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "CBW"), createScalarMxArray(basic.cbw));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "PacketFormat"), createScalarMxArray(basic.packetFormat));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "PacketCBW"), createScalarMxArray(basic.pkt_cbw));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "GI"), createScalarMxArray(basic.guardInterval));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "MCS"), createScalarMxArray(basic.mcs));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "NumSTS"), createScalarMxArray(basic.numSTS));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "NumESS"), createScalarMxArray(basic.numESS));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "NumRx"), createScalarMxArray(basic.numRx));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "NoiseFloor"), createScalarMxArray(basic.noiseFloor));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "RSSI"), createScalarMxArray(basic.rssi));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "RSSI1"), createScalarMxArray(basic.rssi_ctl0));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "RSSI2"), createScalarMxArray(basic.rssi_ctl1));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "RSSI3"), createScalarMxArray(basic.rssi_ctl2));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "RSSI4"), createScalarMxArray(basic.rssi_ctl3));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "RSSI5"), createScalarMxArray(basic.rssi_ctl4));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "RSSI6"), createScalarMxArray(basic.rssi_ctl5));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "RSSI7"), createScalarMxArray(basic.rssi_ctl6));
    mxSetFieldByNumber(rxsArray, 0, mxAddField(rxsArray, "RSSI8"), createScalarMxArray(basic.rssi_ctl7));

    return rxsArray;
}

void transformExtraInfo2MatlabStruct(const ExtraInfo &ei, mxArray *mxExtraInfo) {
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasLength"), createScalarMxArray(ei.hasLength));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasVersion"), createScalarMxArray(ei.hasVersion));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasMacAddr_cur"), createScalarMxArray(ei.hasMacAddr_cur));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasMacAddr_rom"), createScalarMxArray(ei.hasMacAddr_rom));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasChansel"), createScalarMxArray(ei.hasChansel));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasBMode"), createScalarMxArray(ei.hasBMode));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasEVM"), createScalarMxArray(ei.hasEVM));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasTxChainMask"), createScalarMxArray(ei.hasTxChainMask));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasRxChainMask"), createScalarMxArray(ei.hasRxChainMask));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasTxpower"), createScalarMxArray(ei.hasTxpower));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasCFO"), createScalarMxArray(ei.hasCFO));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasTxTSF"), createScalarMxArray(ei.hasTxTSF));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasLastHwTxTSF"), createScalarMxArray(ei.hasLastHWTxTSF));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasChannelFlags"), createScalarMxArray(ei.hasChannelFlags));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasTxNess"), createScalarMxArray(ei.hasTxNess));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasTuningPolicy"), createScalarMxArray(ei.hasTuningPolicy));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasPLLRate"), createScalarMxArray(ei.hasPLLRate));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasPLLClkSel"), createScalarMxArray(ei.hasPLLClkSel));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasPLLRefDiv"), createScalarMxArray(ei.hasPLLRefDiv));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasAGC"), createScalarMxArray(ei.hasAGC));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasAntennaSelection"), createScalarMxArray(ei.hasAntennaSelection));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasSamplingRate"), createScalarMxArray(ei.hasSamplingRate));
    mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "HasTemperature"), createScalarMxArray(ei.hasTemperature));


    if (ei.hasLength) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "Length"), createScalarMxArray(ei.length));
    }
    if (ei.hasVersion) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "Version"), createScalarMxArray(ei.version));
    }
    if (ei.hasMacAddr_cur) {
        mxArray *macaddr_cur = mxCreateNumericMatrix(1, 3, mxUINT8_CLASS, mxREAL);
        auto *ptrR = (uint8_T *)mxGetPr(macaddr_cur);
        ptrR[0] = ei.macaddr_cur[0];
        ptrR[1] = ei.macaddr_cur[1];
        ptrR[2] = ei.macaddr_cur[2];
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "MACAddressCurrent"), macaddr_cur);
    }
    if (ei.hasMacAddr_rom) {
        mxArray *macaddr_rom = mxCreateNumericMatrix(1, 3, mxUINT8_CLASS, mxREAL);
        auto *ptrR = (uint8_T *)mxGetPr(macaddr_rom);
        ptrR[0] = ei.macaddr_rom[0];
        ptrR[1] = ei.macaddr_rom[1];
        ptrR[2] = ei.macaddr_rom[2];
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "MACAddressROM"), macaddr_rom);
    }
    if (ei.hasChansel) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "CHANSEL"), createScalarMxArray(ei.chansel));
    }
    if (ei.hasBMode) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "BMode"), createScalarMxArray(ei.bmode));
    }
    if (ei.hasEVM) {
        auto *evmMxArray = mxCreateNumericMatrix(1, 18, mxINT8_CLASS, mxREAL);
        auto *ptrR = (int8_T *)mxGetPr(evmMxArray);
        for (auto i = 0; i < 18; i++) {
            ptrR[i] = ei.evm[i];
        }
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "EVM"), evmMxArray);
    }
    if (ei.hasTxChainMask) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "TxChainMask"), createScalarMxArray(ei.txChainMask));
    }
    if (ei.hasRxChainMask) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "RxChainMask"), createScalarMxArray(ei.rxChainMask));
    }
    if (ei.hasTxpower) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "TxPower"), createScalarMxArray(ei.txpower));
    }
    if (ei.hasCF) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "CF"), createScalarMxArray(ei.cf));
    }
    if (ei.hasTxTSF) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "TxTSF"), createScalarMxArray(ei.txTSF));
    }
    if (ei.hasLastHWTxTSF) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "LastTXTSF"), createScalarMxArray(ei.lastHwTxTSF));
    }
    if (ei.hasChannelFlags) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "ChannelFlags"), createScalarMxArray(ei.channelFlags));
    }
    if (ei.hasTxNess) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "TXNESS"), createScalarMxArray(ei.tx_ness));
    }
    if (ei.hasTuningPolicy) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "TuningPolicy"), createScalarMxArray(uint8_t(ei.tuningPolicy)));
    }
    if (ei.hasPLLRate) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "PLLRate"), createScalarMxArray(ei.pll_rate));
    }
    if (ei.hasPLLClkSel) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "PLLClockSelect"), createScalarMxArray(ei.pll_clock_select));
    }
    if (ei.hasPLLRefDiv) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "PLLRefDiv"), createScalarMxArray(ei.pll_refdiv));
    }
    if (ei.hasAGC) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "AGC"), createScalarMxArray(ei.agc));
    }
    if (ei.hasAntennaSelection) {
        auto *antSelMxArray = mxCreateNumericMatrix(1, 3, mxUINT8_CLASS, mxREAL);
        auto *ptrR = (int8_T *)mxGetPr(antSelMxArray);
        for (auto i = 0; i < 3; i++) {
            ptrR[i] = ei.ant_sel[i];
        }
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "ANTSEL"), antSelMxArray);
    }
    if (ei.hasSamplingRate) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "SF"), createScalarMxArray(ei.samplingRate));
    }
    if (ei.hasCFO) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "CFO"), createScalarMxArray(double(ei.cfo)));
    }
    if (ei.hasSFO) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "SFO"), createScalarMxArray(double(ei.sfo)));
    }
    if (ei.hasTemperature) {
        mxSetFieldByNumber(mxExtraInfo, 0, mxAddField(mxExtraInfo, "Temperature"), createScalarMxArray(double(ei.temperature)));
    }
}

mxArray *convertExtraInfo2MxArray(const ExtraInfo &ei) {
    auto *array = mxCreateStructMatrix(1, 1, 0, NULL);
    transformExtraInfo2MatlabStruct(ei, array);
    return array;
}

mxArray *convertCSISegment2MxArray(const CSISegment &csiSegment) {
    mxArray *groupCell = mxCreateStructMatrix(1, 1, 0, NULL);
    const auto &csi = csiSegment.getCSI();

    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "DeviceType"), createScalarMxArray(double(csi->deviceType)));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "FirmwareVersion"), createScalarMxArray(double(csi->firmwareVersion)));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "PacketFormat"), createScalarMxArray(double(csi->packetFormat)));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "CBW"), createScalarMxArray(double(csi->cbw)));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "CarrierFreq"), createScalarMxArray(double(csi->carrierFreq)));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "CarrierFreq2"), createScalarMxArray(double(csi->carrierFreq2)));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "IsMerged"), createScalarMxArray(double(csi->isMerged)));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "SamplingRate"), createScalarMxArray(double(csi->samplingRate)));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "SubcarrierBandwidth"), createScalarMxArray(double(csi->subcarrierBandwidth)));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "NumTones"), createScalarMxArray(csi->dimensions.numTones));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "NumTx"), createScalarMxArray(csi->dimensions.numTx));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "NumRx"), createScalarMxArray(csi->dimensions.numRx));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "NumESS"), createScalarMxArray(csi->dimensions.numESS));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "NumCSI"), createScalarMxArray(csi->dimensions.numCSI));
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "ANTSEL"), createScalarMxArray(csi->antSel));

    std::vector<mwSize> csiDataDimensions {csi->dimensions.numTones, mwSize(csi->dimensions.numTx + csi->dimensions.numESS), csi->dimensions.numRx, csi->dimensions.numCSI};
    // printf("%d-%d-%d-%d %d %d %d\n", csiDataDimensions[0], csiDataDimensions[1], csiDataDimensions[2], csiDataDimensions[3], csi.CSIArray.array.size(), csi.phaseArray.array.size(), csi.magnitudeArray.array.size());

    /*
    * csiData is essentially an N_{sc} x N_{sts} x N_{rx} x N_{CSI} 4-D matrix; The output order is the 1-D reshaped output format, like reshape(csiData, [], 1);
    */
    auto *CSIData = copyComplexData2MxArray<double, float>(&csi->CSIArray.array[0], csi->CSIArray.array.size(), csiDataDimensions.size(), csiDataDimensions.data());
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "CSI"), CSIData);

    mxArray *magData, *phaseData;
    // If CSI data is not interpolated, we need to manually compute magnitude and phase from CSI
    if (csi->magnitudeArray.array.size() != csi->CSIArray.array.size()) {
        std::vector<float> magValues(csi->CSIArray.array.size());
        std::vector<float> phaseValues(csi->CSIArray.array.size());
        
        for (size_t i = 0; i < csi->CSIArray.array.size(); i++) {
            magValues[i] = std::abs(csi->CSIArray.array[i]);
            phaseValues[i] = std::arg(csi->CSIArray.array[i]);
        }
        
        magData = copyData2MxArray<double, float>(magValues.data(), magValues.size(), csiDataDimensions.size(), csiDataDimensions.data());
        phaseData = copyData2MxArray<double, float>(phaseValues.data(), phaseValues.size(), csiDataDimensions.size(), csiDataDimensions.data());
    } else {
        magData = copyData2MxArray<double, float>(&csi->magnitudeArray.array[0], csi->magnitudeArray.array.size(), csiDataDimensions.size(), csiDataDimensions.data());
        phaseData = copyData2MxArray<double, float>(&csi->phaseArray.array[0], csi->phaseArray.array.size(), csiDataDimensions.size(), csiDataDimensions.data());
    }
    
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "Mag"), magData);
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "Phase"), phaseData);

    auto *indexData = copyData2MxArray<int16_t, int16_t>(&csi->subcarrierIndices[0], csi->subcarrierIndices.size());
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "SubcarrierIndex"), indexData);

    auto *timingOffsetData = copyData2MxArray<uint32_t, uint32_t>(csi->timingOffsets.data(), csi->timingOffsets.size());
    mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "TimingOffsets"), timingOffsetData);

    /*
    * Phase slope/intercept is an N_{sts} x N_{rx} x N_{CSI} 3-D matrix
    */
   if (!csi->phaseSlope.dimensions.empty()) {
        auto *phaseSlopeData = copyData2MxArray<double, float>(&csi->phaseSlope.array[0], csi->phaseSlope.array.size(), 3, csiDataDimensions.data() + 1);
        mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "PhaseSlope"), phaseSlopeData);
    } else {
        auto * emptyValue = mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL);
        mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "PhaseSlope"), emptyValue);
    }

    if (!csi->phaseIntercept.dimensions.empty()) {
        auto *phaseInterceptData = copyData2MxArray<double, float>(&csi->phaseIntercept.array[0], csi->phaseIntercept.array.size(), 3, csiDataDimensions.data() + 1);
        mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "PhaseIntercept"), phaseInterceptData);
    } else {
        auto * emptyValue = mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL);
        mxSetFieldByNumber(groupCell, 0, mxAddField(groupCell, "PhaseIntercept"), emptyValue);
    }

    return groupCell;
}

mxArray *convertFrameSegmentViaDynamicInterpretation(const AbstractPicoScenesFrameSegment& segment) {
    auto *segmentStruct = mxCreateStructMatrix(1, 1, 0, NULL);
    const auto dynamicType = DynamicContentTypeDictionary::getInstance()->queryType(segment.segmentName, segment.segmentVersionId);

    if (!dynamicType)
        return segmentStruct;

    for(const auto & field: dynamicType->fields) {
        const auto &fieldName = field.fieldName;
        const auto &fieldType = field.fieldType;
        const auto &fieldOffset = field.fieldOffset;
        const auto &arraySize = field.arraySize;

        if (fieldType == DynamicContentFieldPrimitiveType::Int8) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, int8_t>(segment.getDynamicInterpreter().getFieldPointer<int8_t>(fieldName), arraySize));
        } else if (fieldType == DynamicContentFieldPrimitiveType::Uint8) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, uint8_t>(segment.getDynamicInterpreter().getFieldPointer<uint8_t>(fieldName), arraySize));
        } else if (fieldType == DynamicContentFieldPrimitiveType::Int16) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, int16_t>(segment.getDynamicInterpreter().getFieldPointer<int16_t>(fieldName), arraySize));
        } else if (fieldType == DynamicContentFieldPrimitiveType::Uint16) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, uint16_t>(segment.getDynamicInterpreter().getFieldPointer<uint16_t>(fieldName), arraySize));
        } else if (fieldType == DynamicContentFieldPrimitiveType::Int32) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, int32_t>(segment.getDynamicInterpreter().getFieldPointer<int32_t>(fieldName), arraySize));
        } else if (fieldType == DynamicContentFieldPrimitiveType::Uint32) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, uint32_t>(segment.getDynamicInterpreter().getFieldPointer<uint32_t>(fieldName), arraySize));
        } else if (fieldType == DynamicContentFieldPrimitiveType::Int64) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, int64_t>(segment.getDynamicInterpreter().getFieldPointer<int64_t>(fieldName), arraySize));
        } else if (fieldType == DynamicContentFieldPrimitiveType::Uint64) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, uint64_t>(segment.getDynamicInterpreter().getFieldPointer<uint64_t>(fieldName), arraySize));
        } else if (fieldType == DynamicContentFieldPrimitiveType::Single) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, float>(segment.getDynamicInterpreter().getFieldPointer<float>(fieldName), arraySize));
        } else if (fieldType == DynamicContentFieldPrimitiveType::Double) {
            mxSetFieldByNumber(segmentStruct, 0, mxAddField(segmentStruct, fieldName.c_str()), copyData2MxArray<double, double>(segment.getDynamicInterpreter().getFieldPointer<double>(fieldName), arraySize));
        }
    }

    return segmentStruct;
}

mxArray *convertSDRExtra2MxArray(const SDRExtra &sdrExtra) {
    auto *sdrExtraArray = mxCreateStructMatrix(1, 1, 0, NULL);
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "ScramblerInit"), createScalarMxArray(sdrExtra.scramblerInit));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "PacketStartInternal"), createScalarMxArray(sdrExtra.packetStartInternal));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "PreciseRxSampleIndex"), createScalarMxArray(sdrExtra.hardwareRxSamplingIndex));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "PreciseRxTime"), createScalarMxArray(sdrExtra.preciseRxTime));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "LastTxTime"), createScalarMxArray(sdrExtra.lastTxTime));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "SignalInputTime"), createScalarMxArray(sdrExtra.signalInputSystemTime));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "SignalDecodingTime"), createScalarMxArray(sdrExtra.signalDecodeSystemTime));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "SIGEVM"), createScalarMxArray(sdrExtra.sigEVM));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "InitialCFO"), createScalarMxArray(sdrExtra.initialCFO));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "ResidualCFO"), createScalarMxArray(sdrExtra.residualCFO));
    mxSetFieldByNumber(sdrExtraArray, 0, mxAddField(sdrExtraArray, "DecodingDelay"), createScalarMxArray(sdrExtra.signalDecodeSystemTime - sdrExtra.signalInputSystemTime));

    return sdrExtraArray;
}

void convertPicoScenesFrame2Struct(ModularPicoScenesRxFrame &frame, mxArray *outCell, int index) {
    auto *standardHeaderArray = convertStandardHeader2MxArray(frame.standardHeader);
    mxSetFieldByNumber(outCell, index, mxAddField(outCell, "StandardHeader"), standardHeaderArray);

    auto *basicArray = convertRxSBasic2MxArray(frame.rxSBasicSegment->getBasic());
    mxSetFieldByNumber(outCell, index, mxAddField(outCell, "RxSBasic"), basicArray);

    auto *rxExtraInfoArray = convertExtraInfo2MxArray(frame.rxExtraInfoSegment->getExtraInfo());
    mxSetFieldByNumber(outCell, index, mxAddField(outCell, "RxExtraInfo"), rxExtraInfoArray);

    if (frame.mvmExtraSegment) {
        auto *mvmExtraArray = convertFrameSegmentViaDynamicInterpretation(*frame.mvmExtraSegment);
        mxSetFieldByNumber(outCell, index, mxAddField(outCell, "MVMExtra"), mvmExtraArray);
    }

    if (frame.sdrExtraSegment) {
        auto *sdrExtraArray = convertSDRExtra2MxArray(frame.sdrExtraSegment->getSdrExtra());
        mxSetFieldByNumber(outCell, index, mxAddField(outCell, "SDRExtra"), sdrExtraArray);
    }

    auto *rxCSIGroups = convertCSISegment2MxArray(*frame.csiSegment);
    mxSetFieldByNumber(outCell, index, mxAddField(outCell, "CSI"), rxCSIGroups);

    if (frame.PicoScenesHeader) {
        auto *header = convertPicoScenesHeader2MxArray(*frame.PicoScenesHeader);
        mxSetFieldByNumber(outCell, index, mxAddField(outCell, "PicoScenesHeader"), header);
    } else {
        mxSetFieldByNumber(outCell, index, mxAddField(outCell, "PicoScenesHeader"), mxCreateStructMatrix(1, 1, 0, NULL));
    }

    if (frame.txExtraInfoSegment) {
        auto *txExtraInfoArray = convertExtraInfo2MxArray(frame.txExtraInfoSegment->getExtraInfo());
        mxSetFieldByNumber(outCell, index, mxAddField(outCell, "TxExtraInfo"), txExtraInfoArray);
    } else {
        mxSetFieldByNumber(outCell, index, mxAddField(outCell, "TxExtraInfo"), mxCreateStructMatrix(1, 1, 0, NULL));
    }

    auto *txForeignSegments = mxCreateStructMatrix(1, 1, 0, NULL);
    for(const auto& txSegment: frame.txUnknownSegments) {
        auto *segArray = convertFrameSegmentViaDynamicInterpretation(*txSegment.second);
        mxSetFieldByNumber(txForeignSegments, 0, mxAddField(txForeignSegments, txSegment.first.c_str()), segArray);
    }
    mxSetFieldByNumber(outCell, index, mxAddField(outCell, "TxForeignSegments"), txForeignSegments);

    auto *rxForeignSegments = mxCreateStructMatrix(1, 1, 0, NULL);
    for(const auto& rxSegment: frame.rxUnknownSegments) {
        auto *segArray = convertFrameSegmentViaDynamicInterpretation(*rxSegment.second);
        mxSetFieldByNumber(rxForeignSegments, 0, mxAddField(rxForeignSegments, rxSegment.first.c_str()), segArray);
    }
    mxSetFieldByNumber(outCell, index, mxAddField(outCell, "RxForeignSegments"), rxForeignSegments);

    if (frame.legacyCSISegment) {
        auto *legacyRxCSIGroup = convertCSISegment2MxArray(*frame.legacyCSISegment);
        mxSetFieldByNumber(outCell, index, mxAddField(outCell, "LegacyCSI"), legacyRxCSIGroup);
    }

    if (frame.basebandSignalSegment) {
        auto *basebandSignal = convertBasebandSignal2MxArray(*frame.basebandSignalSegment);
        mxSetFieldByNumber(outCell, index, mxAddField(outCell, "BasebandSignals"), basebandSignal);
    }

    if (!frame.mpdus.empty()) {
        auto *mpduData = copyData2MxArray<uint8_t, uint8_t, true>(frame.mpdus[0].data(), frame.mpdus[0].size());
            mxSetFieldByNumber(outCell, index, mxAddField(outCell, "MPDU"), mpduData);
    } else {
        mxSetFieldByNumber(outCell, index, mxAddField(outCell, "MPDU"), copyData2MxArray<uint8_t, uint8_t, true>(nullptr, 0));
    }
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    /* check for proper number of arguments */
    if (nrhs != 2) {
        mexErrMsgIdAndTxt("Wi-Fi Sensing Toolbox:read_csi:nrhs", "Two inputs required: byte buffer and interpolation flag.");
    }
    if (nlhs != 1) {
        mexErrMsgIdAndTxt("Wi-Fi Sensing Toolbox:read_csi:nlhs", "One output required.");
    }
    /* make sure the input argument is a char array */
    if (!mxIsClass(prhs[0], "uint8")) {
        mexErrMsgIdAndTxt("Wi-Fi Sensing Toolbox:read_csi:notBytes", "Input must be a char array");
    }
    if (!mxIsLogicalScalar(prhs[1])) {
        mexErrMsgIdAndTxt("Wi-Fi Sensing Toolbox:read_csi:notLogical", "Interpolation flag must be a logical scalar");
    }

    IntelMVMParsedCSIHeader::registerDefaultMVMHeaderInterpretation();
#ifdef CUSTOM_HEADER_MAPPING_EXISTS
    CustomHeaderMapping::registerPrivateInterpretations();
#endif
    uint8_T *inBytes = (uint8_T *)mxGetData(prhs[0]);
    bool doInterpolation = mxIsLogicalScalarTrue(prhs[1]);
    auto bufferLength = mxGetNumberOfElements(prhs[0]);
    if (auto frame = ModularPicoScenesRxFrame::fromBuffer(inBytes, bufferLength, doInterpolation)) {
        // std::stringstream ss;
        // ss << *frame;
        // mexPrintf("rxframe: %s\n", ss.str().c_str());
        mxArray *result;
        if (auto echoProbeReplyIt = std::find_if(frame->payloadSegments.cbegin(), frame->payloadSegments.cend(), [](const std::shared_ptr<PayloadSegment> &payloadSegment) {
                return payloadSegment.get()->getPayloadData().payloadDescription == "EchoProbeReplyCSI" || payloadSegment.get()->getPayloadData().payloadDescription == "EchoProbeReplyFull";
            });
            echoProbeReplyIt != frame->payloadSegments.cend()) {
            result = mxCreateStructMatrix(2, 1, 0, NULL);
        } else {
            result = mxCreateStructMatrix(1, 1, 0, NULL);
        }

        convertPicoScenesFrame2Struct(*frame, result, 0);

        if (auto echoProbeCSIPayloadIt = std::find_if(frame->payloadSegments.cbegin(), frame->payloadSegments.cend(), [](const std::shared_ptr<PayloadSegment> &payloadSegment) {
                return payloadSegment.get()->getPayloadData().payloadDescription == "EchoProbeReplyCSI";
            });
            echoProbeCSIPayloadIt != frame->payloadSegments.cend()) {
            convertPicoScenesFrame2Struct(*frame, result, 1);  // fake the ack frame for the structual simplicity
            const auto &csiPayload = echoProbeCSIPayloadIt->get()->getPayloadData().payloadData;
            auto txCSISegment = CSISegment(csiPayload.data(), csiPayload.size());
            if (doInterpolation) {
                txCSISegment.getCSI()->removeCSDAndInterpolateCSI();
            }
            auto *rxCSIGroups = convertCSISegment2MxArray(txCSISegment);
            mxSetFieldByNumber(result, 1, mxAddField(result, "CSI"), rxCSIGroups);
        }

        if (auto echoProbeFullPacketIt = std::find_if(frame->payloadSegments.cbegin(), frame->payloadSegments.cend(), [](const std::shared_ptr<PayloadSegment> &payloadSegment) {
                return payloadSegment.get()->getPayloadData().payloadDescription == "EchoProbeReplyFull";
            });
            echoProbeFullPacketIt != frame->payloadSegments.cend()) {
            const auto &rxFrameBuffer = echoProbeFullPacketIt->get()->getPayloadData().payloadData;
            if (auto initiatingFrame = ModularPicoScenesRxFrame::fromBuffer(rxFrameBuffer.data(), rxFrameBuffer.size(), doInterpolation)) {
                convertPicoScenesFrame2Struct(*initiatingFrame, result, 1);
            }
        }

        plhs[0] = result;
    } else {
        mxArray *emptyArray = mxCreateStructMatrix(1, 1, 0, NULL);
        plhs[0] = emptyArray;
    }
}