#include <QtTest/QTest>
#include <ossia/dataflow/dataflow.hpp>
#include <z_libpd.h>
#include <ossia/network/generic/generic_device.hpp>
#include <ossia/network/local/local.hpp>
#include <sndfile.hh>
class pd_graph_node final :
    public ossia::graph_node
{
public:
  void add_dzero(std::string& s)
  {
    s = std::to_string(m_dollarzero) + "-" + s;
  }

  static pd_graph_node* m_currentInstance;
  pd_graph_node(const char* file,
                int inputs,
                int outputs,
                std::vector<std::string> inmess,
                std::vector<std::string> outmess)
    : m_inputs{inputs}
    , m_outputs{outputs}
    , m_inmess{inmess}
    , m_outmess{outmess}
  {

    m_currentInstance = nullptr;
    for(int i = 0; i < inputs ; i++)
      in_ports.push_back(ossia::make_inlet<ossia::audio_port>());
    for(int i = 0; i < outputs ; i++)
      out_ports.push_back(ossia::make_outlet<ossia::audio_port>());

    // TODO add $0 to everyone
    {
      m_instance = pdinstance_new();
      pd_setinstance(m_instance);

      // compute audio    [; pd dsp 1(
      libpd_start_message(1); // one entry in list
      libpd_add_float(1.0f);
      libpd_finish_message("pd", "dsp");

      auto handle = libpd_openfile(file, "/home/jcelerier/travail/formalisation-graphes/scores");
      m_dollarzero = libpd_getdollarzero(handle);

      for(auto& mess : m_inmess)
        add_dzero(mess);
      for(auto& mess : m_outmess)
        add_dzero(mess);

      for(auto& str : m_inmess)
      {
        in_ports.push_back(ossia::make_inlet<ossia::value_port>());
      }

      for(auto& mess : m_outmess)
      {
        libpd_bind(mess.c_str());

        out_ports.push_back(ossia::make_outlet<ossia::value_port>());
      }

      libpd_set_floathook([] (const char *recv, float f) {
        //qDebug() << "received float: " << recv <<  f;

        if(auto v = m_currentInstance->get_vp(recv))
        {
          v->data.push_back(f);
        }
      });
      libpd_set_banghook([] (const char *recv) {
        //qDebug() << "received bang: " << recv;

        if(auto v = m_currentInstance->get_vp(recv))
        {
          v->data.push_back(ossia::impulse{});
        }
      });
      libpd_set_symbolhook([] (const char *recv, const char *sym) {
        //qDebug() << "received sym: " << recv << sym;
        if(auto v = m_currentInstance->get_vp(recv))
        {
          v->data.push_back(std::string(sym));
        }
      });
      libpd_set_listhook([] (const char *recv, int argc, t_atom *argv) {
        //qDebug() << "received list: " << recv << argc;
      });
      libpd_set_messagehook([] (const char *recv, const char *msg, int argc, t_atom *argv) {
        //qDebug() << "received message: " << recv << msg << argc;
      });

      libpd_set_noteonhook([] (int channel, int pitch, int velocity) {
        //qDebug() << "received midi Note: " << channel << pitch << velocity;
      });
      libpd_set_controlchangehook([] (int channel, int controller, int value) {
       // qDebug() << "received midi CC: " << channel << controller << value;

      });
    }
  }
      ossia::outlet* get_outlet(const char* str) const
      {
        for(int i = 0; i < m_outmess.size(); ++i)
        {
          if(m_outmess[i] == str)
          {
            return out_ports[i + m_outputs].get();
          }
        }

        return nullptr;
      }


  ossia::value_port* get_vp(const char* str) const
  {
    for(int i = 0; i < m_outmess.size(); ++i)
    {
      if(m_outmess[i] == str)
      {
        return out_ports[i + m_outputs]->data.target<ossia::value_port>();
      }
    }

    return nullptr;
  }

  ~pd_graph_node()
  {
    pdinstance_free(m_instance);
  }

  struct ossia_to_pd_value
  {
    const char* mess{};
    void operator()() const { }
    template<typename T>
    void operator()(const T&) const { }

    void operator()(float f) const { libpd_float(mess, f); }
    void operator()(int f) const { libpd_float(mess, f); }
    void operator()(const std::string& f) const { libpd_symbol(mess, f.c_str()); }
    void operator()(const ossia::impulse& f) const { libpd_bang(mess); }
  };

  void run(ossia::execution_state& e) override
  {
    pd_setinstance(m_instance);
    libpd_init_audio(m_inputs, m_outputs, 44100);


    const auto bs = libpd_blocksize();
    std::vector<float> inbuf;
    inbuf.resize(m_inputs * bs);

    std::vector<float> outbuf;
    outbuf.resize(m_outputs * bs);

    m_currentInstance = this;
    for(int i = 0; i < m_inputs; i++)
    {
      auto ap = in_ports[i]->data.target<ossia::audio_port>();
      const auto pos = i * bs;
      if(!ap->samples.empty())
      {
        auto& arr = ap->samples.back();
        std::copy_n(arr.begin(), bs, inbuf.begin() + pos);
      }
    }

    for(int i = m_inputs; i < in_ports.size(); ++i)
    {
      auto& dat = in_ports[i]->data.target<ossia::value_port>()->data;

      while(dat.size() > 0)
      {
        ossia::value val = dat.front();
        dat.pop_front();
        val.apply(ossia_to_pd_value{m_inmess[i - m_inputs].c_str()});
      }
    }

    libpd_process_raw(inbuf.data(), outbuf.data());
    m_currentInstance = nullptr;

    for(int i = 0; i < m_outputs; ++i)
    {
      auto ap = out_ports[i]->data.target<ossia::audio_port>();
      ap->samples.resize(ap->samples.size() + 1);

      std::copy_n(outbuf.begin() + i * bs, bs, ap->samples.back().begin());
    }
  }

  t_pdinstance * m_instance{};
  int m_dollarzero = 0;

  int m_inputs{}, m_outputs{};
  std::vector<std::string> m_inmess, m_outmess;
};

pd_graph_node* pd_graph_node::m_currentInstance;

class PdDataflowTest : public QObject
{
  Q_OBJECT

public:
  PdDataflowTest()
  {
    libpd_init();
    libpd_set_printhook([] (const char *s) { qDebug() << "string: " << s; });
  }

private slots:
  void test_pd()
  {
    using namespace ossia; using namespace ossia::net;
    generic_device dev{std::make_unique<local_protocol>(), ""};
    auto& filt_node = create_node(dev.getRootNode(), "/filter");
    auto filt_addr = filt_node.createAddress(ossia::val_type::FLOAT);
    filt_addr->pushValue(100);
    auto& vol_node = create_node(dev.getRootNode(), "/param");
    auto vol_addr = vol_node.createAddress(ossia::val_type::FLOAT);
    vol_addr->pushValue(0.5);
    auto& note_node = create_node(dev.getRootNode(), "/note");
    auto note_addr = note_node.createAddress(ossia::val_type::FLOAT);
    note_addr->pushValue(220.);
    auto& l_node = create_node(dev.getRootNode(), "/l");
    auto l_addr = l_node.createAddress(ossia::val_type::IMPULSE);
    auto& r_node = create_node(dev.getRootNode(), "/r");
    auto r_addr = r_node.createAddress(ossia::val_type::IMPULSE);

    ossia::graph g;
    using string_vec = std::vector<std::string>;
    auto f1 = std::make_shared<pd_graph_node>("gen1.pd", 0, 0, string_vec{}, string_vec{"out-1"});
    auto f1_2 = std::make_shared<pd_graph_node>("gen1.pd", 0, 0, string_vec{}, string_vec{"out-1"});
    auto f2 = std::make_shared<pd_graph_node>("gen2.pd", 0, 0, string_vec{"in-1"}, string_vec{"out-1"});
    auto f3 = std::make_shared<pd_graph_node>("gen3.pd", 0, 2, string_vec{"in-1", "in-2"}, string_vec{});
    auto f4 = std::make_shared<pd_graph_node>("gen4.pd", 2, 2, string_vec{"in-2"}, string_vec{});
    auto f5 = std::make_shared<pd_graph_node>("gen5.pd", 2, 2, string_vec{"in-2"}, string_vec{});
    auto f5_2 = std::make_shared<pd_graph_node>("gen5.pd", 2, 2, string_vec{"in-2"}, string_vec{});

    g.add_node(f1);
    g.add_node(f1_2);
    g.add_node(f2);
    g.add_node(f3);
    g.add_node(f4);
    g.add_node(f5);
    g.add_node(f5_2);

    f1->out_ports[0]->address = note_addr;
    f1_2->out_ports[0]->address = note_addr;
    f2->out_ports[0]->address = note_addr;

    f3->in_ports[0]->address = note_addr;
    f3->in_ports[1]->address = vol_addr;
    f3->out_ports[0]->address = l_addr;
    f3->out_ports[1]->address = r_addr;

    f4->in_ports[0]->address = l_addr;
    f4->in_ports[1]->address = r_addr;
    f4->in_ports[2]->address = filt_addr;
    f4->out_ports[0]->address = l_addr;
    f4->out_ports[1]->address = r_addr;

    f5->in_ports[0]->address = l_addr;
    f5->in_ports[1]->address = r_addr;
    f5->in_ports[2]->address = filt_addr;
    f5->out_ports[0]->address = l_addr;
    f5->out_ports[1]->address = r_addr;

    f5_2->in_ports[0]->address = l_addr;
    f5_2->in_ports[1]->address = r_addr;
    f5->in_ports[2]->address = filt_addr;
    f5_2->out_ports[0]->address = l_addr;
    f5_2->out_ports[1]->address = r_addr;


    g.connect(make_edge(immediate_strict_connection{}, f3->out_ports[0], f4->in_ports[0], f3, f4));
    g.connect(make_edge(immediate_strict_connection{}, f3->out_ports[1], f4->in_ports[1], f3, f4));

    g.connect(make_edge(immediate_glutton_connection{}, f4->out_ports[0], f5->in_ports[0], f4, f5));
    g.connect(make_edge(immediate_glutton_connection{}, f4->out_ports[1], f5->in_ports[1], f4, f5));

    //g.connect(make_edge(delayed_strict_connection{}, f4->out_ports[0], f5_2->in_ports[0], f4, f5_2));
    //g.connect(make_edge(delayed_strict_connection{}, f4->out_ports[1], f5_2->in_ports[1], f4, f5_2));

    f1->set_enabled(true);
    f3->set_enabled(true);
    f4->set_enabled(true);
    f5->set_enabled(true);

    std::vector<float> samples;

    execution_state st;
    auto copy_samples = [&] {
      auto it_l = st.localState.find(l_addr);
      auto it_r = st.localState.find(r_addr);
      QVERIFY(it_l != st.localState.end());
      QVERIFY(it_r != st.localState.end());

      auto audio_l = it_l->second.target<audio_port>();
      auto audio_r = it_r->second.target<audio_port>();
      QVERIFY(audio_l);
      QVERIFY(!audio_l->samples.empty());
      QVERIFY(audio_r);
      QVERIFY(!audio_r->samples.empty());

      auto pos = samples.size();
      samples.resize(samples.size() + 128);
      for(auto& arr : audio_l->samples)
      {
        for(int i = 0; i < 64; i++)
          samples[pos + i * 2] += arr[i];
      }
      for(auto& arr : audio_r->samples)
      {
        for(int i = 0; i < 64; i++)
          samples[pos + i * 2 + 1] += arr[i];
      }
    };

    for(int i = 0; i < 10000; i++)
    {
      st = g.exec_state();
      copy_samples();
    }

    qDebug() << samples.size();
     QVERIFY(samples.size() > 0);
    SndfileHandle file("/tmp/out.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, 2, 44100);
    file.write(samples.data(), samples.size());
    file.writeSync();

  }
};

QTEST_APPLESS_MAIN(PdDataflowTest)

#include "PdDataflowTest.moc"
