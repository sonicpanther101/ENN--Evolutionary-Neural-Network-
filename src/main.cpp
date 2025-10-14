#include "NN.h"
#include <iostream>
#include <Eigen/Dense>
#include <vector>

const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 1200;

int main() {

    NN NN(2,10,2,1, 1);

    Eigen::VectorXd input(2);

    input << 0, 1;

    NN.SetInputValues(input);

    Eigen::VectorXd output = NN.GetOutput();

    std::cout << "Output Values: " << output << std::endl;

    Eigen::VectorXd expectedOutput(1);

    expectedOutput << 1;

    NN.AdjustWeights(expectedOutput);
    
    return 0;
}