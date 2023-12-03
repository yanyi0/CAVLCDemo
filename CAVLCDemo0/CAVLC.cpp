#include "stdafx.h"
#include "CAVLC.h"
#include "CAVLC_Map.h"

using namespace std;

//获取矩阵中非零系数的个数
int get_total_coeffs(const int coeff[16]) {
	int ret = 0;
	for (size_t i = 0; i < 16; i++)
	{
		if (coeff[i] != 0)
		{
			ret++;
		}
	}
	return ret;
}
//获取拖尾系数的个数：拖尾系数的个数(TrailingOnes)：取值范围为[0, 3]，表示最高频的几个值为±1的系数的个数。
int get_trailing_ones(const int coeff[16], int trailing[3]) {
	int ret = 0;
	for (size_t idx = 15; idx >= 0; idx--)
	{
		if (abs(coeff[idx] > 1) || ret == 3)
		{
			break;
		}
		else if (abs(coeff[idx]) == 1)
		{
			if (coeff[idx] == 1)
			{
				trailing[ret] = 1;
			}
			else {
				trailing[ret] = -1;
			}
			ret++;
		}
	}
	return ret;
}
//获取除了拖尾系数之外的非零系数，逆序存放入数组里面
int get_levels(const int coeff[16], int levels[], int levelCnt) {
	//逆序存放非零系数,除开是拖尾系数的非零系数
	int levelIdx = levelCnt - 1;
	for (int idx = 0; idx < 16; idx++)
	{
		if (coeff[idx] != 0 && levelIdx >= 0)
		{
			levels[levelIdx] = coeff[idx];
			levelIdx--;
		}
		else
		{
			break;
		}
	}
	return 0;
}

int get_totalzeros_runbefore(const int coeff[16], int *runBefore, int *zerosLeft, int totalCoeffs) {
	int idx = 15, totalZeros = 0,runIdx = 0;
	//从高频向低频遍历
	for (; idx >= 0; idx--)
	{
		if (coeff[idx]) {
			break;
		}
	}
	//coeff[idx]是第一个非零系数
	//保存所有非零系数之前的零系数的个数
	//最高频的非零系数前面有多少个零,也就是从后往前第一个非零系数的前面有多少个零

	//此处要求的是最后一个非零系数前面的零系数的个数:可以列出1个方程式
	//coeff[idx]左边的零系数的个数为totalZeros,coeff[idx]右边的零系数的个数为16-idx-1,全部的零系数的个数=16-全部的非零系数的个数
	//16 - totalcoeffs = 16 - idx - 1 + totalZeros   =====> totalZeros = idx - totalCoeffs + 1;
	totalZeros = idx - totalCoeffs + 1;

	for( ; idx >= 0; idx--)
	{
		//如果是零系数，跳过，不做处理
		if (coeff[idx] == 0) {
			continue;
		}
		//若为非零系数，计算有多少个连续性的前缀零
		for (int run = 0; run <= idx; run++)
		{
			//判断idx下标的前一个元素是否为零
			if (coeff[idx - 1 - run] == 0)
			{
				//idx左边有多少个连续的0
				runBefore[runIdx]++;
				//idx左边总共有多少个0
				zerosLeft[runIdx]++;
			}
			else {//若idx前面的一个元素不为零，计算位置++
				runIdx++;
				break;
			}
		}
	}
	//[3, 2, 1, -1, 0, -1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]非零系数的个数为6
	//zerosLeft计算每一个非零系数左边有多少个零，总共[2,1,0,0,0,0]
    //runBefore计算每一个非零系数左边有多少个连续零[1,1,0,0,0,0]
	for (int a = 0; a < runIdx; a++)
	{
		for (int b = a + 1; b < runIdx; b++) {
			zerosLeft[a] += zerosLeft[b];
		}
	}

	return totalZeros;
}

//level为普通非零系数的幅值，传入引用是因为可能会对后缀长度进行更新，level为普通非零系数的幅值
string encode_levels(int level, int &suffixLength) {
	string retStr;
	int levelCode = 0;
	if (level > 0)
	{
		levelCode = (level << 1) - 2;
	}
	else {
		levelCode = -(level << 1) - 1;
	}
	//level转化为levelCode

	//获取前缀后缀值
	int levelPrefix = levelCode / (1 << suffixLength);
	int levelSuffix = levelCode % (1 << suffixLength);
	//对前缀后缀的值进行编码：前缀后缀，前缀的值跟codeword的对应关系在标准文档239页 Table9-6，前缀是几，就在前面补几个零，后面加1
	for (int idx = 0; idx < levelPrefix; idx++)
	{
		retStr += "0";
	}
	retStr += "1";

	//编码后缀：已知后缀的值levelSuffix和后缀的长度suffixLength
	for (int idx = 0; idx < suffixLength; idx++)
	{
		if ((levelSuffix >> (suffixLength - idx - 1) & 1) == 1) {
			retStr += "1";
		}
		else
		{
			retStr += "0";
		}
	}
	return retStr;

}

string Encoding_cavlc_16x16(const int coeff[16])
{
	string cavlcCode;
	//所有非零系数的个数
	int totalCoeffs = get_total_coeffs(coeff);
	//拖尾系数的符号,拖尾系数最多3个
	int trailingSign[3] = {0};
	//拖尾系数的个数,也属于非零系数
	int trailingOnes = get_trailing_ones(coeff,trailingSign);
	//获取普通非零系数的个数:总共的非零系数减去拖尾系数
	int levelCnt = totalCoeffs - trailingOnes;

	//保存普通的非零系数,除开了拖尾系数
	int *levels = new int[levelCnt];
	memset(levels, 0, sizeof(int) * levelCnt);
	get_levels(coeff,levels,levelCnt);

	//保存每一个非零系数前有多少个连续的零系数
	int *runBefore = new int[totalCoeffs];
	memset(runBefore, 0, sizeof(int) * totalCoeffs);
	//表示每一个非零系数前总共还剩多少个零
	int *zerosLeft = new int[totalCoeffs];
	memset(zerosLeft, 0, sizeof(int) * totalCoeffs);
	//表示零系数的总个数
	int totalZeros = get_totalzeros_runbefore(coeff,runBefore,zerosLeft,totalCoeffs);

	//coeff_token 先编码非零系数
	//从3开始到16一共有4行,表格中大于等于3的列下标都满足(totalCoeffs - 3) * 4 + trailingOnes + 6
	if (totalCoeffs >= 3) {
		cavlcCode += coeffTokenMap[0][(totalCoeffs - 3) * 4 + trailingOnes + 6];
	}
	else if(totalCoeffs <= 1)//表格里面的第1，2，3行，在字典中就是第1，2，3行列下标正好是totalCoeffs + trailingOnes
	{
		cavlcCode += coeffTokenMap[0][totalCoeffs + trailingOnes];
	}
	else if (totalCoeffs == 2)//表格里面中totalCoeffs=2正好有三行，在字典中的列下标是totalCoeffs + trailingOnes + 1
	{
		cavlcCode = coeffTokenMap[0][totalCoeffs + trailingOnes + 1];
	}
	//编码拖尾系数trailingSign 拖尾系数的符号
	for (int idx = 0; idx < trailingOnes; idx++)
	{
		if (trailingSign[idx] == 1) {//拖尾系数的符号为1时为0
			cavlcCode += "0";
		}
		else if (trailingSign[idx] == -1)//拖尾系数的符号为-1时为1
		{
			cavlcCode += "1";
		}
	}

	//levels 编码拖尾系数之外的普通非零系数的值 相对比较麻烦
	//每一个level分为前缀和后缀分别进行编码
	int suffixLength = 0;//后缀长度
	if (totalCoeffs > 10 && trailingOnes < 3 )//非零系数的个数大于10，拖尾系数的个数小于3，后缀长度置为1
	{
		suffixLength = 1;
	}
	for (int idx = 0; idx < levelCnt; idx++)
	{
		cavlcCode += encode_levels(levels[idx], suffixLength);//为什么叫做上下文自适应的二进制编码，除了coeffTokenMap，此处会对后缀长度进行更新，体现了上下文的思想
		//更新的原则是刚刚编码的levels[idx]大于某一个阈值公式在标准文档239页，就suffixLength ++ 自增1
		if ((abs(levels[idx]) > (suffixLength == 0 ? 0:(3 << (suffixLength-1 )))) && suffixLength < 6 )
		{
			suffixLength++;
		}
	}

	//编码TotalZeros最高频的非零系数前总共有几个0,参照标准文档239页Table-9-7
	cavlcCode += totalZerosMap[totalZeros][totalCoeffs];

	//编码RunBefore:最后一个非零系数前面的0不需要编码，也就是说最低频的非零系数前的零不需要编码
	for (int idx = 0; idx < totalCoeffs - 1; idx++)
	{
		if (zerosLeft[idx] == 0)//说明前面已经没有需要编码的数据了
		{
			break;
		}
		//除此之外可以通过表中的数据查找对应的码流
		cavlcCode += runBeforeMap[runBefore[idx]][zerosLeft[idx]];
	}

	delete[] levels;
	delete[] runBefore;
	delete[] zerosLeft;
	return cavlcCode;
}
