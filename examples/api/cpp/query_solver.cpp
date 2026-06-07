/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * A demonstration of the QuerySolver API for enumerating satisfying instances
 * and retrieving Skolem witnesses for open formulas.
 *
 * QuerySolver provides two main operations:
 *  - findInstances(variable, formula): enumerates all values of `variable`
 *    that satisfy `formula` (suitable for finite-domain types).
 *  - findWitness(variable, formula): returns one satisfying value of
 *    `variable` (suitable for any type, including infinite domains).
 */

#include <cvc5/cvc5.h>

#include <iostream>

using namespace cvc5;

int main()
{
  TermManager tm;
  Solver solver(tm);

  // QuerySolver requires produce-models and incremental mode.
  solver.setOption("produce-models", "true");
  solver.setOption("incremental", "true");

  // -------------------------------------------------------------------------
  // Example 1: Enumerate all 4-bit bit-vector values less than 5
  // -------------------------------------------------------------------------
  {
    Sort bvSort = tm.mkBitVectorSort(4);
    Term x = tm.mkConst(bvSort, "x");

    // Formula: x < 5  (unsigned)
    Term five = tm.mkBitVector(4, 5);
    Term formula = tm.mkTerm(Kind::BITVECTOR_ULT, {x, five});

    QuerySolver qs(solver);
    std::vector<Term> instances = qs.findInstances(x, formula);

    std::cout << "4-bit values of x satisfying (x < 5):" << std::endl;
    for (const Term& v : instances)
    {
      std::cout << "  " << v << std::endl;
    }
    // Expected: #b0000, #b0001, #b0010, #b0011, #b0100  (0 to 4)
    std::cout << "  (total: " << instances.size() << " instances)" << std::endl
              << std::endl;
  }

  // -------------------------------------------------------------------------
  // Example 2: Enumerate at most 3 integer witnesses for x > 100
  // -------------------------------------------------------------------------
  {
    Sort intSort = tm.getIntegerSort();
    Term y = tm.mkConst(intSort, "y");

    // Formula: y > 100
    Term formula =
        tm.mkTerm(Kind::GT, {y, tm.mkInteger(100)});

    QuerySolver qs(solver);
    // maxInstances = 3 bounds the enumeration for this infinite-domain sort
    std::vector<Term> instances = qs.findInstances(y, formula, 3);

    std::cout << "Up to 3 integer values of y satisfying (y > 100):"
              << std::endl;
    for (const Term& v : instances)
    {
      std::cout << "  " << v << std::endl;
    }
    std::cout << std::endl;
  }

  // -------------------------------------------------------------------------
  // Example 3: findWitness for an integer formula (Skolem-style)
  // -------------------------------------------------------------------------
  {
    Sort intSort = tm.getIntegerSort();
    Term z = tm.mkConst(intSort, "z");

    // Formula: z * z = 49  (z^2 = 49)
    Term zSquared = tm.mkTerm(Kind::MULT, {z, z});
    Term formula = tm.mkTerm(Kind::EQUAL, {zSquared, tm.mkInteger(49)});

    QuerySolver qs(solver);
    Term witness = qs.findWitness(z, formula);

    std::cout << "Witness for z satisfying (z * z = 49):" << std::endl;
    if (witness.isNull())
    {
      std::cout << "  No witness found (formula is unsatisfiable)."
                << std::endl;
    }
    else
    {
      std::cout << "  z = " << witness << std::endl;
    }
    std::cout << std::endl;
  }

  // -------------------------------------------------------------------------
  // Example 4: findWitness returns null term for an unsatisfiable formula
  // -------------------------------------------------------------------------
  {
    Sort intSort = tm.getIntegerSort();
    Term w = tm.mkConst(intSort, "w");

    // Formula: w > 0 AND w < 0  (unsatisfiable)
    Term formula = tm.mkTerm(
        Kind::AND,
        {tm.mkTerm(Kind::GT, {w, tm.mkInteger(0)}),
         tm.mkTerm(Kind::LT, {w, tm.mkInteger(0)})});

    QuerySolver qs(solver);
    Term witness = qs.findWitness(w, formula);

    std::cout << "Witness for w satisfying (w > 0 AND w < 0):" << std::endl;
    if (witness.isNull())
    {
      std::cout << "  No witness found (formula is unsatisfiable)."
                << std::endl;
    }
    else
    {
      std::cout << "  w = " << witness << std::endl;
    }
  }

  return 0;
}
