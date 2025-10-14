#include <vector>
#include <Eigen/Dense>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

class NN {
public:
    NN(int INPUT_NODES, int HIDDEN_NODES, int HIDDEN_LAYERS, int OUTPUT_NODES, double LEARNING_RATE);

    void SetInputValues(Eigen::VectorXd vector);

    Eigen::VectorXd ReLU(Eigen::VectorXd vector);

    Eigen::VectorXd Propagate(Eigen::MatrixXd weights, Eigen::VectorXd activationValue, Eigen::VectorXd biases);

    Eigen::VectorXd GetOutput();

    // Δwij =η×δj×Oj
    double Cost(double outputValue, double expectedValue);

    void AdjustWeights(Eigen::VectorXd expectedOutput);
private:
    int INPUT_NODES;
    int HIDDEN_NODES;
    int OUTPUT_NODES;
    int HIDDEN_LAYERS;
    double LEARNING_RATE;

    Eigen::VectorXd inputActivationValues;
    std::vector<Eigen::VectorXd> hiddenActivationValues;
    Eigen::VectorXd outputActivationValues;

    std::vector<Eigen::MatrixXd> weights;
    std::vector<Eigen::VectorXd> biases;
};