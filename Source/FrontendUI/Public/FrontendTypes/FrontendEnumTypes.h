#pragma once

UENUM(BlueprintType)
enum class EConfirmScreenType : uint8
{
    Ok,
    YesNo,
    OKCancel,
    Unknown UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EConfirmScreenButtonType : uint8
{
    Confirm,
    Cancelled,
    Closed,
    Unknown UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EStoryButtonType : uint8
{
    NewStory,
    Continue,
    NewGamePlus,
    Unknown UMETA(Hidden)
};