#pragma once

#include <vector>
#include <memory>
#include <string>
#include <iosfwd>
#include <optional>
#include <sstream>
#include "inputNotMatching.h"

class Operation;

class SetCalculator
{
public:
    SetCalculator(std::istream& istream, std::ostream& ostream );
    void run();

private:
    void eval();
    void del();
    void help();
    void exit();
    void readFromFile(std::ifstream &);
    void openFile(std::ifstream &);
    void initMaxOperations();
    template <typename FuncType>
    void binaryFunc()
    {
        if (auto f0 = readOperationIndex(), f1 = readOperationIndex(); f0 && f1)
        {
            if (auto buffer = std::string();m_ss >> buffer)throw inputNotMatching("too many argument");
            if (m_maxOperations <= m_operations.size())throw inputNotMatching("exceed the number of actions allowed");
            m_operations.push_back(std::make_shared<FuncType>(m_operations[*f0], m_operations[*f1]));
        }
    }

    void printOperations() const;

    enum class Action
    {
        Eval,
        Union,
        Intersection,
        Difference,
        Product,
        Comp,
        Del,
        Help,
        Exit,
        Resize,
        Read,
    };

    struct ActionDetails
    {
        std::string command;
        std::string description;
        Action action;
    };

    using ActionMap = std::vector<ActionDetails>;
    using OperationList = std::vector<std::shared_ptr<Operation>>;
    const ActionMap m_actions;
    OperationList m_operations;
    bool m_running = true;
    std::istream& m_istr;
    std::ostream& m_ostr;
    std::stringstream m_ss;
    std::string m_line;
    std::optional<int> readOperationIndex() ;
    int m_maxOperations = 3,m_minO=3,m_maxO=100;
    void setMaxOperations();
    void deleteOperations(int);
    bool askToContinue() ;
    void readLine(std::istream&);
    Action readAction();//const;
    void runAction(Action action);

    static ActionMap createActions();
    static OperationList createOperations();

    void read();
};
