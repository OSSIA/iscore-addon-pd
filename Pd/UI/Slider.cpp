#include "Slider.hpp"
#include <Pd/UI/NodeItem.hpp>

namespace Dataflow
{

SliderUI::SliderUI()
{
    this->setAcceptedMouseButtons(Qt::LeftButton);
}

void SliderUI::paint(QPainter *painter)
{
    auto rect = this->boundingRect();
    painter->setBrush(Qt::darkBlue);
    painter->setPen(Qt::blue);
    painter->drawRect(rect);
    rect.setSize({rect.width() * m_value, rect.height()});
    painter->setBrush(Qt::darkCyan);
    painter->setPen(Qt::cyan);
    painter->drawRect(rect);
}

void SliderUI::mousePressEvent(QMouseEvent *event)
{
    changeValue(event->localPos().x());
    event->accept();
}

void SliderUI::mouseMoveEvent(QMouseEvent *event)
{
    changeValue(event->localPos().x());
    event->accept();
}

void SliderUI::mouseReleaseEvent(QMouseEvent *event)
{
    changeValue(event->localPos().x());
    event->accept();
}

void SliderUI::changeValue(double pos)
{
    const double w = boundingRect().width();

    if(pos < 0)
        m_value = 0.;
    else if(pos > w)
        m_value = 1.;
    else
        m_value = pos / w;

    update();
}



Slider::Slider(const DocumentPlugin& doc, Id<Process::Node> c, QObject *parent)
    : Process::Node{c, parent}
{
    auto item = new NodeItem{doc.context(), *this};
    item->setParentItem(doc.window.view.contentItem());
    item->setPosition(QPointF(100, 100));
    ui = item;

    auto sld = new SliderUI{};
    sld->setParentItem(item);
    sld->setX(6);
    sld->setY(6);
    sld->setWidth(item->width() - 12);
    sld->setHeight(item->height() - 12);
    connect(sld, &SliderUI::valueChanged,
            this, [=] (double v) { setVolume(v); });
}

void Slider::setVolume(double v)
{
    m_volume = v;
    emit volumeChanged(v);
}

QString Slider::getText() const
{
    return "Slider";
}

std::size_t Slider::audioInlets() const
{
    return 1;
}

std::size_t Slider::messageInlets() const
{
    return 0;
}

std::size_t Slider::midiInlets() const
{
    return 0;
}

std::size_t Slider::audioOutlets() const
{
    return 1;
}

std::size_t Slider::messageOutlets() const
{
    return 0;
}

std::size_t Slider::midiOutlets() const
{
    return 0;
}

std::vector<Process::Port> Slider::inlets() const
{
    std::vector<Process::Port> p(1);
    p[0].type = Process::PortType::Audio;
    return p;
}

std::vector<Process::Port> Slider::outlets() const
{
  std::vector<Process::Port> p(1);
  p[0].type = Process::PortType::Audio;
  p[0].propagate = true;
  return p;
}

std::vector<Id<Process::Cable> > Slider::cables() const
{ return m_cables; }

void Slider::addCable(Id<Process::Cable> c)
{ m_cables.push_back(c); }

void Slider::removeCable(Id<Process::Cable> c)
{ m_cables.erase(ossia::find(m_cables, c)); }

}
