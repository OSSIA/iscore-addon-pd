#pragma once
#include <Pd/DataflowProcess.hpp>
#include <Pd/DocumentPlugin.hpp>
#include <Pd/DataflowObjects.hpp>
#include <Pd/Commands/PdCommandFactory.hpp>
#include <iscore/model/path/Path.hpp>

namespace Dataflow
{
class MoveNode final : public iscore::Command
{

  ISCORE_COMMAND_DECL(Pd::CommandFactoryName(), MoveNode, "Move node")

  public:
    MoveNode(const Dataflow::ProcessModel& model, QPointF newpos);

  void undo() const override;
  void redo() const override;

  void update(const ProcessModel& m, QPointF pos) { m_new = pos; }
protected:
  void serializeImpl(DataStreamInput& s) const override;
  void deserializeImpl(DataStreamOutput& s) override;

private:
  Path<Dataflow::ProcessModel> m_model;
  QPointF m_old, m_new;
};

class CreateCable final : public iscore::Command
{
  ISCORE_COMMAND_DECL(Pd::CommandFactoryName(), CreateCable, "Create cable")

  public:
    CreateCable(
      const Dataflow::DocumentPlugin& dp,
      Cable theCable);

  void undo() const override;
  void redo() const override;

protected:
  void serializeImpl(DataStreamInput& s) const override;
  void deserializeImpl(DataStreamOutput& s) override;

private:
  Path<Dataflow::DocumentPlugin> m_model;
  Cable m_cable;
};

class UpdateCable final : public iscore::Command
{
  ISCORE_COMMAND_DECL(Pd::CommandFactoryName(), UpdateCable, "Update cable")

  public:
    UpdateCable(
      const Dataflow::DocumentPlugin& dp,
      Cable theOld, Cable theNew);

  void undo() const override;
  void redo() const override;

protected:
  void serializeImpl(DataStreamInput& s) const override;
  void deserializeImpl(DataStreamOutput& s) override;

private:
  Path<Dataflow::DocumentPlugin> m_model;
  Cable m_old, m_new;
};

class RemoveCable final : public iscore::Command
{
  ISCORE_COMMAND_DECL(Pd::CommandFactoryName(), RemoveCable, "Remove cable")

  public:
    RemoveCable(
      const Dataflow::DocumentPlugin& dp,
      Cable theCable);

  void undo() const override;
  void redo() const override;

protected:
  void serializeImpl(DataStreamInput& s) const override;
  void deserializeImpl(DataStreamOutput& s) override;

private:
  Path<Dataflow::DocumentPlugin> m_model;
  Cable m_cable;
};
}