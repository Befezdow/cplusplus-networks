#include "lecsema.h"
#include <iostream>
using namespace std;
Lecsema::Lecsema()
{

}

Lecsema::~Lecsema()
{

}

Number::Number(double num)
{
    m_num=num;
}

double Number::getNum()
{
    return m_num;
}

void Number::setNum(double num)
{
    m_num=num;
}

Operand::Operand(char sim, int priority)
{
    m_sim=sim;
    this->setPriority(priority);
}

char Operand::getSim()
{
    return m_sim;
}

void Operand::setSim(char sim)
{
    m_sim=sim;
}

void Operand::setPriority(int priority)
{
    if(priority<0 || priority >3)
    {
        cout<<"Ошибка приоритета"<<endl;
        return;
    }
    m_priority=priority;
}
int Operand::getPriority()
{
    return m_priority;
}

bool Operand::analyzeChar(char c, bool &minus, int &priority, int &counter)
{
    char definedChar=false;
    switch (c)
    {
    case '(':
        definedChar=true;
        counter++;
        priority = 0;
        break;
    case ')':
        definedChar=true;
        counter--;
        priority = 0;
        break;
    case '-':
        minus=true;
        definedChar=true;
        priority = 1;
        break;
    case '+':
        definedChar=true;
        priority = 1;
        break;
    case '/':
        definedChar=true;
        priority = 2;
        break;
    case '*':
        definedChar=true;
        priority = 2;
        break;
    case '^':
        definedChar=true;
        priority = 3;
        break;
    default:
        definedChar=false;
        cout<<"Неопределеный символ. Все очень плохо"<<endl;
        break;
    }
    return definedChar;
}
