###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
##

import pytest
import cvc5
from cvc5 import Kind, TermManager, Solver, QuerySolver


@pytest.fixture
def tm():
    return TermManager()


@pytest.fixture
def solver(tm):
    s = Solver(tm)
    s.setOption("produce-models", "true")
    s.setOption("incremental", "true")
    return s


# findInstances on a finite domain (4-bit BV): all values < 5
def test_find_instances_finite(tm, solver):
    bv_sort = tm.mkBitVectorSort(4)
    x = tm.mkConst(bv_sort, "x")
    five = tm.mkBitVector(4, 5)
    formula = tm.mkTerm(Kind.BITVECTOR_ULT, x, five)

    qs = QuerySolver(solver)
    instances = qs.findInstances(x, formula)

    # Values 0,1,2,3,4 satisfy x < 5 for 4-bit BV
    assert len(instances) == 5


# findInstances returns empty list when formula is unsatisfiable
def test_find_instances_unsat(tm, solver):
    int_sort = tm.getIntegerSort()
    x = tm.mkConst(int_sort, "x")
    # x > 0 AND x < 0 is unsat
    formula = tm.mkTerm(
        Kind.AND,
        tm.mkTerm(Kind.GT, x, tm.mkInteger(0)),
        tm.mkTerm(Kind.LT, x, tm.mkInteger(0)),
    )

    qs = QuerySolver(solver)
    instances = qs.findInstances(x, formula)
    assert instances == []


# findInstances respects max_instances limit
def test_find_instances_max_instances(tm, solver):
    bv_sort = tm.mkBitVectorSort(4)
    x = tm.mkConst(bv_sort, "x")
    # formula is true for all 16 possible 4-bit values
    formula = tm.mkTrue()

    qs = QuerySolver(solver)
    instances = qs.findInstances(x, formula, max_instances=4)
    assert len(instances) == 4


# findWitness returns a satisfying value for a satisfiable formula
def test_find_witness_sat(tm, solver):
    int_sort = tm.getIntegerSort()
    x = tm.mkConst(int_sort, "x")
    formula = tm.mkTerm(Kind.GT, x, tm.mkInteger(100))

    qs = QuerySolver(solver)
    witness = qs.findWitness(x, formula)

    assert witness is not None
    assert witness.isIntegerValue()
    assert int(witness.getIntegerValue()) > 100


# findWitness returns None when formula is unsatisfiable
def test_find_witness_unsat(tm, solver):
    int_sort = tm.getIntegerSort()
    x = tm.mkConst(int_sort, "x")
    # x > 0 AND x < 0 is unsat
    formula = tm.mkTerm(
        Kind.AND,
        tm.mkTerm(Kind.GT, x, tm.mkInteger(0)),
        tm.mkTerm(Kind.LT, x, tm.mkInteger(0)),
    )

    qs = QuerySolver(solver)
    witness = qs.findWitness(x, formula)
    assert witness is None


# findInstances does not pollute the solver's assertion stack
def test_find_instances_preserves_state(tm, solver):
    bv_sort = tm.mkBitVectorSort(4)
    x = tm.mkConst(bv_sort, "x")
    formula = tm.mkTrue()

    qs = QuerySolver(solver)
    qs.findInstances(x, formula)

    # After findInstances the outer context should still have no assertions
    r = solver.checkSat()
    assert r.isSat()


# findWitness does not pollute the solver's assertion stack
def test_find_witness_preserves_state(tm, solver):
    int_sort = tm.getIntegerSort()
    x = tm.mkConst(int_sort, "x")
    formula = tm.mkTerm(Kind.GT, x, tm.mkInteger(0))

    qs = QuerySolver(solver)
    qs.findWitness(x, formula)

    # The outer context is still empty -> SAT
    r = solver.checkSat()
    assert r.isSat()


# findInstances on an uninterpreted sort uses the model-domain optimization
def test_find_instances_uninterpreted_sort_optimization(tm, solver):
    # Enable finite model finding so the uninterpreted sort gets a finite domain.
    solver.setOption("finite-model-find", "true")
    u_sort = tm.mkUninterpretedSort("U")
    x = tm.mkConst(u_sort, "x")
    a = tm.mkConst(u_sort, "a")
    b = tm.mkConst(u_sort, "b")

    # Assert a != b so the model has at least two domain elements.
    solver.assertFormula(tm.mkTerm(Kind.DISTINCT, a, b))

    # Formula: x = a OR x = b (both domain elements satisfy it)
    formula = tm.mkTerm(
        Kind.OR,
        tm.mkTerm(Kind.EQUAL, x, a),
        tm.mkTerm(Kind.EQUAL, x, b),
    )

    qs = QuerySolver(solver)
    instances = qs.findInstances(x, formula)

    # Both a and b should be reported as instances.
    assert len(instances) == 2
