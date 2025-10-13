#include "NN.h"

NN::NN(int INPUT_NODES, int HIDDEN_NODES, int OUTPUT_NODES, int HIDDEN_LAYERS)
    : INPUT_NODES(INPUT_NODES), HIDDEN_NODES(HIDDEN_NODES), OUTPUT_NODES(OUTPUT_NODES), HIDDEN_LAYERS(HIDDEN_LAYERS) {

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

void NN::GetOutput() {

    for (int i = 0; i < weights.size(); ++i){
        
    }
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