#include <cassert>
#include<iostream>
#include <math.h>
#include<iomanip>
#include<fstream>
#include<string>
#include<iostream>
#include<vector>

double LEARNINGRATE = 1;

using namespace std;


// Net를 초기화 하기 전 Net의 크기를 결정할 벡터를 만드는 함수입니다.
vector<int> before_start() {
	cout << "before start\n[node][node] = > layer 0  ....input layer\n[node][node][node] = > layer 1	....hidden layer\n[node][node][node] = > layer 2	....hidden layer\n[node][node] = > layer 3  ....output layer		\n\n위의 그림에선, layer size = 4, ..인풋 레이어와 아웃풋 레이어를 합쳐서 레이어의 층을 입력해 주시면 됩니다.\n이후 각 layer에 있어야 할 노드의 수를 입력해주세요." << endl;
	cout << "show 함수는 각 레이어에서 입력한 노드보다 한개가 더 나옵니다. 마지막 노드는 bias값을 계산하는 노드 입니다.\nbias 노드의 weight 값이 다음 노드들의 bias값입니다." << endl;
	int size;
	cout << "layers 의 수 : ";
	cin >> size;
	cout << endl;
	vector<int> topV(size, 0);
	int tmp;
	for (int i = 0; i < size; i++) {
		if (i == 0) {
			cout << "input layer의 갯수를 입력해주세요";
			cin >> tmp;
			cout << endl;
		}
		else if (i == size - 1) {
			cout << "output layer의 갯수를 입력해주세요";
			cin >> tmp;
			cout << endl;
		}
		else {
			cout << i << "th layer의 갯수를 입력해주세요";
			cin >> tmp;
			cout << endl;
		}
		topV[i] = tmp;
	}
	cout << endl;
	return topV;
}

// 목표를 설정합니다.
int set_input_output() {
	cout << "학습할 형식을 선택하세요.\n";
	cout << "1. AND GATE \t2.OR GATE \t3.XOR GATE \t4. DONUT \t :  ";
	int result;
	cin >> result;
	return result;
}


//활성함수 입니다.
double transferFunc(double x) {
	//	return tanh(x);
	return 1 / (1 + exp(-x));
	//		return 0 < x ? x: 0.1*x ;
}

//활성함수 입니다.
double transferDerivative(double x) {
	//return 1 - x * x;
	return x * (1 - x);
	//	return  0 < x ? 1 :0.1;
}

double random01() {
	return (rand() / double(RAND_MAX)) * 2 - 1;
}
class Node;					// 뉴런이 되는 각 노드입니다. 변수에 대한 설명은 아래에 있습니다.,
class Net;					// 전체 구조를 클래스로 저장했습니다. 변수에 대한 설명은 아래에 있습니다., 
class Matrix;				// 각 노드의 아웃풋 값을 구하기위해 행렬을 구현하여 행렬곱을 이용했습니다.
typedef vector<Node> Layer;	// 퍼셉트론의 각 레이어를 나타냅니다. 따로 변수가 필요없어서, 따로 클래스를 구현하지않고 typedef 만 이용했습니다. 

// 아웃풋 계산을 위해 행렬곱을 사용하므로, 행렬 클래스를 구현하여,
// 클래스의 맴버 함수로 행렬곱을 구현하였습니다.
// 각 노드는 다음 노드를 향한 가중치를 이 행렬에 저장합니다.
class Matrix {
public:
	Matrix() {
		Matrix(0, 0);
	}
	Matrix(int row, int col);

	int rowNum;
	int colNum;

	Matrix transpose();
	vector<vector<double>> matrix;
	Matrix multiply(Matrix& left);
	void setRandom();
};


void Matrix::setRandom() {
	for (int i = 0; i < rowNum; i++) {
		for (int j = 0; j < colNum; j++) {
			this->matrix[i][j] = random01();
		}
	}
}

Matrix Matrix::multiply(Matrix& left) {
	Matrix result = Matrix(this->rowNum, left.colNum);
	double tmp = 0;
	for (int i = 0; i < this->rowNum; i++) {
		for (int j = 0; j < left.colNum; j++) {
			tmp = 0;
			for (int k = 0; k < this->colNum; k++) {
				tmp += this->matrix[i][k] * left.matrix[k][j];
			}
			result.matrix[i][j] = tmp;
		}
	}

	return result;
}

Matrix::Matrix(int row, int col) {
	rowNum = row;
	colNum = col;
	matrix = vector<vector<double>>(row, vector<double>(col, 0));
}

/*
각 뉴런을 노드라는 클래스로 구현하였습니다.
lecun's backpropagation을 목표로 하였기 때문에 각 노드에 델타 정보를 저장합니다.
행렬곱으로 아웃풋을 구할 것이기 때문에, 다음 노드를 향한 가중치 값은 matrix행렬로 구현하였습니다.

*/
class Node {
private:
	int nextLayerSize;
	double bias = random01();

public:
	double outputVal;
	double delta;
	void calLastDelta(double d);				// 마지막 레이어의 gradient 를 구하기 위해 사용합니다. 
	void updateHiddenDelta(Layer& next);		// 히든 레이어, 인풋레이어의 gradient 를 구합니다.
	int myIndex;
	double getBias() { return bias; };
	void setBias(double b) { bias = b; };
	Matrix myWeight;							//행렬곱으로 아웃풋을 구할 것이기 때문에, 다음 노드를 향한 가중치 값은 matrix행렬로 구현하였습니다.
	vector<double> myDeltaWeight;				// lecun's backpropagation을 위해 delta 가중치를 구하는데 필요한 재료를 미리 저장합니다.
												// 그 결과, 재귀함수로서 가중치를 구할 필요가 없어지고 성능의 향상을 노릴 수 있었습니다.
	void updateWeights_and_bias(Layer& prev);	// weight값, bias을 수정합니다 
	void setOutput(double o) { outputVal = o; };
	double getOutput() { return outputVal; };
	Node(int nextLayerSize, int myIndex);
};

void Node::updateWeights_and_bias(Layer& prev) {
	//다음 레이어를 향한 weight값을 수정합니다.
	for (unsigned nodeNum = 0; nodeNum < prev.size(); nodeNum++) {
		Node& N = prev[nodeNum];
		double tmpDeltaWeight = N.getOutput() * this->delta;
		N.myWeight.matrix[0][myIndex] -= tmpDeltaWeight * LEARNINGRATE;

	}
	// 노드마다 한번 씩, bias 값을 수정합니다.
	this->bias -= delta * LEARNINGRATE;
}

void Node::updateHiddenDelta(Layer& next) {

	// 다음 레이어의 delta값*weight 값의 합 입니다.
	// 다음레이어의 deltaWeight는 항상 먼저 계산되기 때문에 오류없이 작동합니다. 
	double sum = 0;
	for (unsigned nodeNum_ = 0; nodeNum_ < next.size(); nodeNum_++) {
		sum += this->myWeight.matrix[0][nodeNum_] * next[nodeNum_].delta;
	}

	delta = sum * transferDerivative(getOutput());

}
// 마지막 레이어의 deltaWeight 값은 항상 먼저 계산되어야하고,
// 다른 layer와 달리, weight 값이 곱해지지 않기때문에 따로 계산 함수를 만듭니다.
void Node::calLastDelta(double goal) {
	double tmp = goal - getOutput();
	delta = -(tmp * transferDerivative(getOutput()));


}




Node::Node(int nextLayerSize, int myIndex) {
	this->myIndex = myIndex;
	this->nextLayerSize = nextLayerSize;

	this->myIndex = myIndex;
	this->bias = random01();

	myDeltaWeight = vector<double>(nextLayerSize, 0);
	myWeight = Matrix(1, nextLayerSize);
	myWeight.setRandom();
}


/*
	노드의 정보와 전체 퍼셉트론 구조의 크기등을 저장한 클래스입니다.
*/
class Net {
public:
	void show();
	Net(vector<int> P);
	void computeOutput(vector<double>& input);	// 각 노드의 아웃풋을 계산합니다. 행렬곱을 이용하여 레이어에 속한 노드들을 한번에 계산합니다.
	void backProp(vector<double>& goalVal);		// delta weight 값을 먼저 업데이트하고 저장한 후 그 값을 이용하여 weight 값을 수정합니다.
	void setRMSE0() { RMSE = 0; };
	double getRMSE() { return RMSE; };

	//아래는 과제, 보고서 분석에 필요한 데이터를 텍스트 파일로 저장하는 함수입니다.
	ofstream os;
	ofstream os2;
	ofstream os3;
	ofstream recordRMSE;
	ofstream recordSmallRMSE;
	ofstream record_weight;
	void saveLines();
	void saveWeights();
	void saveOnlyWeight();
	void saveRMSE(int i, int inputSize);
private:
	vector<Layer>  myLayers;
	double RMSE = 0;
};

void Net::saveRMSE(int i, int inputSize) {						//시행 횟수에 따른 RMSE 에러를 저장합니다.
	recordRMSE << this->getRMSE() / inputSize << " " << i << "\n";
	if (this->getRMSE() / 4 < 0.001 && this->getRMSE() / 4 > 0.0005) {
		recordSmallRMSE << this->getRMSE() / 4 << " " << i << "\n";
	}
}

void Net::saveLines() {	// save 2 dimentional lines  and each RMSE total error
	os3 << myLayers[0][0].myWeight.matrix[0][0] << "x+" << myLayers[0][1].myWeight.matrix[0][0] << "y +" << "(" << myLayers[1][0].getBias() << ") =0 \n";
	os3 << myLayers[0][0].myWeight.matrix[0][1] << "x+" << myLayers[0][1].myWeight.matrix[0][1] << "y +" << "(" << myLayers[1][1].getBias() << ")=0 \n";
	os3 << "\n";

}

void Net::saveWeights() {
	for (unsigned layerNum_ = 0; layerNum_ < myLayers.size(); layerNum_++) {
		for (unsigned nodeNum = 0; nodeNum < myLayers[layerNum_].size(); nodeNum++) {
			os << "( ";
			for (unsigned weiNum = 0; weiNum < myLayers[layerNum_][nodeNum].myWeight.matrix[0].size(); weiNum++) {
				os << myLayers[layerNum_][nodeNum].myWeight.matrix[0][weiNum] << ", ";
			}
			os << "bias: " << myLayers[layerNum_][nodeNum].getBias() << ")\t";
		}
		os << "\n";
	}
}
void Net::saveOnlyWeight() {
	for (unsigned layerNum_ = 0; layerNum_ < myLayers.size(); layerNum_++) {
		for (unsigned nodeNum = 0; nodeNum < myLayers[layerNum_].size(); nodeNum++) {
			for (unsigned weiNum = 0; weiNum < myLayers[layerNum_][nodeNum].myWeight.matrix[0].size(); weiNum++) {
				record_weight << myLayers[layerNum_][nodeNum].myWeight.matrix[0][weiNum] << " ";
			}
			record_weight << " ";
		}
		record_weight << "\n";
	}
}


/*
	Lecun's bakpropagation 모델을 따른 구현입니다.
		1) 마지막 레이어의 gradient 계산, 저장
		2) 히든레이어, 인풋레이어의 gradient 계산,저장
		3) 위의 결과를 바탕으로 bias와 가중치 값 수정.
	위의 순서대로 진행합니디.
*/

void Net::backProp(vector<double>& goalVal) {
	double totalError = 0;
	Layer& lastLayer = myLayers.back();

	totalError = goalVal[0] - lastLayer[0].getOutput();	//아웃풋이 하나라고 가정한 에러 구하기 입니다.
	RMSE += totalError * totalError;

	// 마지막 레이어의 gradient 값을 계산하고.
	for (unsigned nodeNum_ = 0; nodeNum_ < lastLayer.size(); nodeNum_++) {
		lastLayer[nodeNum_].calLastDelta(goalVal[nodeNum_]);
	}
	// 계산된 마지막 layer의 gradient 를 이용하여 뒤에서 부터 히든 레이어의 gradient를 계산합니다.
	for (int layerNum_ = myLayers.size() - 2; layerNum_ >= 0; layerNum_--) {
		Layer& current = myLayers[layerNum_];
		Layer& next = myLayers[layerNum_ + 1];

		for (unsigned nodeNum_ = 0; nodeNum_ < current.size(); nodeNum_++) {
			current[nodeNum_].updateHiddenDelta(next);
		}
	}
	// 그리고 위의 계산된 결과로 delta weight를 구하여 각 weight에 적용합니다. 
	for (unsigned layerNum = myLayers.size() - 1; layerNum > 0; layerNum--) {
		Layer& current = myLayers[layerNum];
		Layer& prev = myLayers[layerNum - 1];

		for (unsigned nodeNum = 0; nodeNum < current.size(); nodeNum++) {
			current[nodeNum].updateWeights_and_bias(prev);
		}

	}


}

// 행렬곱을 이용한 각 노드의 아웃풋 계산입니다.
// 행렬곱을 사용하므로 레이어에 속한 노드의 아웃풋이 한번에 계산됩니다.
void Net::computeOutput(vector<double>& input) {
	//인풋 레이어의 아웃풋 값을 인풋값으로 바꿉니다.
	for (unsigned inputNum = 0; inputNum < input.size(); inputNum++) {
		myLayers[0][inputNum].outputVal = input[inputNum];
	}

	for (unsigned i = 1; i < myLayers.size(); i++) {
		// 레이어 마다 한번에 계산할겁니다. 레이어에 접근 할 수 있는 변수를 설정합니다.
		Layer& before = myLayers[i - 1];
		Layer& current = myLayers[i];

		// 이전 레이어의 아웃풋을 행렬곱이 가능한 형태로 만듭니다.
		Matrix outputMat(1, before.size());
		for (int j = 0; j < before.size(); j++) {
			outputMat.matrix[0][j] = before[j].getOutput();
		}

		//이전 레이어의 가중치를 행렬곱이 가능한 형태로 만듭니다.
		Matrix weightMat(before.size(), current.size());
		int tmp = 0;
		for (int j = 0; j < weightMat.rowNum; j++) {
			for (int k = 0; k < weightMat.colNum; k++) {
				weightMat.matrix[j][k] = before[j].myWeight.matrix[0][k];
			}
		}

		// 행렬곱의 결과를 담을 행렬을 만듭니다.
		Matrix result(1, current.size());
		// 두행렬을 곱하고 다음 레이어의 노드의 결과값으로 넣어줍니다.
		result = outputMat.multiply(weightMat);
		for (int j = 0; j < current.size(); j++) {
			current[j].outputVal = transferFunc(result.matrix[0][j] + current[j].getBias());
		}
	}

}
void Net::show() {
	for (unsigned layerNum_ = 0; layerNum_ < myLayers.size(); layerNum_++) {
		for (unsigned nodeNum = 0; nodeNum < myLayers[layerNum_].size(); nodeNum++) {
			cout << "[ O:" << myLayers[layerNum_][nodeNum].getOutput() << ", W: ";
			for (unsigned weiNum = 0; weiNum < myLayers[layerNum_][nodeNum].myWeight.matrix[0].size(); weiNum++) {
				cout << myLayers[layerNum_][nodeNum].myWeight.matrix[0][weiNum] << "| ";
			}
			cout << "bias: " << myLayers[layerNum_][nodeNum].getBias() << "]\t\t";
		}
		cout << "=>Layer: " << layerNum_ << "\n" << endl;

	}
}

Net::Net(vector<int>p) {
	RMSE = 0;
	for (unsigned layerNum_ = 0; layerNum_ < p.size(); layerNum_++)
	{
		myLayers.push_back(Layer());
		int sizeNextLayer = layerNum_ == p.size() - 1 ? 0 : p[layerNum_ + 1];
		for (unsigned nodeNum_ = 0; nodeNum_ < p[layerNum_]; nodeNum_++) {
			myLayers[layerNum_].push_back(Node(sizeNextLayer, nodeNum_));
		}
	}

}


int main()
{
	srand((unsigned int)time(NULL));
	vector<vector<double>> input_AND = { {0,0},{0,1},{1,0},{1,1} };
	vector<vector<double>> output_AND = { {0},{0},{0},{1} };

	vector<vector<double>> input_OR = { {0,0},{0,1},{1,0},{1,1} };
	vector<vector<double>> output_OR = { {0},{1},{1},{1} };

	vector<vector<double>> input_XOR = { {0,0},{0,1},{1,0},{1,1} };
	vector<vector<double>> output_XOR = { {1},{0},{0},{1} };

	vector<vector<double>> input_DONUT = { {0,0},
	 {0,1},
	 {1,0},
	 {1,1},
	 {0.5,1},
	 {1,0.5},
	 {0,0.5},
	 {0.5,0},
	 {0.5,0.5} };
	vector<vector<double>> output_DONUT = { {0},{0},{0},{0},{0},{0},{0},{0},{1} };
	vector<vector<vector<double>>> inputs = { input_AND,input_OR,input_XOR, input_DONUT };
	vector<vector<vector<double>>> outputs = { output_AND,output_OR,output_XOR, output_DONUT };


	vector<int> tmp = before_start();
	Net n(tmp);
	vector<double> temp = { 1,0 };

	int x = set_input_output();			// 학습 목표를 설정합니다.
	vector<vector<double>> input_target = inputs[x - 1];
	vector<vector<double>> output_target = outputs[x - 1];


	n.os.open("saveWeights.txt");		// 가중치를 행렬로 저장합니다. os
	n.os3.open("saveLines.txt");		// 첫번째 레이어에서 도출되는 직선을 저장합니다.
	n.recordRMSE.open("rmse.txt");		// 시행에 따른 RMSE에러를 저장합니다.
	n.record_weight.open("onlyWeight.txt");

	int iter_num = 10000;

	LEARNINGRATE = 2;

	// 학습을 실행합니다. 탈출조건은 20000번 시행 혹은, rmse 에러가 0.0001 미만 입니다.
	for (int i = 0; i < iter_num; i++) {

		for (int j = 0; j < input_target.size(); j++) {
			n.computeOutput(input_target[j]);
			n.backProp(output_target[j]);
		}

		n.saveRMSE(i, input_target.size());

		if ((i + 1) % 50 == 0 || i == 0) {
			cout << "\nRMSE in " << i << "th iteration : " << n.getRMSE() / input_target.size() << endl;;
			n.saveWeights();
			n.saveOnlyWeight();
			n.os3 << "EPOCH : " << i << " RMSE : " << n.getRMSE() / input_target.size() << "\n";
			n.saveLines();
		}

		if (n.getRMSE() / input_target.size() < 0.0001) {
			n.os3 << "EPOCH : " << i << " RMSE : " << n.getRMSE() / input_target.size() << "\n";
			n.saveLines();
			n.setRMSE0();
			break;
		}
		n.setRMSE0();
		if (i == iter_num - 1) {
			n = Net(tmp);
			cout << " reset Weights." << endl;
			i = 0;
			n.os.open("saveWeights.txt");		// 가중치를 행렬로 저장합니다. os
			n.os3.open("saveLines.txt");		// 첫번째 레이어에서 도출되는 직선을 저장합니다.
			n.recordRMSE.open("rmse.txt");		// 시행에 따른 RMSE에러를 저장합니다.
			n.record_weight.open("onlyWeight.txt");


		}
	}
	n.show();
}

