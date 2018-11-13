#pragma once

#include <memory>
#include <string>

namespace opossum {

class AbstractLQPNode;
class AbstractCostEstimator;
class OptimizationContext;

class AbstractRule {
 public:
  virtual std::string name() const = 0;

  virtual ~AbstractRule() = default;

  /**
   * This function applies the concrete Optimizer Rule to an LQP.
   * apply_to() is intended to be called recursively by the concrete rule.
   * The optimizer will pass the immutable LogicalPlanRootNode to this function.
   * @return whether the rule changed the LQP, used to stop the optimizers iteration
   */
  virtual bool apply_to(const std::shared_ptr<AbstractLQPNode>& root, const AbstractCostEstimator& cost_estimator,
                        const std::shared_ptr<OptimizationContext>& context = {}) const = 0;

 protected:
  /**
   * Apply this rule to @param node's inputs and all subqueries in its expressions
   *
   * IMPORTANT: Takes a copy of the node ptr because applying this rule to inputs of this node might remove this node
   * from the tree, which might result in this node being deleted if we don't take a copy of the shared_ptr here.
   */
  bool _apply_to_inputs(std::shared_ptr<AbstractLQPNode> node, const AbstractCostEstimator& cost_estimator,
                        const std::shared_ptr<OptimizationContext>& context) const;  // NOLINT
};

}  // namespace opossum
