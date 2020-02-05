#pragma once

#include <memory>
#include <string>
#include <vector>

#include "abstract_lqp_node.hpp"
#include "lqp_column_reference.hpp"
#include "types.hpp"

namespace opossum {

class IntersectNode : public EnableMakeForLQPNode<IntersectNode>, public AbstractLQPNode {
 public:
  explicit IntersectNode(const UnionMode union_mode,
                         const std::vector<std::shared_ptr<AbstractExpression>>& join_predicates);

  std::string description(const DescriptionMode mode = DescriptionMode::Short) const override;
  const std::vector<std::shared_ptr<AbstractExpression>>& column_expressions() const override;
  bool is_column_nullable(const ColumnID column_id) const override;

  const UnionMode union_mode;
  const std::vector<std::shared_ptr<AbstractExpression>>& join_predicates() const;

 protected:
  size_t _on_shallow_hash() const override;
  std::shared_ptr<AbstractLQPNode> _on_shallow_copy(LQPNodeMapping& node_mapping) const override;
  bool _on_shallow_equals(const AbstractLQPNode& rhs, const LQPNodeMapping& node_mapping) const override;
};
}  // namespace opossum