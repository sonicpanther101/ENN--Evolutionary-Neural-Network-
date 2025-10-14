#include "NN.h"
#include "Eigen/src/Core/Matrix.h"

NN::NN(int INPUT_NODES, int HIDDEN_NODES, int HIDDEN_LAYERS, int OUTPUT_NODES, double LEARNING_RATE)
    : INPUT_NODES(INPUT_NODES), HIDDEN_NODES(HIDDEN_NODES), HIDDEN_LAYERS(HIDDEN_LAYERS), OUTPUT_NODES(OUTPUT_NODES), LEARNING_RATE(LEARNING_RATE) {

    // Input to first hidden layer
    weights.push_back(Eigen::MatrixXd::Random(HIDDEN_NODES, INPUT_NODES));
    biases.push_back(Eigen::VectorXd::Random(HIDDEN_NODES));

    Eigen::VectorXd placeholder(1);
    hiddenActivationValues.push_back(placeholder);

    // Hidden layer connections
    for (int i = 1; i < HIDDEN_LAYERS; ++i) {
        weights.push_back(Eigen::MatrixXd::Random(HIDDEN_NODES, HIDDEN_NODES));
        biases.push_back(Eigen::VectorXd::Random(HIDDEN_NODES));
        hiddenActivationValues.push_back(placeholder);
    }

    // Last hidden to output
    weights.push_back(Eigen::MatrixXd::Random(OUTPUT_NODES, HIDDEN_NODES));
    biases.push_back(Eigen::VectorXd::Random(OUTPUT_NODES));

    std::cout << "Weights matrices: " << weights.size() << std::endl;
    std::cout << "Biases vectors: " << biases.size() << std::endl;
    std::cout << "hidden activation value vector: " << hiddenActivationValues.size() << std::endl;

    std::cout << "Initialised" << std::endl;
}

void NN::SetInputValues(Eigen::VectorXd input) {
    inputActivationValues = input;

    std::cout << "Inputs set" << std::endl;
}

Eigen::VectorXd NN::ReLU(Eigen::VectorXd vector) {

    std::cout << "Applying ReLU" << std::endl;

    Eigen::VectorXd output(vector.size());

    for (int i = 0; i < vector.size(); ++i) {
        output(i) = std::max(vector(i), 0.0);
    }

    std::cout << "Done ReLU" << std::endl;

    return output;
}

Eigen::VectorXd NN::Propagate(Eigen::MatrixXd weights, Eigen::VectorXd activationValue, Eigen::VectorXd biases) {
    std::cout << "Propagating" << std::endl;

    Eigen::VectorXd newActivationValues = weights*activationValue + biases;

    std::cout << "new values found" << std::endl;

    return NN::ReLU(newActivationValues);
}

 Eigen::VectorXd NN::GetOutput() {

    hiddenActivationValues[0] = Propagate(weights[0], inputActivationValues, biases[0]);

    for (int i = 1; i < HIDDEN_LAYERS; ++i){
        hiddenActivationValues[i] = Propagate(weights[i], hiddenActivationValues[i-1], biases[i]);
    }

    outputActivationValues = Propagate(weights[HIDDEN_LAYERS], hiddenActivationValues[HIDDEN_LAYERS - 1], biases[HIDDEN_LAYERS]);

    std::cout << "Output layer propagated" << std::endl;

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

void NN::AdjustWeights(Eigen::VectorXd expectedOutput) {

    Eigen::MatrixXd deltaWeights = Eigen::MatrixXd::Zero(HIDDEN_NODES, HIDDEN_NODES);
    std::vector<Eigen::VectorXd> errorCum;

    Eigen::VectorXd error = outputActivationValues.cwiseProduct(
        (Eigen::VectorXd::Ones(outputActivationValues.size()) - outputActivationValues)).cwiseProduct(
            expectedOutput - outputActivationValues);

    weights[HIDDEN_LAYERS] = LEARNING_RATE * error.cwiseProduct(outputActivationValues);

    errorCum.push_back(error);

    for (int i = weights.size()-1; i > 0; --i) {
        Eigen::VectorXd error = hiddenActivationValues[i].cwiseProduct(
            Eigen::VectorXd::Ones(hiddenActivationValues[i].size()) - hiddenActivationValues[i]).cwiseProduct(
                weights[i].cwiseProduct(errorCum[weights.size()-i]));

        weights[i] += LEARNING_RATE * error.cwiseProduct(hiddenActivationValues[i]);

        errorCum.push_back(error);
    }
}