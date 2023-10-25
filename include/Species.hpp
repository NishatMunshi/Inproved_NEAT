#pragma once
#include "Genome.hpp"

#define MUTATION_PROBABILITY 0.1f

class Species
{
private:
    std::unordered_map<NeuronID, Neuron *> m_neuronPool;

    std::deque<Genome *> m_genePool;

    Network *m_net;

    const unsigned m_numberOfInputs;
    const unsigned m_numberOfOutputs;
    const unsigned m_population;

public:
    Species(const unsigned _numberOfInputs, const unsigned _numberOfOutputs, const unsigned _population) : m_numberOfInputs(_numberOfInputs), m_numberOfOutputs(_numberOfOutputs), m_population(_population)
    {
        assert(_population % 4 == 0);

        m_net = new Network(_numberOfInputs, _numberOfOutputs);

        // keep a record of all neurons in the neuronPool
        for (const auto &layer : m_net->neuralNetwork)
        {
            for (const auto &[id, neuron] : layer)
            {
                m_neuronPool.insert_or_assign(id, neuron);
            }
        }

        for (unsigned individualIndex = 0; individualIndex < _population; ++individualIndex)
        {
            // make individual genomes
            m_genePool.push_back(new Genome(m_net));
        }
    }

private:
    void reconstruct_net_according_to(const std::unordered_map<SynapseID, Synapse> &_genes)
    {
        for (const auto &[id, neuron] : m_neuronPool)
        {
            neuron->clear_output_synapses();
        }
        for (const auto [id, synapse] : _genes)
        {
            const auto &[startingId, endingId] = id;
            m_neuronPool.at(startingId)->add_output_synapse(synapse);
        }
    }

public:
    void play_one_generation(void)
    {
        std::ifstream file("E:/Code/NEAT/xor.txt");

        std::array<float, NUMBER_OF_INPUTS> inputs;
        std::array<float, NUMBER_OF_OUTPUTS> label;

        for (const auto &genome : m_genePool)
        {
            genome->numberOfCorrectAnswers = 0;
            genome->score = 1000;

            reconstruct_net_according_to(genome->genes);

            for (unsigned line = 0; line < NUMBER_OF_LINES; ++line)
            {
                file >> inputs[0];
                file >> inputs[1];
                file >> inputs[2];

                float output;
                file >> output;

                label[0] = -1;
                label[1] = -1;

                if (output == 1)
                    label[1] = 1;
                else
                    label[0] = 1;

                const auto resultIndex = m_net->feed_forward(inputs);
                if ((resultIndex == 0 and label[0] == 1) or (resultIndex == 1 and label[1] == 1))
                {
                    genome->numberOfCorrectAnswers++;
                }
                const auto error = m_net->calculate_error(label);
                genome->score -= error;
            }
        }
    }

    void print_best_scorer(void) const
    {
        auto less = [](const Genome *A, const Genome *B)
        {
            return A->score < B->score;
        };
        const auto &bestGenome = *std::max_element(m_genePool.begin(), m_genePool.end(), less);

        std::cout << "best genome score: " << bestGenome->score << '\n'
                  << "number of neurons used: " << bestGenome->usedNeurons.size() << '\n'
                  << "number of synapses used: " << bestGenome->genes.size() << '\n'
                  << "number of correct answers: " << bestGenome->numberOfCorrectAnswers << '\n';

        std::cout << "\ngenes: \n";
        for (const auto &[id, synapse] : bestGenome->genes)
        {
            std::cout << id.first << ' ' << id.second << ' ' << synapse.second << '\n';
        }
    }

public:
    void mutate(Genome *_genome)
    {
        // decide which mutation to do
        const auto randomInt = random_U32.generate();

        if (randomInt < UINT32_MAX * 0.8f)
        {
            // change a random weight
            _genome->change_random_weight();
        }
        else if (randomInt >= UINT32_MAX * 0.8f and randomInt < UINT32_MAX * 0.9f)
        {
            // add a new synapse
            _genome->add_new_random_synapse(m_neuronPool);
        }
        else if (randomInt >= UINT32_MAX * 0.9f and randomInt < UINT32_MAX)
        {
            // evolve a synapse
            const auto whatNewNeuronsIDWouldBe = _genome->usedNeurons.size();

            // figure out if this neuron already exists in the genepool
            const auto alreadyExists = m_neuronPool.count(whatNewNeuronsIDWouldBe);
            if (alreadyExists)
            {
                // pass the pointer to this neuron for the genome to save
                _genome->evolve_random_synapse(m_neuronPool.at(whatNewNeuronsIDWouldBe), m_neuronPool);
            }
            else
            {
                // create a new neuron with that given id adn save it in both the net and the pool
                // and pass its pointer to the genome to save
                const auto newNeuronPtr = new Neuron(m_net->neuralNetwork.at(1).size());
                m_net->neuralNetwork.at(1).insert(std::make_pair(whatNewNeuronsIDWouldBe, newNeuronPtr));
                m_neuronPool.insert(std::make_pair(whatNewNeuronsIDWouldBe, newNeuronPtr));

                _genome->evolve_random_synapse(newNeuronPtr, m_neuronPool);
            }
        }
    }
    void repolulate(void)
    {
        // select the best performing individuals
        auto greater = [](const Genome *A, const Genome *B)
        {
            return A->score > B->score;
        };
        std::sort(m_genePool.begin(), m_genePool.end(), greater);

        for (unsigned index = m_population / 2; index < m_population; ++index)
        {
            delete m_genePool.at(index);
        }
        m_genePool.resize(m_population / 2);
        auto &selectedGenomes = m_genePool;

        std::set<NeuronID> neuronsUsedByNewGeneration; // need to keep track for extinction phase

        // reproduction phase
        for (long int index = m_population / 2 - 1; index >= 0; index -= 2)
        {
            const auto &father = selectedGenomes.at(index);
            const auto &mother = selectedGenomes.at(index - 1);

            // each couple must produce four offsprings
            for (unsigned childIndex = 0; childIndex < 4; ++childIndex)
            {
                auto child = father->cross(mother);

                // decide whether to mutate
                const auto randomInt = random_U32.generate();
                if (randomInt < MUTATION_PROBABILITY * UINT32_MAX)
                {
                    mutate(child);
                }

                // add child to the front of genepool
                m_genePool.push_front(child);

                // keep track of which neurons this child uses
                auto childUsedNeurons = child->usedNeurons; /// we need to make  a copy to not alter child
                neuronsUsedByNewGeneration.merge(childUsedNeurons);
            }

            // now that father and mother are done reproducing
            // they can be removed from the species (extinction)
            delete selectedGenomes.back();
            selectedGenomes.pop_back();
            delete selectedGenomes.back();
            selectedGenomes.pop_back();
        }

        // neuron extinction phase
        /// REMEMBER TO REMOVE UNUSED NEURONS FROM THE SPECIES AT THIS POINT IN EVOLUTION
        std::set<NeuronID> neuronIDsToRemove;
        for (const auto &[id, neuron] : m_neuronPool)
        {
            if (not neuronsUsedByNewGeneration.count(id))
            {
                // free memory
                delete neuron;

                // remove it's pointer from neuralnetwork
                m_net->neuralNetwork.at(1).erase(id); // neuron to remove must be a hidden one

                // record the id to remove
                neuronIDsToRemove.insert(id);
            }
        }
        // remove them from the species altogether
        for (const auto &id : neuronIDsToRemove)
        {
            m_neuronPool.erase(id);
        }
    }
};