#include <iostream>
#include "nqueue.h"
#include "lecsema.h"
#include "nstack.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <cmath>
#include "calc.h"

using namespace std;

void setError(int * status , string textOfError)
{
    *status=-1;
    cout<<textOfError<<endl;
}

bool baseCheck(string & str)
{
    if(str.empty())
    {
        cout<<"Пустая строка."<<endl;
        return false;
    }
    for(int i=0; i <= str.length(); i++)
    {
        if(isalpha(str[i]))
        {
            cout<<"Есть буквы"<<endl;
            return false;
        }
        if(str[i]==' '|| str[i]=='\t')
        {
            str.erase(i,1);
            i--;
        }
    }
    return true;
}

double executeOperand(char operand, double a, double b)
{
    switch(operand)
    {
    case '-':
        return b-a;
        break;
    case '+':
        return b+a;
        break;
    case '/':
        return b/a;
        break;
    case '*':
        return b*a;
        break;
    case '^':
        return pow(b,a);
        break;
    default:
        return 0.0;
        break;
    }

}
double calculate(NQueue * base, int * status)
{
    NStack * forOperands=new NStack(base->length()/2);
    NQueue * forNumbers=new NQueue(base->length()/2);
    Lecsema * buf;
    Number * num;
    Operand * oper;
    Operand * bufoper;
    while(1)
    {
        buf=base->get();
        if(buf==NULL)
        {
            while(forOperands->length())
            {
                forNumbers->add(forOperands->get());
            }
            break;
        }
        num=dynamic_cast<Number*>(buf);
        if(num)
        {
            forNumbers->add(buf);
            continue;
        }
        oper=dynamic_cast<Operand*>(buf);
        if(oper)
        {
            while(1)
            {
                bufoper=dynamic_cast<Operand*>(forOperands->cherryPick());
                if(bufoper)
                {
                    if( (bufoper->getPriority() >= oper->getPriority()) && (oper->getPriority()>0) )
                    {
                        forNumbers->add(forOperands->get());
                        continue;
                    }
                    else
                    {
                        if(oper->getSim()==')')
                        {
                            while(1)
                            {
                                Lecsema * buf=forOperands->get();
                                Operand * oper=dynamic_cast<Operand*>(buf);
                                if(oper)
                                {
                                    if(oper->getSim()=='(')
                                    {
                                        break;
                                    }
                                    forNumbers->add(buf);
                                }
                            }
                            break;
                        }
                        forOperands->add(buf);
                        break;
                    }
                }
                else
                {
                    forOperands->add(buf);
                    break;
                }
            }
        }
        else
        {
            setError(status,"Ошибка dynamic cast");
        }

    }


    NStack * calculating=new NStack(forNumbers->length());
    while(1)
    {
        if(forNumbers->length()==0)
        {
            break;
        }
        buf=forNumbers->get();
        num=dynamic_cast<Number*>(buf);
        oper=dynamic_cast<Operand*>(buf);
        if(num)
        {
            calculating->add(buf);
        }
        else if(oper)
        {
            double result;
            double a=0;
            double b=0;
            Number * num1=dynamic_cast<Number*>(calculating->get());
            Number * num2=dynamic_cast<Number*>(calculating->get());
            if(num1 && num2)
            {
                a=num1->getNum();
                b=num2->getNum();
            }
            else
            {
                setError(status,"Не удалось посчитать постфиксную форму");
            }
            result=executeOperand(oper->getSim(),a,b);
            num=new Number(result);
            buf=dynamic_cast<Lecsema*>(num);
            if(buf)
            {
                calculating->add(buf);
            }
        }
    }
    num=dynamic_cast<Number*>(calculating->get());
    if(num)
    {
        return num->getNum();
    }
    else
    {
        setError(status,"Ошибка dynamic cast");
        return 0;
    }

}

void addMinus(NQueue * queue)
{
    Lecsema * minusOne;
    minusOne=dynamic_cast<Lecsema*>(new Operand('(',0));
    queue->add(minusOne);
    minusOne=dynamic_cast<Lecsema*>(new Number(-1));
    queue->add(minusOne);
    minusOne=dynamic_cast<Lecsema*>(new Operand(')',0));
    queue->add(minusOne);
    minusOne=dynamic_cast<Lecsema*>(new Operand('*',2));
    queue->add(minusOne);
}

double calc(string str , int * status)
{
    string expression=str;

    if(!baseCheck(expression))
    {
        setError(status,"Что-то не так с выражением");
        return 0;
    }

    stringstream bufstr;
    bufstr<<expression;

    bool minus=false;
    bool space=false;
    bool wasDigit=false;
    int priority = -1;
    double number;
    int counter=0;//счетчик открытых - закрытых скобок если 0 то все ок

    NQueue * queue= new NQueue(expression.length()-1);
    Number * num;
    Operand * oper;
    Lecsema * buf;
    int i=0;
    while(1)
    {
        char c=(char)bufstr.peek();//stream.peek если число то считать из потока число иначе символ кидать в элс. минус привязать к числу только в начале строки и после открывающей скобки.
        if(c==EOF)
        {
            break;
        }
        if((int)c==32)
        {
            space=true;
            while((int)c==32)
            {
                c=bufstr.get();
            }
            bufstr.unget();
            continue;
        }
        if(isdigit(c))
        {
            i++;
            if(space && wasDigit)
            {
                setError(status,"Ошибка, два числа разделены пробелом");
                return 0;
            }
            bufstr>>number;
            if(minus)
            {
                number=-number;
            }
            num=new Number(number);
            buf=dynamic_cast<Lecsema*>(num);
            if(buf)
            {
                queue->add(buf);
            }
            else
            {
                setError(status, "Ошибка dynamic cast");
            }
            wasDigit=true;
            space=false;
            continue;
        }
        else
        {
            bufstr>>c;
            if(Operand::analyzeChar(c,minus,priority,counter))
            {
                buf=queue->showLast();
                oper=dynamic_cast<Operand*>(buf);
                if(oper)//Если последний символ был операцией
                {
                    if(oper->getPriority() > 0 && priority > 0)//Если предыдущий и текущий символ не скобочки , то ошибка *- и т.д.
                    {
                        setError(status,"Ошибка. Два знака операции подряд (не скобочки) ");
                        return 0;
                    }
                    if(minus && oper->getSim()=='(')// Если после открывающейся скобочки
                    {
                        if((char)bufstr.peek()=='(')
                        {
                            addMinus(queue);
                            continue;
                        }
                        cout<<"Минус будет дописан к числу"<<endl;
                        continue;
                    }
                }
                if(minus && (char)bufstr.peek()=='(' && (i==0))
                {
                    addMinus(queue);
                    continue;
                }
                if(minus && (i==0) && isdigit((char)bufstr.peek()) )
                {
                    cout<<"Минус будет написан к числу"<<endl;
                    continue;
                }
                else
                {
                    minus=false;
                }
                oper=new Operand(c);
                oper->setPriority(priority);
                buf=dynamic_cast<Lecsema*>(oper);
                if(buf)
                {
                    queue->add(buf);
                }
                else
                {
                    setError(status,"Ошибка. dynamic_cast");
                    return 0;
                }
                wasDigit=false;
            }
            else
            {
                setError(status,"Ошибка. Неопознанный символ");
                break;
            }
        }
    }
    if(counter!=0)
    {
        setError(status,"Ошибка. Не все скобки закрыты");
        cout<<"Скобки не закрыты до конца "<<endl;
        return 0;
    }
    return calculate(queue,status);
}
