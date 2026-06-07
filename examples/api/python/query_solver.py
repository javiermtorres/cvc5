#!/usr/bin/env python3
###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
##
#
# A demonstration of the QuerySolver API for enumerating satisfying instances
# and retrieving Skolem witnesses for open formulas.
#
# QuerySolver provides two main operations:
#  - findInstances(variable, formula): enumerates all values of `variable`
#    that satisfy `formula` (suitable for finite-domain types).
#  - findWitness(variable, formula): returns one satisfying value of
#    `variable` (suitable for any type, including infinite domains).
#

import cvc5
from cvc5 import Kind, TermManager, Solver, QuerySolver


def main():
    tm = TermManager()
    solver = Solver(tm)

    # QuerySolver requires produce-models and incremental mode.
    solver.setOption("produce-models", "true")
    solver.setOption("incremental", "true")

    qs = QuerySolver(solver)

    # -------------------------------------------------------------------------
    # Example 1: Enumerate all 4-bit bit-vector values less than 5
    # -------------------------------------------------------------------------
    bv_sort = tm.mkBitVectorSort(4)
    x = tm.mkConst(bv_sort, "x")
    five = tm.mkBitVector(4, 5)
    formula = tm.mkTerm(Kind.BITVECTOR_ULT, x, five)

    instances = qs.findInstances(x, formula)
    print("4-bit values of x satisfying (x < 5):")
    for v in instances:
        print(" ", v)
    # Expected: #b0000, #b0001, #b0010, #b0011, #b0100  (0 to 4)
    print(f"  (total: {len(instances)} instances)\n")

    # -------------------------------------------------------------------------
    # Example 2: Enumerate at most 3 integer witnesses for y > 100
    # -------------------------------------------------------------------------
    int_sort = tm.getIntegerSort()
    y = tm.mkConst(int_sort, "y")
    formula2 = tm.mkTerm(Kind.GT, y, tm.mkInteger(100))

    # max_instances=3 bounds the enumeration for this infinite-domain sort
    instances2 = qs.findInstances(y, formula2, max_instances=3)
    print("Up to 3 integer values of y satisfying (y > 100):")
    for v in instances2:
        print(" ", v)
    print()

    # -------------------------------------------------------------------------
    # Example 3: findWitness for an integer formula (Skolem-style)
    # -------------------------------------------------------------------------
    z = tm.mkConst(int_sort, "z")
    z_sq = tm.mkTerm(Kind.MULT, z, z)
    formula3 = tm.mkTerm(Kind.EQUAL, z_sq, tm.mkInteger(49))

    witness = qs.findWitness(z, formula3)
    print("Witness for z satisfying (z * z = 49):")
    if witness is None:
        print("  No witness found (formula is unsatisfiable).")
    else:
        print(f"  z = {witness}")
    print()

    # -------------------------------------------------------------------------
    # Example 4: findWitness returns None for an unsatisfiable formula
    # -------------------------------------------------------------------------
    w = tm.mkConst(int_sort, "w")
    formula4 = tm.mkTerm(
        Kind.AND,
        tm.mkTerm(Kind.GT, w, tm.mkInteger(0)),
        tm.mkTerm(Kind.LT, w, tm.mkInteger(0)),
    )

    witness4 = qs.findWitness(w, formula4)
    print("Witness for w satisfying (w > 0 AND w < 0):")
    if witness4 is None:
        print("  No witness found (formula is unsatisfiable).")
    else:
        print(f"  w = {witness4}")


if __name__ == "__main__":
    main()
