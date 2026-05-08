#pragma once

namespace DebugHelper
{
	static void Print(const FString& Msg, int32 InKey = -1, const FColor& InColor = FColor::MakeRandomColor(), float InDuration = 9999.f)
	{
		if (GEngine)
		{
			// 防止被 console 命令 DisableAllScreenMessages 或某段代码关掉后看不到消息
			GEngine->bEnableOnScreenDebugMessages = true;

			GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, *Msg);

			UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		}
	}
}
