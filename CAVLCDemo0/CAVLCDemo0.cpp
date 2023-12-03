// CAVLCDemo0.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "CAVLC.h"

using namespace std;

int main()
{
	//经过扫描之后的矩阵作为输入
	int coeff[16] = { 3, 2, 1, -1, 0, -1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 };

	string cavlc_code = Encoding_cavlc_16x16(coeff);

	cout << cavlc_code << endl;

    return 0;
}

