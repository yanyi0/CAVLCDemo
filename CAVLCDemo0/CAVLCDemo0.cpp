// CAVLCDemo0.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "CAVLC.h"

using namespace std;

int main()
{
	//����ɨ��֮��ľ�����Ϊ����
	int coeff[16] = { 3, 2, 1, -1, 0, -1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 };

	string cavlc_code = Encoding_cavlc_16x16(coeff);

	cout << cavlc_code << endl;

    return 0;
}

