// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AbilitySystem/SkillConfigTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSkillFormulaBasicTest,
	"FrontendUI.Skill.Formula.Basic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillFormulaBasicTest::RunTest(const FString& Parameters)
{
	// 纯数字
	TestEqual("Pure number", FFormulaEvaluator::Evaluate(TEXT("42"), 1, 1), 42.f);

	// 简单加法
	TestEqual("Simple add", FFormulaEvaluator::Evaluate(TEXT("10 + 5"), 1, 1), 15.f);

	// 含 Level 变量
	TestEqual("Level variable L1", FFormulaEvaluator::Evaluate(TEXT("10 + 5 * Level"), 3, 1), 25.f);
	TestEqual("Level variable L2", FFormulaEvaluator::Evaluate(TEXT("10 + 5 * Level"), 1, 1), 15.f);

	// 含 Stack 变量
	TestEqual("Stack variable", FFormulaEvaluator::Evaluate(TEXT("5 * Stack"), 1, 3), 15.f);

	// 括号
	TestEqual("Parentheses", FFormulaEvaluator::Evaluate(TEXT("(10 + 5) * 2"), 1, 1), 30.f);

	// 减法
	TestEqual("Subtract", FFormulaEvaluator::Evaluate(TEXT("20 - Level * 3"), 2, 1), 14.f);

	// 空格处理
	TestEqual("Spaces", FFormulaEvaluator::Evaluate(TEXT("  20 - 5  "), 1, 1), 15.f);

	// 空字符串
	TestEqual("Empty", FFormulaEvaluator::Evaluate(TEXT(""), 1, 1), 0.f);

	// 除零保护
	TestEqual("Div by zero guard", FFormulaEvaluator::Evaluate(TEXT("10 / 0"), 1, 1), 10.f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSkillFormulaEdgeTest,
	"FrontendUI.Skill.Formula.EdgeCases",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillFormulaEdgeTest::RunTest(const FString& Parameters)
{
	// 负数
	TestEqual("Negative", FFormulaEvaluator::Evaluate(TEXT("-5 + 10"), 1, 1), 5.f);
	TestEqual("Negative result", FFormulaEvaluator::Evaluate(TEXT("5 - 10"), 1, 1), -5.f);

	// 复杂嵌套
	float Result = FFormulaEvaluator::Evaluate(TEXT("(10 + Level) * (2 + Stack)"), 2, 3);
	TestEqual("Nested parens", Result, 60.f); // (12) * (5) = 60

	// 大 Level 值
	TestEqual("Large level", FFormulaEvaluator::Evaluate(TEXT("Level * 100"), 99, 1), 9900.f);

	// 除法
	TestEqual("Division", FFormulaEvaluator::Evaluate(TEXT("100 / 4"), 1, 1), 25.f);

	// 混合运算优先级 乘除优于加减
	TestEqual("Precedence", FFormulaEvaluator::Evaluate(TEXT("2 + 3 * 4"), 1, 1), 14.f);

	return true;
}

#endif //WITH_DEV_AUTOMATION_TESTS
