#include "SetCalculator.h"

#include "Union.h"
#include "Intersection.h"
#include "Difference.h"
#include "Product.h"
#include "Comp.h"
#include "Identity.h"
#include <istream>
#include <ostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <stdexcept>


namespace rng = std::ranges;

SetCalculator::SetCalculator(std::istream &istream, std::ostream &ostream)
        : m_actions(createActions()), m_operations(createOperations()), m_istr(istream), m_ostr(ostream) {


}

void SetCalculator::run() {

    initMaxOperations();

    do {
        printOperations();
        m_ostr << "Enter command ('help' for the list of available commands): ";
        try {
            readLine(m_istr);
            if (m_ss.str().empty())continue;
            auto action = readAction();
            runAction(action);
        } catch (std::runtime_error &e) {
            m_ostr << e.what() << std::endl;
        }
    } while (m_running);
}

void SetCalculator::eval() {

    if (auto index = readOperationIndex(); index) {
        const auto &operation = m_operations[*index];
        auto inputs = std::vector<Set>();
        for (auto i = 0; i < operation->inputCount(); ++i) {
            inputs.push_back(Set(m_ss));
        }
        if (auto buffer = std::string();m_ss >> buffer)throw inputNotMatching("too many argument");
        operation->print(m_ostr, inputs);
        m_ostr << " = " << operation->compute(inputs) << std::endl;
    }
}

void SetCalculator::del() {
    if (auto i = readOperationIndex(); i) {
        if (auto buffer = std::string();m_ss >> buffer)throw inputNotMatching("too many argument");
        m_operations.erase(m_operations.begin() + *i);
    }

}

void SetCalculator::help() {
    if (auto buffer = std::string();m_ss >> buffer)throw inputNotMatching("too many argument");
    m_ostr << "The available commands are:" << std::endl;
    for (const auto &action: m_actions) {
        m_ostr << "* " << action.command << ":" << action.description << std::endl;
    }
    m_ostr << std::endl;
}

void SetCalculator::exit() {
    if (auto buffer = std::string();m_ss >> buffer)throw inputNotMatching("too many argument");
    m_ostr << "Goodbye!" << std::endl;
    m_running = false;
}

void SetCalculator::printOperations() const {
    m_ostr << "List of available set operations:" << std::endl;
    for (decltype(m_operations.size()) i = 0; i < m_operations.size(); ++i) {
        m_ostr << i << ".\t";
        auto gen = NameGenerator();
        m_operations[i]->print(m_ostr, gen);
        m_ostr << std::endl;
    }
    m_ostr << std::endl;
}

std::optional<int> SetCalculator::readOperationIndex() {
    int i = 0;
    if (!(m_ss >> i))throw inputNotMatching("missing or not valid operation num ");
    if (i >= m_operations.size() || i < 0)throw inputNotMatching("operation doesn't exist");

    return i;
}

void SetCalculator::setMaxOperations() {
    int num;
    if (!(m_ss >> num)) throw inputNotMatching("missing or not valid size");
    if (auto buffer = std::string();m_ss >> buffer)throw inputNotMatching("too many argument");
    if (num > m_maxO || num < m_minO)throw inputNotMatching("size out of range");
    if (m_operations.size() > num) {
        m_ostr << "the new max operation size is smaller then the current max operation size \n";
        if (!askToContinue())return;
        deleteOperations(num);
    }
    m_maxOperations = num;
}

void SetCalculator::deleteOperations(int newMax) {
    m_operations.erase(m_operations.begin() + newMax, m_operations.end());
}

bool SetCalculator::askToContinue() {
    do {
        try {
            m_ostr << "press y to continue, c to cancel" << std::endl;
            readLine(m_istr);
            char c;
            if (!(m_ss >> c))throw inputNotMatching("missing or not valid input");
            if (auto buffer = std::string();m_ss >> buffer)throw inputNotMatching("too many argument");
            if (c == 'y')return true;
            if (c == 'c')return false;
        }
        catch (const inputNotMatching &e) {
            m_ostr << e.what() << std::endl;
        }

    } while (true);
}

SetCalculator::Action SetCalculator::readAction() {
    auto action = std::string();
    if (!(m_ss >> action))throw inputNotMatching("missing or not valid command");

    const auto i = std::ranges::find(m_actions, action, &ActionDetails::command);
    if (i != m_actions.end()) {
        return i->action;
    }

    throw inputNotMatching("command not exist!");

}

void SetCalculator::runAction(Action action) {
    switch (action) {
        default:
            m_ostr << "Unknown enum entry used!\n";
            break;

        case Action::Resize:
            setMaxOperations();
            break;
        case Action::Eval:
            eval();
            break;
        case Action::Union:
            binaryFunc<Union>();
            break;
        case Action::Intersection:
            binaryFunc<Intersection>();
            break;
        case Action::Difference:
            binaryFunc<Difference>();
            break;
        case Action::Product:
            binaryFunc<Product>();
            break;
        case Action::Comp:
            binaryFunc<Comp>();
            break;
        case Action::Del:
            del();
            break;
        case Action::Help:
            help();
            break;
        case Action::Read:
            read();
            break;
        case Action::Exit:
            exit();
            break;
    }
}

SetCalculator::ActionMap SetCalculator::createActions() {
    return ActionMap
            {
                    {
                            "read",
                            " path to file ... - read commands from file",
                            Action::Read
                    },
                    {
                            "resize",
                            " num ... - resize the max operations size",
                            Action::Resize
                    },
                    {
                            "eval",
                            "(uate) num ... - compute the result of function #num on the "
                            "following set(s); each set is prefixed with the count of numbers to"
                            " read",
                            Action::Eval
                    },
                    {
                            "uni",
                            "(on) num1 num2 - Creates an operation that is the union of "
                            "operation #num1 and operation #num2",
                            Action::Union
                    },
                    {
                            "inter",
                            "(section) num1 num2 - Creates an operation that is the "
                            "intersection of operation #num1 and operation #num2",
                            Action::Intersection
                    },
                    {
                            "diff",
                            "(erence) num1 num2 - Creates an operation that is the "
                            "difference of operation #num1 and operation #num2",
                            Action::Difference
                    },
                    {
                            "prod",
                            "(uct) num1 num2 - Creates an operation that returns the product of"
                            " the items from the results of operation #num1 and operation #num2",
                            Action::Product
                    },
                    {
                            "comp",
                            "(osite) num1 num2 - creates an operation that is the composition "
                            "of operation #num1 and operation #num2",
                            Action::Comp
                    },
                    {
                            "del",
                            "(ete) num - delete operation #num from the operation list",
                            Action::Del
                    },
                    {
                            "help",
                            " - print this command list",
                            Action::Help
                    },
                    {
                            "exit",
                            " - exit the program",
                            Action::Exit
                    }
            };
}

SetCalculator::OperationList SetCalculator::createOperations() {
    return OperationList
            {
                    std::make_shared<Union>(std::make_shared<Identity>(), std::make_shared<Identity>()),
                    std::make_shared<Intersection>(std::make_shared<Identity>(), std::make_shared<Identity>()),
                    std::make_shared<Difference>(std::make_shared<Identity>(), std::make_shared<Identity>())
            };
}

void SetCalculator::openFile(std::ifstream &file) {
    auto pathToFile = std::string();

    if (!(m_ss >> pathToFile))throw std::iostream::failure("missing or not valid file path");
    if (auto buffer = std::string();m_ss >> buffer)throw inputNotMatching("too many argument");

    file.open(pathToFile);
    if (file.fail())throw std::iostream::failure("missing or not valid file path");
}

void SetCalculator::readFromFile(std::ifstream &file) {
    auto lineNum = 0;

    while (std::getline(file, m_line)) {
        m_ss = std::stringstream(m_line);
        if (m_ss.str().empty())continue;
        if (file.fail() || m_ss.fail())throw std::iostream::failure("read failed");
        ++lineNum;
        try {
            auto action = readAction();
            runAction(action);
        }
        catch (std::runtime_error &e) {
            m_ostr << "in line: " << lineNum << " " << e.what() << std::endl;
           // if (!askToContinue()) break;
        }
    }
}

void SetCalculator::read() {
    std::ifstream file;
    openFile(file);
    readFromFile(file);

}

void SetCalculator::readLine(std::istream &stream) {
    std::getline(stream, m_line);
    m_ss = std::stringstream(m_line);
    if (stream.fail() || m_ss.fail())throw std::iostream::failure("read failed");
}

void SetCalculator::initMaxOperations() {
    while (true) {
        try {
            m_ostr << "hi :) please insert max operation num between " << m_minO << " to " << m_maxO << std::endl;
            readLine(m_istr);
            if (m_ss.str().empty())continue;
            setMaxOperations();
            break;
        }
        catch (std::runtime_error &e) {
            m_ostr << e.what() << std::endl;
        }
    }
}
