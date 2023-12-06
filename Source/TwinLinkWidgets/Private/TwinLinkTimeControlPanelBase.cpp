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
    const auto T = Now - FTimespan((SelectableDaySpan - 1), 0, 0, 0, 0);  // SelectableDaySpanが7の時  Now-6, Now-5,... ,Now-1,Now
    StartDateTime = FDateTime(T.GetYear(), T.GetMonth(), T.GetDay());
    CalcTimeParamter(Now);  // OffsetDateTime, SlideTimespan
    SetupWBP();
}

void UTwinLinkTimeControlPanelBase::SetSlideTimespan(float Step) {
    // 24時間以内
    const auto HoursF = FMath::Lerp(0.0, 24.0, Step); // 小数付き
    const auto Hours = FMath::FloorToInt(HoursF);

    SlideTimespan = FTimespan(Hours, 0, 0);
    
    OnChangedSelectDateTime(GetSelectedDateTime());

}

void UTwinLinkTimeControlPanelBase::SetRealTime() {
    const auto Now = FDateTime::Now();
    CalcTimeParamter(Now);
    OnChangedSelectDateTime(GetSelectedDateTime(), true);

}

void UTwinLinkTimeControlPanelBase::CalcTimeParamter(const FDateTime& InTime) {
    const auto Offset = FDateTime(InTime.GetYear(), InTime.GetMonth(), InTime.GetDay());
    const auto Step = NormalizeDateTimeAsDay(FTimespan(InTime.GetHour(), InTime.GetMinute(), InTime.GetSecond()));

    OffsetDateTime = Offset;

    // 24時間以内
    const auto HoursF = FMath::Lerp(0.0, 24.0, Step); // 小数付き
    const auto Hours = FMath::FloorToInt(HoursF);

    SlideTimespan = FTimespan(Hours, 0, 0);

}

void UTwinLinkTimeControlPanelBase::ActivateRealtimeMode(bool bIsActive) {
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    
    // 同じなので何もしない
    if (bIsRealtimeMode == bIsActive)
        return;

    if (bIsActive) {
        const bool bLoop = true;
        const float FirstDelay = 0.0f;
        const auto UpdateDateTimeFromNow = [this]() {
            SetRealTime();
            };
        TimerManager.SetTimer(RealTimeModeUpdateTimerHnd, UpdateDateTimeFromNow, RealtimeUpdateRate, bLoop, FirstDelay);
        SetRealTime();
    }
    else {
        TimerManager.ClearTimer(RealTimeModeUpdateTimerHnd);
    }
    bIsRealtimeMode = bIsActive;
    OnChangedRealtimeMode(bIsRealtimeMode);
}

void UTwinLinkTimeControlPanelBase::SetOffsetDateTime(const FDateTime& Offset) {
    OffsetDateTime = Offset;

    OnChangedSelectDateTime(GetSelectedDateTime());
}

void UTwinLinkTimeControlPanelBase::SetOffsetDateTimeFromStep(const float& Step) {
    const auto PassedDay = (int)(Step * (SelectableDaySpan - 1));   // 当日に合わせるため 
    OffsetDateTime = StartDateTime + FTimespan(PassedDay, 0, 0, 0);

    OnChangedSelectDateTime(GetSelectedDateTime());
}

FDateTime UTwinLinkTimeControlPanelBase::GetSelectedDateTime() const {
    FDateTime Ret = OffsetDateTime + SlideTimespan;
    const auto Now = FDateTime::Now();
    if (Ret >= Now) {
        Ret = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), Now.GetHour());
    }
    return Ret;
}

float UTwinLinkTimeControlPanelBase::GetOffsetDateAsNormalized() const {
    const auto Now = FDateTime::Now();
    const auto NowOffset = Now - StartDateTime;
    const auto OffsetDateFromStart = OffsetDateTime - (StartDateTime - FTimespan(1, 0, 0));
    const auto TimeSpan = FTimespan(SelectableDaySpan - 1, 0, 0, 0);
    const auto Step = (float)OffsetDateFromStart.GetTicks() / TimeSpan.GetTicks();
    return Step;
}

float UTwinLinkTimeControlPanelBase::GetOffsetTimeAsNormalized() const {
    const auto TimeSpan = FTimespan(24, 0, 0);
    const auto Step = (float)SlideTimespan.GetTicks() / TimeSpan.GetTicks();
    return Step;

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

double UTwinLinkTimeControlPanelBase::NormalizeDateTimeAsDay(const FTimespan& Date) const {
    const auto OneDay = FTimespan(1, 0, 0, 0);
    return (double)Date.GetTicks()/OneDay.GetTicks();
}