#include "variable.h"

class Method
{
private:
    vector<Variable> methodVariables;
    vector<string> lines;

    string logicOperatorValue,
        methodName,
        className,
        symbolString,
        valueOne_,
        valueTwo_;

    bool isForLoop_,
        isIF_,
        isIndestructible,
        isInfinite_,
        isListLoop_,
        isPrivate_,
        isPublic_,
        isTemplate_,
        isWhileLoop_,
        isBad_;

    int startValue,
        stopValue,
        templateClasses;

    List list;

    Container nest;

    char defaultSymbol;

public:
    Method() {}

    Method(string name)
    {
        initialize(name);
    }

    Method(string name, bool isTemplate)
    {
        initialize(name);
        isTemplate_ = isTemplate;
    }

    ~Method()
    {
        clear();
    }

    void setClass(string name)
    {
        className = name;
    }

    void setIndestructible(bool value)
    {
        isIndestructible = value;
    }

    bool indestructible()
    {
        return isIndestructible;
    }

    string getClass()
    {
        return className;
    }

    /**
     * symbol is the variable containing the current iteration value.
     */
    void setSymbol(string symbol)
    {
        symbolString = symbol;
    }

    /**
     * symbol is the variable containing the current iteration value.
     */
    void setDefaultSymbol(string symbol)
    {
        defaultSymbol = symbol[0];
    }

    string getSymbolString()
    {
        return symbolString;
    }

    char getDefaultSymbol()
    {
        return defaultSymbol;
    }

    void setPrivate()
    {
        isPrivate_ = true;
        isPublic_ = false;
    }

    void setPublic()
    {
        isPublic_ = true;
        isPrivate_ = false;
    }

    bool isPublic()
    {
        return isPublic_;
    }

    bool isPrivate()
    {
        return isPrivate_;
    }

    bool isTemplate()
    {
        return isTemplate_;
    }

    vector<Variable> getMethodVariables()
    {
        return methodVariables;
    }

    void setName(string name)
    {
        methodName = name;
    }

    void add(string line)
    {
        lines.push_back(line);
    }

    void addMethodVariable(string value, string variableName)
    {
        Variable newVariable(variableName, value);
        methodVariables.push_back(newVariable);
    }

    void addMethodVariable(double value, string variableName)
    {
        Variable newVariable(variableName, value);
        methodVariables.push_back(newVariable);
    }

    void addMethodVariable(Variable variable)
    {
        methodVariables.push_back(variable);
    }

    string at(int index)
    {
        if (index < (int)lines.size())
            return lines.at(index);

        return "#!=no_line";
    }

    void buildNest()
    {
        Container newNest(methodName + "<nest>");
        nest = newNest;
    }

    Container getNest()
    {
        return nest;
    }

    void inNest(string line)
    {
        nest.add(line);
    }

    string nestAt(int index)
    {
        if (index < nest.size())
            return nest.at(index);
        else
            return "nothing!!!";
    }

    void clear()
    {
        lines.clear();
        methodVariables.clear();
    }

    vector<string> getLines()
    {
        return lines;
    }

    void initialize(string name)
    {
        defaultSymbol = '$';
        logicOperatorValue = "",
        methodName = name,
        className = "",
        symbolString = "$",
        valueOne_ = "",
        valueTwo_ = "";
        isForLoop_ = false,
        isIF_ = false,
        isIndestructible = false,
        isInfinite_ = false,
        isListLoop_ = false,
        isPublic_ = false,
        isPrivate_ = false,
        isTemplate_ = false,
        isWhileLoop_ = false;
        startValue = 0,
        stopValue = 0,
        templateClasses = 0;
    }

    void setIsBad(bool value)
    {
        isBad_ = value;
    }

    bool isBad()
    {
        return isBad_;
    }

    string name()
    {
        return methodName;
    }

    void setTemplateSize(int size)
    {
        templateClasses = size;
    }

    int getTemplateSize()
    {
        return templateClasses;
    }

    int size()
    {
        return (int)lines.size();
    }

    bool isIF()
    {
        return isIF_;
    }

    bool isForLoop()
    {
        return isForLoop_;
    }

    bool isWhileLoop()
    {
        return isWhileLoop_;
    }

    bool isInfinite()
    {
        return isInfinite_;
    }

    int start()
    {
        return startValue;
    }

    int stop()
    {
        return stopValue;
    }

    string valueOne()
    {
        return valueOne_;
    }

    string valueTwo()
    {
        return valueTwo_;
    }

    string logicOperator()
    {
        return logicOperatorValue;
    }

    void setInfinite()
    {
        isInfinite_ = true;
    }

    void setBool(bool b)
    {
        isIF_ = b;
    }

    void setFor(bool b)
    {
        isForLoop_ = b;
    }

    void setWhile(bool b)
    {
        isWhileLoop_ = b;
    }

    void setWhileValues(string v1, string op, string v2)
    {
        valueOne_ = v1,
        logicOperatorValue = op,
        valueTwo_ = v2;
    }

    void setForValues(int a, int b)
    {
        startValue = a,
        stopValue = b;
    }

    void setForList(List l)
    {
        list = l;
    }

    bool isListLoop()
    {
        return isListLoop_;
    }

    void setListLoop()
    {
        isListLoop_ = true;
    }

    List getList()
    {
        return list;
    }
};