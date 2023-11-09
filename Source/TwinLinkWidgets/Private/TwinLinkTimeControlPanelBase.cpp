// Copyright (C) 2023, MLIT Japan. All rights reserved.


#include "TwinLinkTimeControlPanelBase.h"

void UTwinLinkTimeControlPanelBase::NativeConstruct() {
    // BPよりも先に初期化　（基底クラスよりも先に初期化するので注意）
    //...

    Super::NativeConstruct();

    // BPよりも後に初期化
    //...
    Setup();

}

void UTwinLinkTimeControlPanelBase::NativeDestruct() {

    //...
    Super::NativeDestruct();
}

void UTwinLinkTimeControlPanelBase::Setup() {
    const auto Now = FDateTime::Now();
    const auto T = Now - FTimespan(6, 0, 0, 0, 0);  // Now-6, Now-5,... ,Now-1,Now
    StartDateTime = FDateTime(T.GetYear(), T.GetMonth(), T.GetDay());
    SetupWBP();
}

void UTwinLinkTimeControlPanelBase::SetSlideTimespan(float Step) {
    //// 24時間以内
    const auto HoursF = FMath::Lerp(0.0, 24.0, Step); // 小数付き
    const auto Hours = FMath::FloorToInt(HoursF);
    //const auto MinutesF = (HoursF - Hours) * 60.0;      // 小数付き
    //const auto Minutes = FMath::FloorToInt(MinutesF);
    //const auto SecondsF = (MinutesF - Minutes) * 60.0; // 小数付き
    //const auto Seconds = FMath::FloorToInt(SecondsF);

    SlideTimespan = FTimespan(Hours, 0, 0);
    
    OnChangedSelectDateTime(GetCurrentDateTime());

}

void UTwinLinkTimeControlPanelBase::ActivateRealtimeMode(bool bIsActive) {
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    
    // 同じなので何もしない
    if (bIsRealtimeMode == bIsActive)
        return;

    if (bIsActive) {
        const float Rate = 0.05f;
        const bool bLoop = true;
        const float FirstDelay = 0.0f;
        const auto UpdateDateTimeFromNow = [this]() {
            const auto Now = FDateTime::Now();
            SetOffsetDateTime(FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay()));
            const auto Step = NormalizeDateTimeAsDay(FTimespan(Now.GetHour(), Now.GetMinute(), Now.GetSecond()));
            SetSlideTimespan(Step);
            };
        TimerManager.SetTimer(RealTimeModeUpdateTimerHnd, UpdateDateTimeFromNow, Rate, bLoop, FirstDelay);
    }
    else {
        TimerManager.ClearTimer(RealTimeModeUpdateTimerHnd);
    }
    bIsRealtimeMode = bIsActive;
    OnChangedRealtimeMode(bIsRealtimeMode);
}

void UTwinLinkTimeControlPanelBase::SetOffsetDateTime(const FDateTime& Offset) {
    OffsetDateTime = Offset;

    OnChangedSelectDateTime(GetCurrentDateTime());
}

FDateTime UTwinLinkTimeControlPanelBase::GetCurrentDateTime() const {
    FDateTime Ret = OffsetDateTime + SlideTimespan;
    const auto Now = FDateTime::Now();
    if (Ret >= Now) {
        Ret = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), Now.GetHour());
    }
    return Ret;
}

const TArray<FDateTime> UTwinLinkTimeControlPanelBase::GetWeekCollection() const {
    const auto DaysOfWeek = 7;
    TArray<FDateTime> WeekCollection;   WeekCollection.Reserve(DaysOfWeek);

    const auto OneDay = FTimespan(1, 0, 0, 0);
    FDateTime Temp = StartDateTime;
    for (int i = 0; i < DaysOfWeek; i++) {
        WeekCollection.Add(Temp);
        Temp = Temp + OneDay;
    }

    return WeekCollection;
}

double UTwinLinkTimeControlPanelBase::NormalizeDateTimeAsDay(const FTimespan& Date) {
    const auto OneDay = FTimespan(1, 0, 0, 0);
    return (double)Date.GetTicks()/OneDay.GetTicks();
}