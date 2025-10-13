#include <vector>
#include <Eigen/Dense>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

class NN {
public:
    NN(int INPUT_NODES, int HIDDEN_NODES, int OUTPUT_NODES, int HIDDEN_LAYERS);

    void SetInputValues(Eigen::VectorXd vector);

    Eigen::VectorXd ReLU(Eigen::VectorXd vector);

    Eigen::VectorXd Propogate(Eigen::MatrixXd weights, Eigen::VectorXd activationValue, Eigen::VectorXd biases);

    void GetOutput();

    double Cost(Eigen::VectorXd outputActivation, Eigen::VectorXd expectedValue);
private:
    int INPUT_NODES;
    int HIDDEN_NODES;
    int OUTPUT_NODES;
    int HIDDEN_LAYERS;

    Eigen::VectorXd inputActivationValues;
    std::vector<Eigen::VectorXd> hiddenActivationValues;
    Eigen::VectorXd outputActivationValues;

    std::vector<Eigen::MatrixXd> weights;
    std::vector<Eigen::VectorXd> biases;
};