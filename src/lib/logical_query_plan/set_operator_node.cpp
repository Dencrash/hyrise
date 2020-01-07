#include "set_operator_node.hpp"

namespace opossum {

SetOperatorNode::SetOperatorNode(const SetOperatorMode set_operator_mode)
    : AbstractLPQNode(LQPNodeType::SetOperator), set_operator_mode(set_operator_mode) {}

std::string SetOperatorNode::description() {}

const SetOperatorNode::std::vector<std::shared_ptr<AbstractExpression>>& column_expressions() {}

bool SetOperatorNode::is_column_nullable(const ColumnID column_id) {}

size_t SetOperatorNode::_shallow_hash() {}

std::shared_ptr<AbstractLQPNode> SetOperatorNode::_on_shallow_copy(LQPNodeMapping& node_mapping) {}

bool SetOperatorNode::_on_shallow_equals(const AbstractLQPNode& rhs, const LQPNodeMapping& node_mapping) {}

}  // namespace opossum