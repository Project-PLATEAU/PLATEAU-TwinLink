// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "NavSystem/TwinLinkNavSystemParam.h"

float UTwinLinkNavSystemParam::GetMoveSpeedKmPerH(TwinLinkNavSystemMoveType MoveType) const {
    switch (MoveType) {
    case TwinLinkNavSystemMoveType::Walk:
        return WalkSpeedKmPerH;
    case TwinLinkNavSystemMoveType::Car:
        return CarSpeedKmPerH;
    case TwinLinkNavSystemMoveType::Max:
    case TwinLinkNavSystemMoveType::Undefined:
        break;
    default:
        break;
    }
    return 1.f;
}
