#pragma once
#include <iscore/model/path/Path.hpp>
#include <State/Address.hpp>
#include <ossia/detail/optional.hpp>
#include <iscore/model/IdentifiedObject.hpp>
#include <nodes/../../src/Connection.hpp>
#include <ossia/dataflow/graph_edge.hpp>
namespace Dataflow
{
class ProcessModel;
enum class CableType { ImmediateGlutton, ImmediateStrict, DelayedGlutton, DelayedStrict };

struct CableData
{
  CableType type;
  Path<Dataflow::ProcessModel> source, sink;
  ossia::optional<int> outlet, inlet;

  friend bool operator==(const CableData& lhs, const CableData& rhs)
  {
    return lhs.type == rhs.type
        && lhs.source == rhs.source
        && lhs.sink == rhs.sink
        && lhs.outlet == rhs.outlet
        && lhs.inlet == rhs.inlet;
  }
};

struct Cable
    : public IdentifiedObject<Cable>
    , public CableData
{
  Cable() = delete;
  Cable(const Cable&) = delete;
  Cable(Id<Cable> c): IdentifiedObject{c, "Cable", nullptr} { }

  QtNodes::Connection* gui{};
  std::shared_ptr<ossia::graph_edge> exec;
};

enum class PortType { Message, Audio, Midi };
struct Port
{
  PortType type;
  QString customData;
  State::AddressAccessor address;

  friend bool operator==(const Port& lhs, const Port& rhs)
  {
    return lhs.type == rhs.type && lhs.customData == rhs.customData && lhs.address == rhs.address;
  }
};

}
