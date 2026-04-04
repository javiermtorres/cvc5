/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of the QuerySolver class of the C++ API.
 */

#include <cvc5/cvc5.h>
#include <gtest/gtest.h>

#include "test_api.h"

namespace cvc5::internal {

namespace test {

class TestApiBlackQuerySolver : public TestApi
{
 protected:
  void SetUp() override
  {
    TestApi::SetUp();
    d_solver->setOption("produce-models", "true");
    d_solver->setOption("incremental", "true");
  }
};

// findInstances on a finite domain (4-bit BV): all values < 5
TEST_F(TestApiBlackQuerySolver, findInstancesFinite)
{
  Sort bvSort = d_tm.mkBitVectorSort(4);
  Term x = d_tm.mkConst(bvSort, "x");
  Term five = d_tm.mkBitVector(4, 5);
  Term formula = d_tm.mkTerm(Kind::BITVECTOR_ULT, {x, five});

  QuerySolver qs(*d_solver);
  std::vector<Term> instances = qs.findInstances(x, formula);

  // Values 0,1,2,3,4 satisfy x < 5 for 4-bit BV
  ASSERT_EQ(instances.size(), 5u);
}

// findInstances returns empty vector when formula is unsatisfiable
TEST_F(TestApiBlackQuerySolver, findInstancesUnsat)
{
  Sort intSort = d_tm.getIntegerSort();
  Term x = d_tm.mkConst(intSort, "x");
  // x > 0 AND x < 0 is unsat
  Term formula = d_tm.mkTerm(
      Kind::AND,
      {d_tm.mkTerm(Kind::GT, {x, d_tm.mkInteger(0)}),
       d_tm.mkTerm(Kind::LT, {x, d_tm.mkInteger(0)})});

  QuerySolver qs(*d_solver);
  std::vector<Term> instances = qs.findInstances(x, formula);
  ASSERT_TRUE(instances.empty());
}

// findInstances respects maxInstances limit
TEST_F(TestApiBlackQuerySolver, findInstancesMaxInstances)
{
  Sort bvSort = d_tm.mkBitVectorSort(4);
  Term x = d_tm.mkConst(bvSort, "x");
  // formula is true for all 16 possible 4-bit values
  Term formula = d_tm.mkTrue();

  QuerySolver qs(*d_solver);
  std::vector<Term> instances = qs.findInstances(x, formula, 4);
  ASSERT_EQ(instances.size(), 4u);
}

// findWitness returns a satisfying value for a satisfiable formula
TEST_F(TestApiBlackQuerySolver, findWitnessSat)
{
  Sort intSort = d_tm.getIntegerSort();
  Term x = d_tm.mkConst(intSort, "x");
  // x > 100
  Term formula = d_tm.mkTerm(Kind::GT, {x, d_tm.mkInteger(100)});

  QuerySolver qs(*d_solver);
  Term witness = qs.findWitness(x, formula);

  ASSERT_FALSE(witness.isNull());
  // The returned value must itself be an integer > 100
  ASSERT_TRUE(witness.isIntegerValue());
  ASSERT_GT(std::stoll(witness.getIntegerValue()), 100LL);
}

// findWitness returns null term when formula is unsatisfiable
TEST_F(TestApiBlackQuerySolver, findWitnessUnsat)
{
  Sort intSort = d_tm.getIntegerSort();
  Term x = d_tm.mkConst(intSort, "x");
  // x > 0 AND x < 0 is unsat
  Term formula = d_tm.mkTerm(
      Kind::AND,
      {d_tm.mkTerm(Kind::GT, {x, d_tm.mkInteger(0)}),
       d_tm.mkTerm(Kind::LT, {x, d_tm.mkInteger(0)})});

  QuerySolver qs(*d_solver);
  Term witness = qs.findWitness(x, formula);
  ASSERT_TRUE(witness.isNull());
}

// findInstances does not pollute the solver's assertion stack
TEST_F(TestApiBlackQuerySolver, findInstancesPreservesState)
{
  Sort bvSort = d_tm.mkBitVectorSort(4);
  Term x = d_tm.mkConst(bvSort, "x");
  Term formula = d_tm.mkTrue();

  QuerySolver qs(*d_solver);
  qs.findInstances(x, formula);

  // After findInstances the outer context should still have no assertions,
  // so checkSat() should still be SAT (empty set of assertions).
  cvc5::Result r = d_solver->checkSat();
  ASSERT_TRUE(r.isSat());
}

// findWitness does not pollute the solver's assertion stack
TEST_F(TestApiBlackQuerySolver, findWitnessPreservesState)
{
  Sort intSort = d_tm.getIntegerSort();
  Term x = d_tm.mkConst(intSort, "x");
  Term formula = d_tm.mkTerm(Kind::GT, {x, d_tm.mkInteger(0)});

  QuerySolver qs(*d_solver);
  qs.findWitness(x, formula);

  // The outer context is still empty -> SAT
  cvc5::Result r = d_solver->checkSat();
  ASSERT_TRUE(r.isSat());
}

}  // namespace test
}  // namespace cvc5::internal
