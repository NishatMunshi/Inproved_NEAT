#pragma once
#include "Neuron.hpp"

class Network
{
private:
    const unsigned m_numberOfInputs;
    const unsigned m_numberOfOutputs;

public:
    using Layer = std::unordered_map<NeuronID, Neuron *>;
    std::array<Layer, 3> neuralNetwork;

    // runs once
    Network(const unsigned _numberOfInputs, const unsigned _numberOfOutputs) : m_numberOfInputs(_numberOfInputs), m_numberOfOutputs(_numberOfOutputs)
    {
        for (unsigned index = 0; index < _numberOfInputs; ++index)
        {
            neuralNetwork.front().insert_or_assign(index, new Neuron(index));
        }
        for (unsigned index = 0; index < _numberOfOutputs; ++index)
        {
            neuralNetwork.back().insert_or_assign(_numberOfInputs + index, new Neuron(index));
        }
    }

public:
    auto feed_forward(const std::array<float, NUMBER_OF_INPUTS> &_inputs)
    {
        assert(m_numberOfInputs == _inputs.size());

        for (const auto &[id, neuron] : neuralNetwork.front())
        {
            neuron->set_input(_inputs.at(neuron->indexInLayer));
        }
        for (const auto &layer : neuralNetwork)
        {
            for (const auto &[id, neuron] : layer)
            {
                neuron->feed_forward();
            }
        }

        using NeuronGene = std::pair<NeuronID, Neuron *>;
        auto iterator = std::max_element(neuralNetwork.back().begin(), neuralNetwork.back().end(), [](const NeuronGene &_a, const NeuronGene &_b)
                                         { return _a.second->get_output() < _b.second->get_output(); });
        return iterator->second->indexInLayer;
    }

    auto calculate_error(const std::array<float, NUMBER_OF_OUTPUTS> &_label)
    {
        assert(m_numberOfOutputs == _label.size());

        float error = 0;

        for (const auto &[id, neuron] : neuralNetwork.back())
        {
            auto del = neuron->get_output() - _label.at(neuron->indexInLayer);
            del *= del;
            error += del;
        }
        return error / m_numberOfOutputs;
    }
};
