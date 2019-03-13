#ifndef LECSEMA_H
#define LECSEMA_H
#include <cstddef>



class Lecsema
{
public:
    Lecsema();
    virtual ~Lecsema()=0;
};

class Number : public Lecsema
{
public:
    Number(double num=0);
    double getNum();
    void setNum(double num);
private:
    double m_num;
};

class Operand : public Lecsema
{
public:
    static bool analyzeChar(char c, bool &minus, int &priority, int &counter);
    Operand(char sim=0,int priority=0);
    char getSim();
    int getPriority();
    void setSim(char sim);
    void setPriority(int priority);
private:
    int m_priority;
    char m_sim;
};

#endif // LECSEMA_H
