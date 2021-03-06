#include "driver.h"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

class BasicTest : public ::testing::Test {
protected:
	void SetUp() override {}

	void TearDown() override {
		// Code here will be called immediately after each test
		// (right before the destructor).
	}
};

TEST_F(BasicTest, CompileExample) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "y := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + y -> x + y + z;\n"
									 "z -> 0;\n"
									 "}\n"
									 "}\n";
	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x := 50;\n"
										"y := 30;\n"
										"x + y -> x + y + z;\n"
										"z -> 0;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, NoMainModule) {
	std::string in = "module notmain {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "y := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + y -> x + y + z;\n"
									 "z -> 0;\n"
									 "}\n"
									 "}\n";

	driver drv;
	drv.parse_string(in);
	ASSERT_THROW(drv.Compile(), NoMainModuleException);
}

TEST_F(BasicTest, TwoModules) {
	std::string in = "module useless {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "y := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + y -> x + y + z;\n"
									 "z -> 0;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "y := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + y -> x + y + z;\n"
									 "z -> 0;\n"
									 "}\n"
									 "}\n";
	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x := 50;\n"
										"y := 30;\n"
										"x + y -> x + y + z;\n"
										"z -> 0;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, CompositionTest) {
	std::string in = "module Addition {\n"
									 "input: [x, y];\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "y -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: e;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "d = Addition(a, b);\n"
									 "e = Addition(d, c);\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C e\n"
										"a := 50;\n"
										"b := 30;\n"
										"c := 30;\n"
										"d -> e;\n"
										"c -> e;\n"
										"a -> d;\n"
										"b -> d;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, ReactionRateTest) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "y := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + y ->(2) x + y + z;\n"
									 "z -> 0;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x := 50;\n"
										"y := 30;\n"
										"x + y ->(2) x + y + z;\n"
										"z -> 0;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, ReactionRateZeros) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "y := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + y ->(2) x + y + z;\n"
									 "z ->(3) 0;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x := 50;\n"
										"y := 30;\n"
										"x + y ->(2) x + y + z;\n"
										"z ->(3) 0;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, ReactionRateTrailingZero) {

	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "y := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + z ->(2.00001) x;\n"
									 "z ->(3.0) 0;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x := 50;\n"
										"y := 30;\n"
										"x + z ->(2.00001) x;\n"
										"z ->(3) 0;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}
TEST_F(BasicTest, BiArrowInt) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x + y (3)<->(2) x + z;\n"
									 "x + y (3)<-> x + z;\n"
									 "x + y  <->(3) x + z;\n"
									 "x + y  <-> x + z;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x + y ->(2) x + z;\n"
										"x + z ->(3) x + y;\n"

										"x + y -> x + z;\n"
										"x + z ->(3) x + y;\n"

										"x + y ->(3) x + z;\n"
										"x + z -> x + y;\n"

										"x + y -> x + z;\n"
										"x + z -> x + y;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}
TEST_F(BasicTest, BiArrowDouble) {

	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x + y (3.32)<->(2.32) x + z;\n"
									 "x + y (3.32)<-> x + z;\n"
									 "x + y  <->(3.32) x + z;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x + y ->(2.32) x + z;\n"
										"x + z ->(3.32) x + y;\n"

										"x + y -> x + z;\n"
										"x + z ->(3.32) x + y;\n"

										"x + y ->(3.32) x + z;\n"
										"x + z -> x + y;\n";

	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}
TEST_F(BasicTest, BiArrowIntDouble) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x + y (3)<->(2.32) x + z;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x + y ->(2.32) x + z;\n"
										"x + z ->(3) x + y;\n";

	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}
TEST_F(BasicTest, BiArrowZeroInt) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x <-> 0;\n"
									 "x <->(2) 0;\n"
									 "x (2)<->(2) 0;\n"
									 "x (2)<-> 0;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x -> 0;\n"
										"0 -> x;\n"

										"x ->(2) 0;\n"
										"0 -> x;\n"

										"x ->(2) 0;\n"
										"0 ->(2) x;\n"

										"x -> 0;\n"
										"0 ->(2) x;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, BiArrowZeroDecimal) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x <-> 0;\n"
									 "x <->(2.1) 0;\n"
									 "x (2.2)<->(2.2) 0;\n"
									 "x (2.1)<-> 0;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x -> 0;\n"
										"0 -> x;\n"

										"x ->(2.1) 0;\n"
										"0 -> x;\n"

										"x ->(2.2) 0;\n"
										"0 ->(2.2) x;\n"

										"x -> 0;\n"
										"0 ->(2.1) x;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, BiArrowZeroComp) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x (2.2)<->(2) 0;\n"
									 "x (2)<->(2.2) 0;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"

										"x ->(2) 0;\n"
										"0 ->(2.2) x;\n"

										"x ->(2.2) 0;\n"
										"0 ->(2) x;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, InputConcException) {
	std::string in = "module Addition {\n"
									 "input: x;\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 5;\n"
									 "}\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: a;\n"
									 "output: b;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "}\n"
									 "compositions: {\n"
									 "b = Addition(a);\n"
									 "}\n"
									 "}\n";

	driver drv;
	ASSERT_THROW(drv.parse_string(in), InputSpecieConcException);
}

TEST_F(BasicTest, MultSpeciesInReact) {
	std::string in = "module main {\n"
									 "private: x;\n"
									 "output: y;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "}\n"
									 "reactions: {\n"
									 "2x -> y;\n"
									 "}\n"
									 "}\n";
	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C y\n"
										"x := 50;\n"
										"2x -> y;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, MultSpeciesInReact2) {
	std::string in = "module main {\n"
									 "private: x;\n"
									 "output: y;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + x + x -> y;\n"
									 "}\n"
									 "}\n";
	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C y\n"
										"x := 50;\n"
										"3x -> y;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, CompWrongInputSize) {
	std::string in = "module Addition {\n"
									 "input: x;\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b];\n"
									 "output: c;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 50;\n"
									 "}\n"
									 "compositions: {\n"
									 "c = Addition(a, b);\n"
									 "}\n"
									 "}\n";

	driver drv;
	ASSERT_THROW(drv.parse_string(in), CompositionException);
}

TEST_F(BasicTest, CompWrongOutputSize) {
	std::string in = "module Addition {\n"
									 "input: x;\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: a;\n"
									 "output: [b, c];\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "}\n"
									 "compositions: {\n"
									 "b, c = Addition(a);\n"
									 "}\n"
									 "}\n";

	driver drv;
	ASSERT_THROW(drv.parse_string(in), CompositionException);
}

TEST_F(BasicTest, NestedComposition) {
	std::string input = "module thing {\n"
											"input: c;\n"
											"output: j;\n"
											"private: chem;\n"
											"concentrations: {\n"
											"chem := 420;\n"
											"}\n"
											"}\n"
											"module some {\n"
											"input: x;\n"
											"private: aa;\n"
											"output: b;\n"
											"concentrations: {\n"
											"aa := 20;\n"
											"}\n"
											"compositions: {\n"
											"b = thing(aa);\n"
											"}\n"
											"}\n"
											"module main {\n"
											"input: b;\n"
											"output: z;\n"
											"private: a;\n"
											"concentrations: {\n"
											"a := 1337;\n"
											"}\n"
											"compositions: {\n"
											"z = some(a);\n"
											"}\n"
											"}\n";
	driver drv;
	drv.parse_string(input);
	std::string expected = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
												 "a := 1337;\n"
												 "some_0_aa := 20;\n"
												 "some_0_thing_0_chem := 420;\n";
	EXPECT_EQ(drv.Compile(), expected);
}

TEST_F(BasicTest, privateSpecMapOneSubMod) {
	std::string in = "module Addition {\n"
									 "input: x;\n"
									 "private: y;\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "y := 20;\n"
									 "}\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "y -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: e;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "d = Addition(a);\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C e\n"
										"Addition_0_y := 20;\n"
										"a := 50;\n"
										"b := 30;\n"
										"c := 30;\n"
										"a -> d;\n"
										"Addition_0_y -> d;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, privateSpecMapTwoSubMod) {
	std::string in = "module AdditionTwo {\n"
									 "input: x;\n"
									 "private: y;\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "y := 20;\n"
									 "}\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "y -> z;\n"
									 "}\n"
									 "}\n"
									 "module Addition {\n"
									 "input: x;\n"
									 "private: y;\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "y := 20;\n"
									 "}\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "y -> z;\n"
									 "}\n"
									 "compositions: {\n"
									 "z = AdditionTwo(y);\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: e;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "d = Addition(a);\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C e\n"
										"Addition_0_AdditionTwo_0_y := 20;\n"
										"Addition_0_y := 20;\n"
										"a := 50;\n"
										"b := 30;\n"
										"c := 30;\n"
										"a -> d;\n"
										"Addition_0_y -> d;\n"
										"Addition_0_y -> d;\n"
										"Addition_0_AdditionTwo_0_y -> d;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, privateSpecSubModReacConc) {
	std::string in = "module Addition {\n"
									 "input: x;\n"
									 "private: y;\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "y := 20;\n"
									 "}\n"
									 "reactions: {\n"
									 "5x -> z;\n"
									 "10y -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: e;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "d = Addition(a);\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C e\n"
										"Addition_0_y := 20;\n"
										"a := 50;\n"
										"b := 30;\n"
										"c := 30;\n"
										"5a -> d;\n"
										"10Addition_0_y -> d;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, outputSpecInSubModConc) {
	std::string in = "module Addition {\n"
									 "input: x;\n"
									 "private: y;\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "y := 20;\n"
									 "z := 20;\n"
									 "}\n"
									 "reactions: {\n"
									 "5x -> z;\n"
									 "10y -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: e;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "d = Addition(a);\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C e\n"
										"Addition_0_y := 20;\n"
										"a := 50;\n"
										"b := 30;\n"
										"c := 30;\n"
										"d := 20;\n"
										"5a -> d;\n"
										"10Addition_0_y -> d;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, UsingInputSpecieAsOutputOfCompModuleException) {
	std::string in = "module AdditionTwo {\n"
									 "input: x;\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "z := 20;\n"
									 "}\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "}\n"
									 "}\n"
									 "module Addition {\n"
									 "input: x;\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "}\n"
									 "compositions: {\n"
									 "x = AdditionTwo(z);\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b];\n"
									 "output: e;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "b = Addition(a);\n"
									 "}\n"
									 "}\n";

	driver drv;
	drv.parse_string(in);
	ASSERT_THROW(drv.Compile(), MapConcForSubModuleException);
}

TEST_F(BasicTest, multipleOutputSpecie) {
	std::string in = "module main {\n"
									 "private: [a, b, c];\n"
									 "output: [e, f];\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "a + b -> a + b + e;\n"
									 "a + e -> a + e + f;\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C e,f\n"
										"a := 50;\n"
										"b := 30;\n"
										"c := 30;\n"
										"a + b -> a + b + e;\n"
										"a + e -> a + e + f;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, multipleOutputSpecieWithSubMod) {
	std::string in = "module Addition {\n"
									 "input: x;\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "5x -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b, c];\n"
									 "output: [e, f];\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "d = Addition(a);\n"
									 "e = Addition(b);\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C e,f\n"
										"a := 50;\n"
										"b := 30;\n"
										"c := 30;\n"
										"5b -> e;\n"
										"5a -> d;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, FuzzyLogicTest) {
	std::string in = "module ident {\n"
									 "input: x;\n"
									 "output: y;\n"
									 "reactions: {\n"
									 "x -> x + y;\n"
									 "y -> 0;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "a := 5;\n"
									 "b := 0;\n"
									 "}\n"
									 "compositions: {\n"
									 "if (b) {\n"
									 "z = ident(a);\n"
									 "}\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"a := 5;\n"
										"b := 0;\n"
										"a + b -> a + b + z;\n"
										"b + z -> b;\n";

	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, ReactionCoefficient) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "y := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + y -> 2x;\n"
									 "z -> 0;\n"
									 "}\n"
									 "}\n";
	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"x := 50;\n"
										"y := 30;\n"
										"x + y -> 2x;\n"
										"z -> 0;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, ReactionCoefficientDecimal) {
	std::string in = "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x + y -> 2.2x;\n"
									 "x + y <-> 2.3x"

									 "x + y ->(1.2) 2.2x;\n"
									 "x + y (2)<->(2) 2.3x;\n"
									 "x + y (2)<-> 2.3x;\n"
									 "x + y <->(2) 2.3x;\n"
									 "}\n"
									 "}\n";
	driver drv;
	EXPECT_NE(drv.parse_string(in), 0);
}

// TODO: This unit test depends on where you execute it, which is bad
TEST_F(BasicTest, IsReentrant) {
	std::string in = "import chemlib/oscillator.chem;\n"
									 "module main {\n"
									 "private: [x, y];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "x := 50;\n"
									 "y := 30;\n"
									 "}\n"
									 "reactions: {\n"
									 "x + y -> x + y + z;\n"
									 "z -> 0;\n"
									 "}\n"
									 "}\n";

	driver drv;
	drv.parse_string(in);
	drv.Compile();
}

TEST_F(BasicTest, NestedIncludes) {
	std::string in = "import tests/chemfiles/include1.chem;\n"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "a := 3;\n"
									 "b := 3;\n"
									 "c := 3;\n"
									 "d := 3;\n"
									 "}\n"
									 "compositions: {"
									 "z = MulAdditions(a, b, c, d);"
									 "}}";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"a := 3;\n"
										"b := 3;\n"
										"c := 3;\n"
										"d := 3;\n"
										"MulAdditions_0_x + MulAdditions_0_y -> MulAdditions_0_x + "
										"MulAdditions_0_y + z;\n"
										"z -> 0;\n"
										"c -> MulAdditions_0_y + c;\n"
										"d -> MulAdditions_0_y + d;\n"
										"MulAdditions_0_y -> 0;\n"
										"a -> MulAdditions_0_x + a;\n"
										"b -> MulAdditions_0_x + b;\n"
										"MulAdditions_0_x -> 0;\n";

	driver drv;
	drv.parse_string(in);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, NoArgComp) {
	std::string in = "module ReturnFive { \n"
									 "output: x;\n"
									 "private: y;\n"
									 "concentrations: { y := 5; }\n"
									 "reactions: { y -> y + x; x -> 0; }\n"
									 "}\n"
									 "module main {\n"
									 "output: v;\n"
									 "compositions: { v = ReturnFive(); } }";

	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C v\n"
										"ReturnFive_0_y := 5;\n"
										"ReturnFive_0_y -> ReturnFive_0_y + v;\n"
										"v -> 0;\n";
	ASSERT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, MultipleIncludes) {
	std::string in = "import chemlib/oscillator.chem;\n"
									 "import tests/chemfiles/include1.chem;\n"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "a := 3;\n"
									 "b := 3;\n"
									 "c := 3;\n"
									 "d := 3;\n"
									 "}\n"
									 "compositions: {"
									 "z = MulAdditions(a, b, c, d);"
									 "}}";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C z\n"
										"a := 3;\n"
										"b := 3;\n"
										"c := 3;\n"
										"d := 3;\n"
										"MulAdditions_0_x + MulAdditions_0_y -> MulAdditions_0_x + "
										"MulAdditions_0_y + z;\n"
										"z -> 0;\n"
										"c -> MulAdditions_0_y + c;\n"
										"d -> MulAdditions_0_y + d;\n"
										"MulAdditions_0_y -> 0;\n"
										"a -> MulAdditions_0_x + a;\n"
										"b -> MulAdditions_0_x + b;\n"
										"MulAdditions_0_x -> 0;\n";

	driver drv;
	drv.parse_string(in);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, SmallRate) {
	std::string in = "module main {\n"
									 "private: a;\n"
									 "output: b;\n"
									 "reactions: { a ->(0.00000001) b; }}";

	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C b\n"
										"a ->(0.00000001) b;\n";

	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, ScaleRateTest) {
	std::string in = "module Addition {\n"
									 "input: [x, y];\n"
									 "output: z;\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "y -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: e;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "scale (10) {\n"
									 "d = Addition(a, b);\n"
									 "e = Addition(d, c);\n"
									 "}\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C e\n"
										"a := 50;\n"
										"b := 30;\n"
										"c := 30;\n"
										"a ->(10) d;\n"
										"b ->(10) d;\n"
										"d ->(10) e;\n"
										"c ->(10) e;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, ScaleRateTestDown) {
	std::string in = "module Addition {\n"
									 "input: [x, y];\n"
									 "output: z;\n"
									 "private: a;\n"
									 "concentrations: {\n"
									 "a := 5;\n"
									 "}\n"
									 "reactions: {\n"
									 "x -> z;\n"
									 "y -> z;\n"
									 "}\n"
									 "}\n"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: e;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "scale (0.1) {\n"
									 "d = Addition(a, b);\n"
									 "e = Addition(d, c);\n"
									 "}\n"
									 "}\n"
									 "}\n";

	std::string out = "#!/usr/bin/env -S crnsimul -e -P -C e\n"
										"Addition_0_a := 5;\n"
										"Addition_1_a := 5;\n"
										"a := 50;\n"
										"b := 30;\n"
										"c := 30;\n"
										"a ->(0.1) d;\n"
										"b ->(0.1) d;\n"
										"d ->(0.1) e;\n"
										"c ->(0.1) e;\n";
	driver drv;
	ASSERT_EQ(drv.parse_string(in), 0);
	EXPECT_EQ(drv.Compile(), out);
}

TEST_F(BasicTest, CompErrorTest) {
	std::string in = "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: e;\n"
									 "concentrations: {\n"
									 "a := 50;\n"
									 "b := 30;\n"
									 "c := 30;\n"
									 "}\n"
									 "compositions: {\n"
									 "d = Addition(a, b);\n"
									 "e = Addition(d, c);\n"
									 "}\n"
									 "}\n";

	driver drv;
	ASSERT_THROW(drv.parse_string(in), NoSuchModuleException);
}

TEST_F(BasicTest, MultipleModulesWithSameName) {
	std::string in = "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "a := 3;\n"
									 "b := 3;\n"
									 "c := 3;\n"
									 "d := 3;\n"
									 "}\n"
									 "}"
									 "module main {\n"
									 "private: [a, b, c, d];\n"
									 "output: z;\n"
									 "concentrations: {\n"
									 "a := 3;\n"
									 "b := 3;\n"
									 "c := 3;\n"
									 "d := 3;\n"
									 "}\n"
									 "}";

	driver drv;
	// drv.parse_string(in);
	EXPECT_THROW(drv.parse_string(in), MultipleModulesWithSameName);
}