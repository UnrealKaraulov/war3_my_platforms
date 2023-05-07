#ifndef STOCHASTIC_H_
#define STOCHASTIC_H_

#include <vector>

struct DistributionLess {
	bool operator() (const float &lhs, const float &rhs) const
	{return lhs < rhs -1.e-5;}
};

//����
struct Dice {
	uint32_t	sides;		//����������m�����ӣ�ֵΪ 1 ~ m�����1
	float		factor;		//���ű�����factor = 0.5��m������ֵΪ 0.5 ~ 0.5m�����0.5

	Dice ();
	Dice (uint32_t _sides);	//��ʼ������Ϊ��ʼֵ1�����1��m������
	
	void operator *= (float scale);	//��������

	float		maxValue() const;
	float		minValue() const;
	float		expectValue() const;

	bool		ident(const Dice &other) const;

	const std::string toString() const;
};

typedef std::vector<Dice> dicecontainer;
//��ɢ�ֲ�
//����/�˺�����ֲ�: ����ֵ + {����1 ... ����n}
class DnDAtkDist {

	float			_base;				//����ֵ�����ڵ�������������ӦdmgPlus + buffs�������˺��ܺͣ���Ӧ�˺�����������ܺ͡�
	dicecontainer	_dices;

public:
	DnDAtkDist ();
	DnDAtkDist (float base);
	DnDAtkDist (float base, Dice atkDice, uint32_t atkDiceCount);

	dicecontainer *dices ();

	float		maxValue() const;//TODO: �ڶ�����cache
	float		minValue() const;
	float		expectValue() const;

	void append (Dice d, float baseMod = 0);	//���һ�����Ӳ��ҿ��Ըı����ֵ
	void operator += (float baseMod);			//�ı����ֵ
	void operator *= (float factor);
	void operator &= (DnDAtkDist & other);		//�ϲ����������ֲ�: ����ֵ��ӣ����Ӳ��뼯��

	float chanceGreaterEqual(float value, bool flagRoundedToInt) const;//����ڵ��ڸ���ֵ�ĸ���

	const std::string toString() const;
};

const float RASTER_UNIT = 0.01f;		//�������ڵ�Ԫ������Ӧ��xֵ��
typedef std::vector<float> valuecontainer;
//�����ֲ�
//����������㷨��������
class Function {

	float			_offset;			//ƫ�ƣ�����������ݶ�Ӧ��xֵ
	valuecontainer	_values;

public:

	Function ();
	Function (float start, float end, float totalValue);
	void setValueFromDiscrete (std::map<float, float, DistributionLess> *discreteValueMap, float minBase);
	std::pair<float, float> maxima (bool preferLowerKey = true);
	void convolution (std::map<float, float, DistributionLess> *discreteValueMap);
	void smooth (uint32_t radius);//һάƽ��ƽ��
	void shift(float val){_offset += val;}//λ��key

	uint32_t size() const;
	void print() const;
};







#endif