#pragma once

#include <boost/hana/tuple.hpp>
#include <memory>

#include "strong_typedef.hpp"
#include "utils/assert.hpp"

/**
 * We use STRONG_TYPEDEF to avoid things like adding chunk ids and value ids.
 * Because implicit constructors are deleted, you cannot initialize a ChunkID
 * like this
 *   ChunkID x = 3;
 * but need to use
 *   ChunkID x{3}; or
 *   auto x = ChunkID{3};
 *
 * WorkerID, TaskID, CommitID, and TransactionID are used in std::atomics and
 * therefore need to be trivially copyable. That's currently not possible with
 * the strong typedef (as far as I know).
 *
 * TODO(anyone): Also, strongly typing ChunkOffset causes a lot of errors in
 * the group key and adaptive radix tree implementations. Unfortunately, I
 * wasn’t able to properly resolve these issues because I am not familiar with
 * the code there
 */

STRONG_TYPEDEF(uint32_t, ChunkID);
STRONG_TYPEDEF(uint16_t, ColumnID);
STRONG_TYPEDEF(uint32_t, ValueID);  // Cannot be larger than ChunkOffset
STRONG_TYPEDEF(uint32_t, NodeID);
STRONG_TYPEDEF(uint32_t, CpuID);

// Used to identify a Parameter within a (Sub)Select. This can be either a parameter of a Prepared SELECT statement
// `SELECT * FROM t WHERE a > ?` or a correlated parameter in a Subselect.
STRONG_TYPEDEF(size_t, ParameterID);

namespace opossum {

namespace hana = boost::hana;

// This corresponds to the definitions in all_type_variant.hpp. We have the list of data types in two different files
// because while including the DataType enum is cheap, building the variant is not.

static constexpr auto data_types = hana::make_tuple(hana::type_c<int32_t>, hana::type_c<int64_t>, hana::type_c<float>,
                                                    hana::type_c<double>, hana::type_c<std::string>);

// We use a boolean data type in the JitOperatorWrapper.
// However, adding it to DATA_TYPE_INFO would trigger many unnecessary template instantiations for all other operators
// and should thus be avoided for compilation performance reasons.
// We thus only add "Bool" to the DataType enum and define JIT_DATA_TYPE_INFO (with a boolean data type) in
// "lib/operators/jit_operator/jit_types.hpp".
// We need to append to the end of the enum to not break the matching of indices between DataType and AllTypeVariant.

enum class DataType : uint8_t { Null, Int, Long, Float, Double, String, Bool };

using ChunkOffset = uint32_t;

constexpr ChunkOffset INVALID_CHUNK_OFFSET{std::numeric_limits<ChunkOffset>::max()};
constexpr ChunkID INVALID_CHUNK_ID{std::numeric_limits<ChunkID::base_type>::max()};

struct RowID {
  ChunkID chunk_id{INVALID_CHUNK_ID};
  ChunkOffset chunk_offset{INVALID_CHUNK_OFFSET};

  RowID() = default;

  RowID(const ChunkID chunk_id, const ChunkOffset chunk_offset) : chunk_id(chunk_id), chunk_offset(chunk_offset) {
    DebugAssert((chunk_offset == INVALID_CHUNK_OFFSET) == (chunk_id == INVALID_CHUNK_ID),
                "If you pass in one of the arguments as INVALID/NULL, the other has to be INVALID/NULL as well. This "
                "makes sure there is just one value representing an invalid row id.");
  }

  // Faster than row_id == ROW_ID_NULL, since we only compare the ChunkOffset
  bool is_null() const { return chunk_offset == INVALID_CHUNK_OFFSET; }

  // Joins need to use RowIDs as keys for maps.
  bool operator<(const RowID& other) const { return chunk_id < other.chunk_id || chunk_offset < other.chunk_offset; }

  // Useful when comparing a row ID to NULL_ROW_ID
  bool operator==(const RowID& other) const { return chunk_id == other.chunk_id && chunk_offset == other.chunk_offset; }
};

using WorkerID = uint32_t;
using TaskID = uint32_t;

// When changing these to 64-bit types, reading and writing to them might not be atomic anymore.
// Among others, the validate operator might break when another operator is simultaneously writing begin or end CIDs.
using CommitID = uint32_t;
using TransactionID = uint32_t;

using AttributeVectorWidth = uint8_t;

using ColumnIDPair = std::pair<ColumnID, ColumnID>;

constexpr NodeID INVALID_NODE_ID{std::numeric_limits<NodeID::base_type>::max()};
constexpr TaskID INVALID_TASK_ID{std::numeric_limits<TaskID>::max()};
constexpr CpuID INVALID_CPU_ID{std::numeric_limits<CpuID::base_type>::max()};
constexpr WorkerID INVALID_WORKER_ID{std::numeric_limits<WorkerID>::max()};
constexpr ColumnID INVALID_COLUMN_ID{std::numeric_limits<ColumnID::base_type>::max()};

constexpr NodeID CURRENT_NODE_ID{std::numeric_limits<NodeID::base_type>::max() - 1};

// Declaring one part of a RowID as invalid would suffice to represent NULL values. However, this way we add an extra
// safety net which ensures that NULL values are handled correctly. E.g., getting a chunk with INVALID_CHUNK_ID
// immediately crashes.
const RowID NULL_ROW_ID = RowID{INVALID_CHUNK_ID, INVALID_CHUNK_OFFSET};  // TODO(anyone): Couldn’t use constexpr here

constexpr ValueID INVALID_VALUE_ID{std::numeric_limits<ValueID::base_type>::max()};

// The Scheduler currently supports just these 3 priorities, subject to change.
enum class SchedulePriority {
  Default = 1,  // Schedule task at the end of the queue
  High = 0      // Schedule task at the beginning of the queue
};

enum class PredicateCondition {
  Equals,
  NotEquals,
  LessThan,
  LessThanEquals,
  GreaterThan,
  GreaterThanEquals,
  Between,
  In,
  NotIn,
  Like,
  NotLike,
  IsNull,
  IsNotNull
};

bool is_binary_predicate_condition(const PredicateCondition predicate_condition);

// ">" becomes "<" etc.
PredicateCondition flip_predicate_condition(const PredicateCondition predicate_condition);

// ">" becomes "<=" etc.
PredicateCondition inverse_predicate_condition(const PredicateCondition predicate_condition);

enum class JoinMode { Inner, Left, Right, Outer, Cross, Semi, Anti };

enum class UnionMode { Positions };

enum class OrderByMode { Ascending, Descending, AscendingNullsLast, DescendingNullsLast };

enum class TableType { References, Data };

enum class HistogramType { EqualWidth, EqualHeight, EqualDistinctCount };

enum class DescriptionMode { SingleLine, MultiLine };

enum class UseMvcc : bool { Yes = true, No = false };

enum class CleanupTemporaries : bool { Yes = true, No = false };

// Used as a template parameter that is passed whenever we conditionally erase the type of a template. This is done to
// reduce the compile time at the cost of the runtime performance. Examples are iterators, which are replaced by
// AnySegmentIterators that use virtual method calls.
enum class EraseTypes { OnlyInDebug, Always };

class Noncopyable {
 protected:
  Noncopyable() = default;
  Noncopyable(Noncopyable&&) noexcept = default;
  Noncopyable& operator=(Noncopyable&&) noexcept = default;
  ~Noncopyable() = default;
  Noncopyable(const Noncopyable&) = delete;
  const Noncopyable& operator=(const Noncopyable&) = delete;
};

// Dummy type, can be used to overload functions with a variant accepting a Null value
struct Null {};

}  // namespace opossum
