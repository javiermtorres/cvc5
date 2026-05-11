/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * White box testing of cvc5::decision::AssertionList.
 */

#include "context/context.h"
#include "decision/assertion_list.h"
#include "test_node.h"

namespace cvc5::internal {

using namespace cvc5::context;
using namespace decision;

namespace test {

class TestPropWhiteAssertionList : public TestNode
{
 protected:
  UserContext d_assertionContext;
  Context d_internalContext;
};

TEST_F(TestPropWhiteAssertionList, retractDynamicAssertion)
{
  AssertionList al(&d_assertionContext, &d_internalContext, true);
  Node a = d_nodeManager->mkVar("a", *d_boolTypeNode);
  Node b = d_nodeManager->mkVar("b", *d_boolTypeNode);

  al.addAssertion(a);

  d_assertionContext.push();
  al.addAssertion(b);
  al.notifyStatus(b, DecisionStatus::BACKTRACK);
  d_assertionContext.pop();

  ASSERT_EQ(al.getNextAssertion(), a);

  d_assertionContext.push();
  al.addAssertion(b);
  al.notifyStatus(b, DecisionStatus::BACKTRACK);

  ASSERT_EQ(al.getNextAssertion(), b);
}

}  // namespace test
}  // namespace cvc5::internal
