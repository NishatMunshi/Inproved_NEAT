#pragma once
#include "Network.hpp"

class Genome
{
public:
    std::set<NeuronID> usedNeurons; // cannot permit repetitaions
    // std::set<SynapseID> usedSynapses; // cannot permit repeattions
    std::unordered_map<SynapseID, Synapse> genes;

    unsigned numberOfCorrectAnswers;
    float score;

    // this constructor is called only when starting the species
    Genome(Network *const _network)
    {
        // fully connect input to output layer
        for (const auto &[inputId, sensor] : _network->neuralNetwork.front())
        {
            usedNeurons.insert(inputId);
            for (const auto &[outputId, motor] : _network->neuralNetwork.back())
            {
                usedNeurons.insert(outputId); /// since it is std::set there is no duplicate inserted

                const auto synapseId = SynapseID(inputId, outputId);
                const auto newSynapse = Synapse(motor, random_float());

                // usedSynapses.insert(synapseId);
                genes.insert_or_assign(synapseId, newSynapse);
            }
        }
    }

public:
    Genome *cross(const Genome *_mother) const
    {
        const auto &father = this;
        auto child = new Genome(*father);

        // child must use all neurons used by father and mother
        auto motherNeurons = _mother->usedNeurons; // need this copy to not alter mother's genes
        child->usedNeurons.merge(motherNeurons);

        // child must use all synapseIndeces used by father and mother
        // auto mothersSynapses = _mother->usedSynapses; // need this copy to not alter mother's genes
        // child->usedSynapses.merge(mothersSynapses);

        for (const auto &[id, synapse] : _mother->genes)
        {
            const auto matched = father->genes.count(id);
            if (matched)
            {
                // decide whether to use mother's gene
                auto decision = random_bool.generate();
                if (decision)
                {
                    // use mother's gene
                    child->genes.at(id) = synapse;
                }
            }
            // if it did not match, it must be a new one and we have to insert it
            else
            {
                child->genes.insert_or_assign(id, synapse);
            }
        }
        return child;
    }

public: // mutations
    void change_random_weight(void)
    {
        const auto &randomShift = random_U32.generate(0, genes.size() - 1);
        const auto &randomSynapseIterator = std::next(genes.begin(), randomShift);

        randomSynapseIterator->second.second = random_float();
    }
    void add_new_random_synapse(const std::unordered_map<NeuronID, Neuron *> &_neuronPool)
    {
        const auto &randomShift1 = random_U32.generate(0, usedNeurons.size() - 1);
        const auto &neuronIDIterator1 = std::next(usedNeurons.begin(), randomShift1);

        const auto &randomShift2 = random_U32.generate(0, usedNeurons.size() - 1);
        const auto &neuronIDIterator2 = std::next(usedNeurons.begin(), randomShift2);

        const auto newSynapseId = std::make_pair(*neuronIDIterator1, *neuronIDIterator2);
        const auto newSynapse = Synapse(_neuronPool.at(*neuronIDIterator2), random_float());

        // attempt to insert it, if already there then change its weight
        genes.insert(std::make_pair(newSynapseId, newSynapse));
    }

    void evolve_random_synapse(Neuron *_newPtr, const std::unordered_map<NeuronID, Neuron *> &_neuronPool)
    {
        const NeuronID whatNewNeuronsIDWouldBe = usedNeurons.size();

        const auto &randomShift = random_U32.generate(0, genes.size() - 1);
        const auto &randomSynapseIterator = std::next(genes.begin(), randomShift);

        // DISABLE IT (make weight 0)
        randomSynapseIterator->second.second = 0.f;

        // exctract the information about where it comes from and whre it goes
        const auto startingNeuronID = randomSynapseIterator->first.first;
        const auto endingNeuronID = randomSynapseIterator->first.second;

        // const auto startingNeuronPtr = _neuronPool.at(startingNeuronID);
        const auto endingNeuronPtr = _neuronPool.at(endingNeuronID);

        // make two new syanpseids and corresponding synapses with this info
        const auto newSynapseId1 = std::make_pair(startingNeuronID, whatNewNeuronsIDWouldBe);
        const auto newSynapse1 = Synapse(_newPtr, random_float());
        const auto newGene1 = std::make_pair(newSynapseId1, newSynapse1);

        const auto newSynapseId2 = std::make_pair(whatNewNeuronsIDWouldBe, endingNeuronID);
        const auto newSynapse2 = Synapse(endingNeuronPtr, random_float());
        const auto newGene2 = std::make_pair(newSynapseId2, newSynapse2);

        // add the new genes and the newly used neurons ids in their proper containers
        usedNeurons.insert(whatNewNeuronsIDWouldBe);

        genes.insert(newGene1);
        genes.insert(newGene2);
    }
};