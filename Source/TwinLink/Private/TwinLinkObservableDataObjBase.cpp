// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkObservableDataObjBase.h"

void UTwinLinkObservableDataObjBase::BroadcastEvOnChanged() const {
    if (EvOnChanged.IsBound()) {
        EvOnChanged.Broadcast();
    }
}
