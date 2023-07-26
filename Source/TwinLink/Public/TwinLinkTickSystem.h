// Copyright (C) 2023, MLIT Japan. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TwinLinkSubSystemBase.h"
#include "TwinLinkTickSystem.generated.h"

// 遅延実行用のイベント
DECLARE_EVENT(UTwinLinkTickSystem, FDelDelayExec);

/**
 * TwinLinkのTick()に記述するような処理を扱うサブシステム
 * それぞれのサブシステムでTick()を実装すると実行順が分からず状況の再現性が落ちるため作成
 * イベント実行機能も実装
 */
UCLASS()
class TWINLINK_API UTwinLinkTickSystem : public UTwinLinkSubSystemBase, public FTickableGameObject
{
	GENERATED_BODY()

public:
    /**
     * @brief TwinLinkのサブシステムがTick()を利用するためのインターフェイス
    */
    class ITwinLinkTickable {
    public:
        virtual void Tick(float DeltaTime) = 0;
    };

public:

    /**
     * @brief
     * @param Collection
    */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /**
     * @brief
    */
    virtual void Deinitialize() override;

    // FTickableGameObjectからオーバーライド
    virtual TStatId GetStatId() const override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const  override { return bTickable; }

    /**  **/
    
    /**
     * @brief 遅延実行用
     * 使用例１ あるウィジェットのイベントで自身を削除する時　（直接削除を行うとそれ以降の動作が不安定になるため）
     * 
     * Broadcast(), Clear()関数は呼ばないでください
     * Add()系関数のみ利用してください
    */
    FDelDelayExec EvDelayExec;

public:
    /**
     * @brief サブシステムの追加
     * Tick()内でこの関数を呼び出してはいけない
     * @param SubSystem 
    */
    void AddSubSystem(ITwinLinkTickable* SubSystem);
    
    // この関数を実装する場合は一度 Remove候補リストに入れて Tick()の最後でRemoveする
    // TickableCollection走査中に要素を削除する可能性があるため
    //void RemoveSubSystem(int Priority, const ITwinLinkTickable* SubSystem);

private:
    /** サブシステムを追加出来るか **/
    bool bCanAddSubSystem;

    /** Tick()が呼べるか **/
    bool bTickable;

    /** Tick()内で実行する対象 **/
    TArray<ITwinLinkTickable*> TickableCollection;


};
