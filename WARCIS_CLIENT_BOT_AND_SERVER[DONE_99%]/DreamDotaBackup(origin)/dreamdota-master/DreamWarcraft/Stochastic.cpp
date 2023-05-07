#include "stdafx.h"
#include "Stochastic.h"
#include "Tools.h"
#include <sstream>


Dice::Dice () : sides(), factor() { }

Dice::Dice (uint32_t _sides) : sides(_sides), factor(1.0f) { }

void Dice::operator *= (float scale) {factor *= scale;}

float Dice::maxValue () const {
	return sides * factor;
}

float Dice::minValue () const {
	return factor;
}

float Dice::expectValue () const{
	return (1 + sides) * factor / 2;
}

bool Dice::ident(const Dice &other) const{
	return (sides == other.sides && factor == other.factor);
}

const std::string Dice::toString() const{
	std::ostringstream s;
	s << "(" << sides << " x " << factor << ")";
	return s.str();
}

DnDAtkDist::DnDAtkDist() : _base(), _dices() { }

DnDAtkDist::DnDAtkDist(float base) : _base(base), _dices() { }

DnDAtkDist::DnDAtkDist (float base, Dice atkDice, uint32_t atkDiceCount) : _base(base), _dices() {
	if (atkDiceCount) _dices.assign(atkDiceCount, atkDice);//FIXME: �Ƿ���Ҫreserve
}

const std::string DnDAtkDist::toString() const{
	std::ostringstream s;
	s << _base;
	//ͳ����������
	Dice lastDice = Dice();
	uint32_t lastDiceCount = 0;
	for (dicecontainer::const_iterator it = _dices.begin();
		it != _dices.end(); ++it)
	{
		if (!lastDice.ident(*it)){
			if (lastDiceCount){
				s << " + " << lastDiceCount << "d" << (lastDice.toString());
			}
			lastDice = *it;
			lastDiceCount = 1;
		}
		else{
			++lastDiceCount;
		}
	}
	if (lastDiceCount){
		s << " + " << lastDiceCount << "d" << (lastDice.toString());
	}
	return s.str();
}

dicecontainer *DnDAtkDist::dices () {
	return &_dices;
}

void DnDAtkDist::append (Dice d, float baseMod) {
	_base += baseMod;
	_dices.push_back(d);
}

void DnDAtkDist::operator += (float baseMod) {
	_base += baseMod;
}

void DnDAtkDist::operator *= (float factor) {
	_base *= factor;
	for (uint32_t i = 0, size = _dices.size(); i < size; ++i){
		_dices[i].factor *= factor;
	}
}

void DnDAtkDist::operator &= (DnDAtkDist & other) {
	_base += other._base;
	dicecontainer *d = other.dices();
	_dices.insert(_dices.end(), d->begin(), d->end());
}

float DnDAtkDist::maxValue () const {
	float rv = _base;
	for (uint32_t i = 0, size = _dices.size(); i < size; ++i){
		rv += _dices[i].maxValue();
	}
	return rv;
}

float DnDAtkDist::minValue () const {
	float rv = _base;
	for (uint32_t i = 0, size = _dices.size(); i < size; ++i){
		rv += _dices[i].minValue();
	}
	return rv;
}

float DnDAtkDist::expectValue () const{
	float rv = _base;
	for (uint32_t i = 0, size = _dices.size(); i < size; ++i){
		rv += _dices[i].expectValue();
	}
	return rv;
}

typedef std::map<float, float, DistributionLess> //����
	Distribution;
static Distribution D_allSteps[0x100];//�������

float DnDAtkDist::chanceGreaterEqual(float value, bool flagRoundedToInt) const{
	//ԭ���⣺base + {����...} >= value�� ת�����⣺{����...} >= value - base
	//diceMax = �����������ֵ��diceMin = ����������Сֵ
	//distribution d = ��
	//��ÿ������loop:
	//	diceMax, diceMin -= ��ǰ����max, min
	//	d ���� ����: iter.first + diceÿ����ĵ���Ϊ�µļ�ֵ��iter.second * dice����Ϊ�µĸ���
	//	����¼�ֵ + diceMax < x�����Ᵽ����ʵ���ֵ-��
	//	����¼�ֵ + diceMin > x�����Ᵽ����ʵ���ֵ��
	//	���򱣴�(�¼�ֵ, �¸���)

	/*	TODO:
	1. ����һϵ�й�������ƽ��ֵ�����ɱ��Ŀ����Ǵο�ʼ�㣬��ǰ��������Ƶ�����С��1%֮������������ٽ�����������
	������2��3��ʱ�䣬���ݾ�ȷ��>99%��
	2. ʹ��һ�����̼߳��㣨�Զ��cpu����~1��ʱ�䲢�Ҳ�Ӱ�����̣߳�
	*/

	float diceMax = maxValue(), diceMin = minValue();
	if (diceMax < value) return 0;//Ԥ����֤һ���Ƿ��Ѿ�Ϊ�����������0����1
	if (diceMin >= value) return 1;
	diceMax -= _base;
	diceMin -= _base;
	float valueTransformed = value - _base;

	uint32_t stepCount = _dices.size();
	//Distribution *d_allSteps = new Distribution[stepCount];
	float chanceLesser = 0, chanceGreater = 0;

	//ѭ����������
	Distribution start;	start[0] = 1;//��ʼ��һ���ֲ�
	Distribution *distributionLast = &start;
	Distribution *distributionCurrent;


	uint32_t stepCurrent = 0;
	dicecontainer::const_iterator diceIter = _dices.begin(), diceIterEnd = _dices.end();

	while (	diceIter != diceIterEnd) 
	{
	
		diceMax -= diceIter->maxValue();
		diceMin -= diceIter->minValue();
		//����distributionCurrent����d_allSteps[stepCurrent]
		//distributionCurrent����distributionLast��ÿ��Ԫ���뵱ǰ���ӵ���
		float diceChance = 1.f/diceIter->sides;//����ÿ����ĸ��ʶ���1/side
		Distribution::iterator distValueIter, distValueIterEnd;

		distributionCurrent = &(D_allSteps[stepCurrent]);
		distributionCurrent->clear();
		
		if (!distributionLast->empty()){
			
			distValueIter = distributionLast->begin();
			distValueIterEnd = distributionLast->end();
			while(distValueIter!=distValueIterEnd) {
				float currentKeyVal = distValueIter->first;//��ֵ
				float newKeyVal;
				float dicePoint = diceIter->minValue();
				for (uint32_t i = 0; i < diceIter->sides; ++i, dicePoint+=diceIter->factor) {
					newKeyVal = currentKeyVal + dicePoint;
					if (newKeyVal + diceMax < valueTransformed) {
						chanceLesser += distValueIter->second * diceChance;
					}
					else if (newKeyVal + diceMin > valueTransformed) {
						chanceGreater += distValueIter->second * diceChance;
					}
					else {
						if (flagRoundedToInt) newKeyVal = floor(newKeyVal + 0.5f);
						distributionCurrent->operator[](newKeyVal) += distValueIter->second * diceChance;
					}
				}//��ǰ���ӵ���ѭ��
				++distValueIter;
			}//��ǰ�ֲ�Ԫ��ѭ��
			//���ڸ��ʷֲ��γ���3���֣���С���ʣ�������ʺ��������ʣ���map�У�
		}
		distributionLast = distributionCurrent;
		++diceIter;
		++stepCurrent;
		if (stepCurrent >= 0x100) break;//�������
	}//��������ѭ��
	
	//���ع������ + distributionLast�����д��ڵ���valueTransformed�ĸ��ʺ�
	float rvChance = chanceGreater;

	if (!distributionLast->empty()) {
		Distribution::iterator distValueIter, distValueIterEnd;
		for (	distValueIter = distributionLast->begin(),
				distValueIterEnd = distributionLast->end(); 
				distValueIter != distValueIterEnd; 
				++distValueIter	)
		{
			if (distValueIter->first >= valueTransformed){
				rvChance += distValueIter->second;
			}
		}
	}
	return rvChance;
}

Function::Function () : _offset(), _values() { }

Function::Function (float start, float end, float totalValue) {
	int count = (int)(floor((end-start)/RASTER_UNIT));
	_values.assign(count, totalValue/count);
	_offset = start;
}

void Function::setValueFromDiscrete (std::map<float, float, DistributionLess> *discreteValueMap, float minBase) {
	_offset = 0;
	_values.clear();

	//discreteValueMap��ÿ��ֵ��־�Ŵ���ʱ��ʼ��������ֵ��
	if (!discreteValueMap->size()) return;
	
	_offset = discreteValueMap->begin()->first;
	uint32_t index = 0;
	float prevY = 0.f, currX = 0.f, currY = 0.f;
	for (std::map<float, float, DistributionLess>::const_iterator 
		iter = discreteValueMap->begin(); 
		iter != discreteValueMap->end(); 
		++iter) 
	{
		currX = iter->first;
		currY = iter->second; 
		while (_offset + index * RASTER_UNIT < currX) {
			_values.push_back(prevY);
			++index;
		}
		prevY = currY;
	}
	_values.push_back(0);
}

std::pair<float, float> Function::maxima (bool preferLowerKey){
	uint32_t size = _values.size();
	if (size){
		float val; float maxVal = _values[0]; uint32_t maxIndex = 0;
		bool improve;
		uint32_t i = 0;
		for (i = 0; i < size; ++i){
			val = _values[i];
			improve = preferLowerKey? (maxVal < val) : (maxVal <= val);
			if (improve){
				maxVal = val;
				maxIndex = i;
			}
		}
		return std::pair<float, float> (RASTER_UNIT * maxIndex + _offset, maxVal);
	}
	return std::pair<float, float>(-1.f,-1.f);
}

void Function::convolution (std::map<float, float, DistributionLess> *discreteValueMap){
	uint32_t size = _values.size();
	if (_offset && size && discreteValueMap->size()){
		std::map<float, float, DistributionLess> tempValMap;
		float key, value;
		std::map<float, float, DistributionLess>::iterator mapIter;
		for (uint32_t i = 0; i < size; ++i){
			value = _values[i];
			key = RASTER_UNIT * i + _offset;
			for (mapIter = discreteValueMap->begin(); mapIter != discreteValueMap->end(); ++mapIter){
				tempValMap[(int)((key + mapIter->first) / RASTER_UNIT) * RASTER_UNIT] += value * mapIter->second;//TODO �Ƿ�Ҫ�жϲ�Ϊ0?
			}
		}
		setValueFromDiscrete(&tempValMap, 0);
	}
}

void Function::smooth (uint32_t radius) {//TODO ��չsize !
	if (radius == 0) return; //ʲôҲ����Ҫ��
	uint32_t size = _values.size();
	uint32_t smoothSize = (radius*2+1);
	//ÿ����ֵ������ֵ�����˸�����radius���ֵ��ƽ��ֵ
	valuecontainer tempValues = _values;
	float tmp, subVal, addVal; uint32_t tmpIndex;
	bool initCalc = false;
	for (uint32_t i = 0; i < size; ++i){
		tmp = 0;
		if (!initCalc) {
			for (uint32_t rangeIndex = i - radius; rangeIndex <= i + radius; ++rangeIndex){
				tmp += (rangeIndex >= 0 && rangeIndex < size) ?
					tempValues[rangeIndex] : 0;
			}
			_values[i] = (tmp / smoothSize);
			initCalc = true;
		}
		else{
			tmpIndex = i-radius-1;
			subVal = (tmpIndex >= 0 && tmpIndex < size) ? tempValues[tmpIndex] / smoothSize : 0;
			tmpIndex = i+radius;
			addVal = (tmpIndex >= 0 && tmpIndex < size) ? tempValues[tmpIndex] / smoothSize : 0;
			_values[i] = _values[i-1] - subVal + addVal;
		}
	}

}

uint32_t Function::size() const {
	return _values.size();
}

void Function::print () const {//�ƺ���bug
	uint32_t size = _values.size();
	float valueX = _offset;
	float valueXold = 0;
	float valueY, valueYold = -1.f;

	for (uint32_t i = 0; i < size; ++i) {
		valueY = _values[i];
		valueX += RASTER_UNIT;
		if (valueY!=valueYold){
			OutputScreen(10, "values from [%.2f ~ %.2f] = %.3f", valueXold, valueX-RASTER_UNIT, valueY);
			valueYold = valueY;
			valueXold = valueX;
		}
	}
}