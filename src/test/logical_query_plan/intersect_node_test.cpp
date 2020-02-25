#include <memory>

#include "base_test.hpp"

#include "logical_query_plan/lqp_utils.hpp"
#include "logical_query_plan/mock_node.hpp"

namespace opossum {

class IntersectNodeTest : public BaseTest {
 protected:
  void SetUp() override {
    _mock_node1 = MockNode::make(
        MockNode::ColumnDefinitions{{DataType::Int, "a"}, {DataType::Int, "b"}, {DataType::Int, "c"}}, "t_a");
    _mock_node2 = MockNode::make(MockNode::ColumnDefinitions{{DataType::Int, "u"}, {DataType::Int, "v"}}, "t_b");
    _mock_node3 = MockNode::make(MockNode::ColumnDefinitions{{DataType::Int, "x"}}, "t_v");

    _a = {_mock_node1, ColumnID{0}};
    _b = {_mock_node1, ColumnID{1}};
    _c = {_mock_node1, ColumnID{2}};

    _intersect_node = UnionNode::make(SetOperationMode::Positions);
    _intersect_node->set_left_input(_mock_node1);
    _intersect_node->set_right_input(_mock_node1);
  }

  std::shared_ptr<MockNode> _mock_node1, _mock_node2, _mock_node3;
  std::shared_ptr<IntersectNode> _intersect_node;
  LQPColumnReference _a;
  LQPColumnReference _b;
  LQPColumnReference _c;
};
TEST_F(IntersectNodeTest, Description) {
  EXPECT_EQ(_intersect_node->description(), "[IntersectNode] Mode: SetOperationPositions");
}

TEST_F(IntersectNodeTest, OutputColumnExpressions) {
  EXPECT_EQ(*_intersect_node->column_expressions().at(0), *_mock_node1->column_expressions().at(0));
  EXPECT_EQ(*_intersect_node->column_expressions().at(1), *_mock_node1->column_expressions().at(1));
  EXPECT_EQ(*_intersect_node->column_expressions().at(2), *_mock_node1->column_expressions().at(2));
}

TEST_F(IntersectNodeTest, HashingAndEqualityCheck) {
  auto same_intersect_node = IntersectNode::make(SetOperationMode::Positions);
  same_intersect_node->set_left_input(_mock_node1);
  same_intersect_node->set_right_input(_mock_node1);
  auto different_intersect_node = IntersectNode::make(SetOperationMode::All);
  different_intersect_node->set_left_input(_mock_node1);
  different_intersect_node->set_right_input(_mock_node1);
  auto different_intersect_node_1 = IntersectNode::make(SetOperationMode::Positions);
  different_intersect_node_1->set_left_input(_mock_node1);
  different_intersect_node_1->set_right_input(_mock_node2);
  auto different_intersect_node_2 = IntersectNode::make(SetOperationMode::Positions);
  different_intersect_node_2->set_left_input(_mock_node2);
  different_intersect_node_2->set_right_input(_mock_node1);
  auto different_intersect_node_3 = IntersectNode::make(SetOperationMode::Positions);
  different_intersect_node_3->set_left_input(_mock_node2);
  different_intersect_node_3->set_right_input(_mock_node2);

  EXPECT_EQ(*_intersect_node, *same_intersect_node);
  EXPECT_NE(*_intersect_node, *different_intersect_node);
  EXPECT_NE(*_intersect_node, *different_intersect_node_1);
  EXPECT_NE(*_intersect_node, *different_intersect_node_2);
  EXPECT_NE(*_intersect_node, *different_intersect_node_3);
  EXPECT_NE(*_intersect_node, *IntersectNode::make(SetOperationMode::Positions));
  EXPECT_NE(*_intersect_node, *IntersectNode::make(SetOperationMode::All));

  EXPECT_EQ(_intersect_node->hash(), same_intersect_node->hash());
  EXPECT_NE(_intersect_node->hash(), different_intersect_node->hash());
  EXPECT_NE(_intersect_node->hash(), different_intersect_node_1->hash());
  EXPECT_NE(_intersect_node->hash(), different_intersect_node_2->hash());
  EXPECT_NE(_intersect_node->hash(), different_intersect_node_3->hash());
  EXPECT_NE(_intersect_node->hash(), ExceptNode::make(SetOperationMode::Positions)->hash());
  EXPECT_NE(_intersect_node->hash(), ExceptNode::make(SetOperationMode::All)->hash());
}

TEST_F(IntersectNodeTest, Copy) { EXPECT_EQ(*_intersect_node->deep_copy(), *_intersect_node); }

TEST_F(IntersectNodeTest, NodeExpressions) { ASSERT_EQ(_intersect_node->node_expressions.size(), 0u); }

}  // namespace opossum
