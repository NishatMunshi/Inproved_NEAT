#pragma once
#include "Synapse.hpp"

class Neuron
{
    float m_input;
    float m_output;

    std::unordered_map<Neuron *, float> m_outputSynapses;

private:
    inline static float activation_function(const float _x) { return tanh(_x); }

public:
    const unsigned indexInLayer;
    Neuron(const unsigned _indexInLayer) : indexInLayer(_indexInLayer)
    {
        m_input = 0;
        m_output = 1;
    }
    inline void add_output_synapse(const Synapse &synapse) { m_outputSynapses.insert(synapse); }

public:
    void feed_forward(void)
    {
        m_output = activation_function(m_input);
        m_input = 0;

        for (const auto &[neuron, weight] : m_outputSynapses)
        {
            neuron->m_input += weight * this->m_output;
        }
    }

public: // member access
    inline void set_input(const float _input) { m_input = _input; }
    inline auto get_output(void) const { return m_output; }

public:
    inline void clear_output_synapses(void) { m_outputSynapses.clear(); }
};
