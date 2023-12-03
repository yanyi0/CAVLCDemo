#include "stdafx.h"
#include "CAVLC.h"
#include "CAVLC_Map.h"

using namespace std;

//��ȡ�����з���ϵ���ĸ���
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
//��ȡ��βϵ���ĸ�������βϵ���ĸ���(TrailingOnes)��ȡֵ��ΧΪ[0, 3]����ʾ���Ƶ�ļ���ֵΪ��1��ϵ���ĸ�����
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
//��ȡ������βϵ��֮��ķ���ϵ��������������������
int get_levels(const int coeff[16], int levels[], int levelCnt) {
	//�����ŷ���ϵ��,��������βϵ���ķ���ϵ��
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
	//�Ӹ�Ƶ���Ƶ����
	for (; idx >= 0; idx--)
	{
		if (coeff[idx]) {
			break;
		}
	}
	//coeff[idx]�ǵ�һ������ϵ��
	//�������з���ϵ��֮ǰ����ϵ���ĸ���
	//���Ƶ�ķ���ϵ��ǰ���ж��ٸ���,Ҳ���ǴӺ���ǰ��һ������ϵ����ǰ���ж��ٸ���

	//�˴�Ҫ��������һ������ϵ��ǰ�����ϵ���ĸ���:�����г�1������ʽ
	//coeff[idx]��ߵ���ϵ���ĸ���ΪtotalZeros,coeff[idx]�ұߵ���ϵ���ĸ���Ϊ16-idx-1,ȫ������ϵ���ĸ���=16-ȫ���ķ���ϵ���ĸ���
	//16 - totalcoeffs = 16 - idx - 1 + totalZeros   =====> totalZeros = idx - totalCoeffs + 1;
	totalZeros = idx - totalCoeffs + 1;

	for( ; idx >= 0; idx--)
	{
		//�������ϵ������������������
		if (coeff[idx] == 0) {
			continue;
		}
		//��Ϊ����ϵ���������ж��ٸ������Ե�ǰ׺��
		for (int run = 0; run <= idx; run++)
		{
			//�ж�idx�±��ǰһ��Ԫ���Ƿ�Ϊ��
			if (coeff[idx - 1 - run] == 0)
			{
				//idx����ж��ٸ�������0
				runBefore[runIdx]++;
				//idx����ܹ��ж��ٸ�0
				zerosLeft[runIdx]++;
			}
			else {//��idxǰ���һ��Ԫ�ز�Ϊ�㣬����λ��++
				runIdx++;
				break;
			}
		}
	}
	//[3, 2, 1, -1, 0, -1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]����ϵ���ĸ���Ϊ6
	//zerosLeft����ÿһ������ϵ������ж��ٸ��㣬�ܹ�[2,1,0,0,0,0]
    //runBefore����ÿһ������ϵ������ж��ٸ�������[1,1,0,0,0,0]
	for (int a = 0; a < runIdx; a++)
	{
		for (int b = a + 1; b < runIdx; b++) {
			zerosLeft[a] += zerosLeft[b];
		}
	}

	return totalZeros;
}

//levelΪ��ͨ����ϵ���ķ�ֵ��������������Ϊ���ܻ�Ժ�׺���Ƚ��и��£�levelΪ��ͨ����ϵ���ķ�ֵ
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
	//levelת��ΪlevelCode

	//��ȡǰ׺��׺ֵ
	int levelPrefix = levelCode / (1 << suffixLength);
	int levelSuffix = levelCode % (1 << suffixLength);
	//��ǰ׺��׺��ֵ���б��룺ǰ׺��׺��ǰ׺��ֵ��codeword�Ķ�Ӧ��ϵ�ڱ�׼�ĵ�239ҳ Table9-6��ǰ׺�Ǽ�������ǰ�油�����㣬�����1
	for (int idx = 0; idx < levelPrefix; idx++)
	{
		retStr += "0";
	}
	retStr += "1";

	//�����׺����֪��׺��ֵlevelSuffix�ͺ�׺�ĳ���suffixLength
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
	//���з���ϵ���ĸ���
	int totalCoeffs = get_total_coeffs(coeff);
	//��βϵ���ķ���,��βϵ�����3��
	int trailingSign[3] = {0};
	//��βϵ���ĸ���,Ҳ���ڷ���ϵ��
	int trailingOnes = get_trailing_ones(coeff,trailingSign);
	//��ȡ��ͨ����ϵ���ĸ���:�ܹ��ķ���ϵ����ȥ��βϵ��
	int levelCnt = totalCoeffs - trailingOnes;

	//������ͨ�ķ���ϵ��,��������βϵ��
	int *levels = new int[levelCnt];
	memset(levels, 0, sizeof(int) * levelCnt);
	get_levels(coeff,levels,levelCnt);

	//����ÿһ������ϵ��ǰ�ж��ٸ���������ϵ��
	int *runBefore = new int[totalCoeffs];
	memset(runBefore, 0, sizeof(int) * totalCoeffs);
	//��ʾÿһ������ϵ��ǰ�ܹ���ʣ���ٸ���
	int *zerosLeft = new int[totalCoeffs];
	memset(zerosLeft, 0, sizeof(int) * totalCoeffs);
	//��ʾ��ϵ�����ܸ���
	int totalZeros = get_totalzeros_runbefore(coeff,runBefore,zerosLeft,totalCoeffs);

	//coeff_token �ȱ������ϵ��
	//��3��ʼ��16һ����4��,����д��ڵ���3�����±궼����(totalCoeffs - 3) * 4 + trailingOnes + 6
	if (totalCoeffs >= 3) {
		cavlcCode += coeffTokenMap[0][(totalCoeffs - 3) * 4 + trailingOnes + 6];
	}
	else if(totalCoeffs <= 1)//�������ĵ�1��2��3�У����ֵ��о��ǵ�1��2��3�����±�������totalCoeffs + trailingOnes
	{
		cavlcCode += coeffTokenMap[0][totalCoeffs + trailingOnes];
	}
	else if (totalCoeffs == 2)//���������totalCoeffs=2���������У����ֵ��е����±���totalCoeffs + trailingOnes + 1
	{
		cavlcCode = coeffTokenMap[0][totalCoeffs + trailingOnes + 1];
	}
	//������βϵ��trailingSign ��βϵ���ķ���
	for (int idx = 0; idx < trailingOnes; idx++)
	{
		if (trailingSign[idx] == 1) {//��βϵ���ķ���Ϊ1ʱΪ0
			cavlcCode += "0";
		}
		else if (trailingSign[idx] == -1)//��βϵ���ķ���Ϊ-1ʱΪ1
		{
			cavlcCode += "1";
		}
	}

	//levels ������βϵ��֮�����ͨ����ϵ����ֵ ��ԱȽ��鷳
	//ÿһ��level��Ϊǰ׺�ͺ�׺�ֱ���б���
	int suffixLength = 0;//��׺����
	if (totalCoeffs > 10 && trailingOnes < 3 )//����ϵ���ĸ�������10����βϵ���ĸ���С��3����׺������Ϊ1
	{
		suffixLength = 1;
	}
	for (int idx = 0; idx < levelCnt; idx++)
	{
		cavlcCode += encode_levels(levels[idx], suffixLength);//Ϊʲô��������������Ӧ�Ķ����Ʊ��룬����coeffTokenMap���˴���Ժ�׺���Ƚ��и��£������������ĵ�˼��
		//���µ�ԭ���Ǹոձ����levels[idx]����ĳһ����ֵ��ʽ�ڱ�׼�ĵ�239ҳ����suffixLength ++ ����1
		if ((abs(levels[idx]) > (suffixLength == 0 ? 0:(3 << (suffixLength-1 )))) && suffixLength < 6 )
		{
			suffixLength++;
		}
	}

	//����TotalZeros���Ƶ�ķ���ϵ��ǰ�ܹ��м���0,���ձ�׼�ĵ�239ҳTable-9-7
	cavlcCode += totalZerosMap[totalZeros][totalCoeffs];

	//����RunBefore:���һ������ϵ��ǰ���0����Ҫ���룬Ҳ����˵���Ƶ�ķ���ϵ��ǰ���㲻��Ҫ����
	for (int idx = 0; idx < totalCoeffs - 1; idx++)
	{
		if (zerosLeft[idx] == 0)//˵��ǰ���Ѿ�û����Ҫ�����������
		{
			break;
		}
		//����֮�����ͨ�����е����ݲ��Ҷ�Ӧ������
		cavlcCode += runBeforeMap[runBefore[idx]][zerosLeft[idx]];
	}

	delete[] levels;
	delete[] runBefore;
	delete[] zerosLeft;
	return cavlcCode;
}
