#include "NN.h"

NN::NN(int INPUT_NODES, int HIDDEN_NODES, int HIDDEN_LAYERS, int OUTPUT_NODES)
    : INPUT_NODES(INPUT_NODES), HIDDEN_NODES(HIDDEN_NODES), HIDDEN_LAYERS(HIDDEN_LAYERS), OUTPUT_NODES(OUTPUT_NODES) {

    // Input to first hidden layer
    weights.push_back(Eigen::MatrixXd::Random(HIDDEN_NODES, INPUT_NODES));
    biases.push_back(Eigen::VectorXd::Random(HIDDEN_NODES));

    // Hidden layer connections
    for (int i = 1; i < HIDDEN_LAYERS; ++i) {
        weights.push_back(Eigen::MatrixXd::Random(HIDDEN_NODES, HIDDEN_NODES));
        biases.push_back(Eigen::VectorXd::Random(HIDDEN_NODES));
    }

    // Last hidden to output
    weights.push_back(Eigen::MatrixXd::Random(OUTPUT_NODES, HIDDEN_NODES));
    biases.push_back(Eigen::VectorXd::Random(OUTPUT_NODES));
}

void NN::SetInputValues(Eigen::VectorXd input) {
    inputActivationValues = input;
}

Eigen::VectorXd NN::ReLU(Eigen::VectorXd vector) {

    Eigen::VectorXd output(vector.size());

    for (int i = 0; i < vector.size(); ++i) {
        output(i) = std::max(vector(i), 0.0);
    }

    return output;
}

Eigen::VectorXd NN::Propogate(Eigen::MatrixXd weights, Eigen::VectorXd activationValue, Eigen::VectorXd biases) {

    return NN::ReLU(weights*activationValue + biases);
}

 Eigen::VectorXd NN::GetOutput() {

    hiddenActivationValues[0] = Propogate(weights[0], inputActivationValues, biases[0]);

    for (int i = 0; i < HIDDEN_LAYERS; ++i){
        hiddenActivationValues[i+1] = Propogate(weights[i+1], hiddenActivationValues[i+1], biases[i+1]);
    }

    outputActivationValues = Propogate(weights[HIDDEN_LAYERS], hiddenActivationValues[HIDDEN_LAYERS], biases[HIDDEN_LAYERS]);

    return outputActivationValues;
}

double Cost(Eigen::VectorXd outputActivation, Eigen::VectorXd expectedValue) {

    if (outputActivation.size() != expectedValue.size()) {
        std::cout << "size missmatch" << std::endl;
    }

    double totalError = 0;

    for (int i = 0; i < outputActivation.size(); ++i) {
        totalError += std::pow(outputActivation(i) - expectedValue(i), 2);
    }

    return totalError;
}