#include "PdProcess.hpp"
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/serialization/JSONValueVisitor.hpp>
#include <iscore/serialization/JSONVisitor.hpp>


namespace Pd
{
ProcessModel::ProcessModel(
        const TimeValue& duration,
        const Id<Process::ProcessModel>& id,
        QObject* parent):
    Process::ProcessModel{duration, id, Metadata<ObjectKey_k, ProcessModel>::get(), parent}
{
    m_script = "(function(t) { \n"
               "     var obj = new Object; \n"
               "     obj[\"address\"] = 'OSCdevice:/millumin/layer/x/instance'; \n"
               "     obj[\"value\"] = t + iscore.value('OSCdevice:/millumin/layer/y/instance'); \n"
               "     return [ obj ]; \n"
               "});";

    metadata().setName(QString("PureData.%1").arg(this->id().val()));
}

ProcessModel::ProcessModel(
        const ProcessModel& source,
        const Id<Process::ProcessModel>& id,
        QObject* parent):
    Process::ProcessModel{source.duration(), id, Metadata<ObjectKey_k, ProcessModel>::get(), parent},
    m_script{source.m_script}
{
}

ProcessModel::~ProcessModel()
{
}

void ProcessModel::setScript(const QString& script)
{
    m_script = script;
    emit scriptChanged(script);
}

}

template<>
void Visitor<Reader<DataStream>>::readFrom_impl(const Pd::ProcessModel& proc)
{
    m_stream << proc.m_script;

    insertDelimiter();
}

template<>
void Visitor<Writer<DataStream>>::writeTo(Pd::ProcessModel& proc)
{
    QString str;
    m_stream >> str;
    proc.setScript(str);

    checkDelimiter();
}

template<>
void Visitor<Reader<JSONObject>>::readFrom_impl(const Pd::ProcessModel& proc)
{
    m_obj["Script"] = proc.script();
}

template<>
void Visitor<Writer<JSONObject>>::writeTo(Pd::ProcessModel& proc)
{
    proc.setScript(m_obj["Script"].toString());
}
