#include "common/standard_chunk_read_planner.h"

#include <algorithm>
#include <sstream>
#include <gtest/gtest.h>

#include "common/goal.h"
#include "common/MFSCommunication.h"
#include "unittests/chunk_type_constants.h"
#include "unittests/operators.h"
#include "unittests/plan_tester.h"

#define VERIFY_PLAN_FOR(planner, firstBlock, count) \
	ASSERT_EQ( \
		unittests::PlanTester::expectedAnswer(standard, firstBlock, count), \
		unittests::PlanTester::executePlan( \
				planner.buildPlanFor(firstBlock, count), \
				planner.partsToUse(), count))

class StandardReadPlannerTests : public testing::Test {
public:
	StandardReadPlannerTests() {
		auto partsUsedInTests = std::vector<ChunkType>{
				standard,
				xor_p_of_2, xor_1_of_2, xor_2_of_2,
				xor_p_of_3, xor_1_of_3, xor_2_of_3, xor_3_of_3,
				xor_p_of_6, xor_1_of_6, xor_2_of_6, xor_3_of_6, xor_4_of_6, xor_5_of_6, xor_6_of_6,
		};
		for (ChunkType part : partsUsedInTests) {
			scores[part] = 1.0;
		}
	}
protected:
	std::map<ChunkType, float> scores;

	void verifyPlanner(const std::vector<ChunkType>& availableParts) {
		for (ChunkType part : availableParts) {
			scores[part] = 1.0;
		}
		StandardChunkReadPlanner planner;
		planner.prepare(availableParts, scores);
		VERIFY_PLAN_FOR(planner, 0, 1); // blocks: 0
		VERIFY_PLAN_FOR(planner, 0, 2); // blocks: 0, 1
		VERIFY_PLAN_FOR(planner, 0, 3); // blocks: 0, 1, 2
		VERIFY_PLAN_FOR(planner, 0, 4); // blocks: 0, 1, 2, 3
		VERIFY_PLAN_FOR(planner, 1, 1); // blocks: 1
		VERIFY_PLAN_FOR(planner, 1, 2); // blocks: 1, 2
		VERIFY_PLAN_FOR(planner, 1, 3); // blocks: 1, 2, 3
		VERIFY_PLAN_FOR(planner, 1, 4); // blocks: 1, 2, 3, 4
		VERIFY_PLAN_FOR(planner, 2, 1); // blocks: 2
		VERIFY_PLAN_FOR(planner, 2, 2); // blocks: 2, 3
		VERIFY_PLAN_FOR(planner, 2, 3); // blocks: 2, 3, 4
		VERIFY_PLAN_FOR(planner, 2, 4); // blocks: 2, 3, 4, 5
		VERIFY_PLAN_FOR(planner, 3, 1); // blocks: 3
		VERIFY_PLAN_FOR(planner, 3, 2); // blocks: 3, 4
		VERIFY_PLAN_FOR(planner, 3, 3); // blocks: 3, 4, 5
		VERIFY_PLAN_FOR(planner, 3, 4); // blocks: 3, 4, 5, 6
		VERIFY_PLAN_FOR(planner, MFSBLOCKSINCHUNK - 4, 4); // last four blocks
		VERIFY_PLAN_FOR(planner, MFSBLOCKSINCHUNK - 3, 3); // last three blocks
		VERIFY_PLAN_FOR(planner, MFSBLOCKSINCHUNK - 2, 2); // last two blocks
		VERIFY_PLAN_FOR(planner, MFSBLOCKSINCHUNK - 1, 1); // last block
		VERIFY_PLAN_FOR(planner, MFSBLOCKSINCHUNK - 4, 1);
		VERIFY_PLAN_FOR(planner, MFSBLOCKSINCHUNK - 3, 1);
		VERIFY_PLAN_FOR(planner, MFSBLOCKSINCHUNK - 2, 1);
		VERIFY_PLAN_FOR(planner, 0, MFSBLOCKSINCHUNK); // all blocks
		VERIFY_PLAN_FOR(planner, 1, MFSBLOCKSINCHUNK - 1);
		VERIFY_PLAN_FOR(planner, 2, MFSBLOCKSINCHUNK - 2);
		VERIFY_PLAN_FOR(planner, 3, MFSBLOCKSINCHUNK - 3);
		VERIFY_PLAN_FOR(planner, 4, MFSBLOCKSINCHUNK - 4);
	}
};

TEST_F(StandardReadPlannerTests, ChoosePartsToUseStandard1) {
	std::vector<ChunkType> chunks{standard, standard, standard};
	std::vector<ChunkType> expected{standard};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseStandard2) {
	std::vector<ChunkType> chunks{standard, xor_1_of_2};
	std::vector<ChunkType> expected{standard};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseStandard3) {
	std::vector<ChunkType> chunks{standard, xor_p_of_3};
	std::vector<ChunkType> expected{standard};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseStandard4) {
	std::vector<ChunkType> chunks{standard, xor_1_of_2, xor_p_of_2};
	std::vector<ChunkType> expected{standard};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseXor1) {
	std::vector<ChunkType> chunks{xor_p_of_2, xor_1_of_2, xor_2_of_2};
	std::vector<ChunkType> expected{xor_1_of_2, xor_2_of_2};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseXor2) {
	std::vector<ChunkType> chunks{xor_1_of_2, xor_2_of_2};
	std::vector<ChunkType> expected{xor_1_of_2, xor_2_of_2};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseXor3) {
	std::vector<ChunkType> chunks{xor_p_of_3, xor_1_of_3, xor_2_of_3, xor_3_of_3};
	std::vector<ChunkType> expected{xor_1_of_3, xor_2_of_3, xor_3_of_3};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseXorParity1) {
	std::vector<ChunkType> chunks{xor_p_of_2, xor_2_of_2};
	std::vector<ChunkType> expected{xor_p_of_2, xor_2_of_2};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseXorParity2) {
	std::vector<ChunkType> chunks{xor_p_of_3, xor_1_of_3, xor_1_of_2, xor_p_of_2};
	std::vector<ChunkType> expected{xor_p_of_2, xor_1_of_2};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseHighestXor1) {
	std::vector<ChunkType> chunks{xor_1_of_2, xor_2_of_2, xor_1_of_3, xor_2_of_3, xor_3_of_3};
	std::vector<ChunkType> expected{xor_1_of_3, xor_2_of_3, xor_3_of_3};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseHighestXor2) {
	std::vector<ChunkType> chunks{xor_1_of_2, xor_2_of_2, xor_1_of_3, xor_2_of_3, xor_p_of_3};
	std::vector<ChunkType> expected{xor_1_of_2, xor_2_of_2};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseHighestXorParity1) {
	std::vector<ChunkType> chunks{xor_p_of_3, xor_1_of_3, xor_2_of_3, xor_1_of_2, xor_p_of_2};
	std::vector<ChunkType> expected{xor_p_of_3, xor_1_of_3, xor_2_of_3};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_EQ(expected, planner.partsToUse());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseImpossible1) {
	std::vector<ChunkType> chunks { };
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_FALSE(planner.isReadingPossible());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseImpossible2) {
	std::vector<ChunkType> chunks{xor_1_of_2, xor_2_of_3, xor_3_of_3};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_FALSE(planner.isReadingPossible());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseImpossible3) {
	std::vector<ChunkType> chunks{xor_1_of_6, xor_2_of_6, xor_3_of_6, xor_5_of_6, xor_6_of_6};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_FALSE(planner.isReadingPossible());
}

TEST_F(StandardReadPlannerTests, ChoosePartsToUseImpossible4) {
	std::vector<ChunkType> chunks{xor_p_of_6, xor_2_of_6, xor_3_of_6, xor_5_of_6, xor_6_of_6};
	StandardChunkReadPlanner planner;
	planner.prepare(chunks, scores);
	EXPECT_FALSE(planner.isReadingPossible());
}

TEST_F(StandardReadPlannerTests, GetPlanForStandard) {
	verifyPlanner({standard});
}

TEST_F(StandardReadPlannerTests, GetPlanForXorLevel2) {
	verifyPlanner({xor_1_of_2, xor_2_of_2});
}

TEST_F(StandardReadPlannerTests, GetPlanForXorLevel2WithoutPart2) {
	verifyPlanner({xor_1_of_2, xor_p_of_2});
}

TEST_F(StandardReadPlannerTests, GetPlanForXorLevel2WithoutPart1) {
	verifyPlanner({xor_2_of_2, xor_p_of_2});
}

TEST_F(StandardReadPlannerTests, GetPlanForXorLevel6WithoutPart3) {
	verifyPlanner({xor_1_of_6, xor_6_of_6, xor_2_of_6, xor_5_of_6, xor_p_of_6, xor_4_of_6});
}

TEST_F(StandardReadPlannerTests, GetPlanForXorLevel3) {
	verifyPlanner({xor_1_of_3, xor_2_of_3, xor_3_of_3});
}

TEST_F(StandardReadPlannerTests, GetPlanForXorLevel3WithoutPart2) {
	verifyPlanner({xor_1_of_3, xor_3_of_3, xor_p_of_3});
}

TEST_F(StandardReadPlannerTests, GetPlanForMaxXorLevel) {
	ChunkType::XorLevel level = kMaxXorLevel;
	std::vector<ChunkType> parts;
	for (ChunkType::XorPart part = 1; part <= level; ++part) {
		parts.push_back(ChunkType::getXorChunkType(level, part));
	}
	verifyPlanner(parts);
}
