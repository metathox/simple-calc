#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <cmath>
#include <cctype>
#include <stdexcept>

class Calculator
{
    private:
        std::string expr;
        double result;

        struct Token
        {
            enum Type {NUMBER, OPERATOR, PAREN_LEFT, PAREN_RIGHT} type;
            double value;
            char op;
        };

    public:
        void inputExpr();
        std::string getExpr() { return expr; }

        int precedence(char op);
        double applyOperation(double x, double y, char op);

        std::vector<Token> tokenize();
        std::vector<Token> toPostfix(const std::vector<Token>& tokens);
        double evaluatePostfix(const std::vector<Token>& postfix);

        void evaluateExpr();
        void displayResult() const;
};

void Calculator::inputExpr()
{
    std::getline(std::cin, expr);
}

int Calculator::precedence(char op)
{
    switch(op)
    {
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        case 'u': return 3;
        case '^': return 4;
        case '%': return 5;
        default: return 0;
    }
}

double Calculator::applyOperation(double x, double y, char op)
{
    switch(op)
    {
        case '+': return x + y;
        case '-': return x - y;
        case '*': return x * y;
        case '/': 
            if(y == 0)
                throw std::runtime_error("Division by zero!");
            return x / y;
        case '^':
            return pow(x, y);
        default:
            throw std::runtime_error(std::string("Unknown operator: ") + op);
    }
}

std::vector<Calculator::Token> Calculator::tokenize()
{
    std::vector<Token> tokens;
    size_t i = 0;

    while(i < expr.size())
    {
        char c = expr[i];

        if(isspace(c)) { ++i; continue; }

        if(c == '-' && (i == 0 || tokens.empty() || tokens.back().type == Token::OPERATOR || tokens.back().type == Token::PAREN_LEFT))
        {
            tokens.push_back({Token::OPERATOR, 0, 'u'});
            ++i;
            continue;
        }

        if(isdigit(c) || (c == '.' && i + 1 < expr.size() && isdigit(expr[i + 1])))
        {
            std::string numStr;
            bool hasDecimal = false;

            while(i < expr.size() && (isdigit(expr[i]) || expr[i] == '.'))
            {
                if(expr[i] == '.')
                {
                    if(hasDecimal)
                    {
                        throw std::runtime_error("Invalid number: multiple decimal points.");
                    }
                    hasDecimal = true;
                }

                numStr += expr[i++];
            }

            tokens.push_back({Token::NUMBER, std::stod(numStr), 0});
            continue;
        }

        if(c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%')
        {
            tokens.push_back({Token::OPERATOR, 0, c});
            ++i;
            continue;
        }

        if(c == '(')
        {
            tokens.push_back({Token::PAREN_LEFT, 0, 0});
            ++i;
            continue;
        }

        if(c == ')')
        {
            tokens.push_back({Token::PAREN_RIGHT, 0, 0});
            ++i;
            continue;
        }

        throw std::runtime_error(std::string("Unknown character: ") + c);
    }

    return tokens;
}

std::vector<Calculator::Token> Calculator::toPostfix(const std::vector<Token>& tokens)
{
    std::vector<Token> output;
    std::stack<Token> opStack;

    auto isRightAssociative = [](char op) { return op == '^' || op == 'u' || op == '%'; };

    for(const auto &tok : tokens)
    {
        if(tok.type == Token::NUMBER) output.push_back(tok);

        else if(tok.type == Token::OPERATOR)
        {
            while(!opStack.empty() && opStack.top().type == Token::OPERATOR)
            {
                char topOp = opStack.top().op;

                if((!isRightAssociative(tok.op) && this->precedence(topOp) >= this->precedence(tok.op)) ||
                   (isRightAssociative(tok.op) && this->precedence(topOp) > this->precedence(tok.op)))
                {
                    output.push_back(opStack.top()); opStack.pop();
                }
                else break;
            }

            opStack.push(tok);
        }

        else if(tok.type == Token::PAREN_LEFT) opStack.push(tok);

        else if(tok.type == Token::PAREN_RIGHT)
        {
            while(!opStack.empty() && opStack.top().type != Token::PAREN_LEFT)
            {   
                output.push_back(opStack.top());
                opStack.pop();
            }

            if(opStack.empty())
                throw std::runtime_error("Mismatched parentheses: unexpected ')'");
            
            opStack.pop();
        }
    }

    while(!opStack.empty())
    {
        if(opStack.top().type == Token::PAREN_LEFT)
            throw std::runtime_error("Mismatched parentheses: unclosed '('");

        output.push_back(opStack.top());
        opStack.pop();
    }

    return output;
}

double Calculator::evaluatePostfix(const std::vector<Token>& postfix)
{
    std::stack<double> st;

    for(const auto &tok : postfix)
    {
        if(tok.type == Token::NUMBER)
            st.push(tok.value);
            
        else if(tok.type == Token::OPERATOR)
        {
            if(tok.op == 'u')
            {
                if(st.empty())
                    throw std::runtime_error("Invalid expression: missing operand for unary minus.");

                double x = st.top();
                st.pop();
                st.push(-x);
            }

            else if(tok.op == '%')
            {
                if(st.empty())
                    throw std::runtime_error("Invalid expression: missing operand for '%'.");
                
                double x = st.top();
                st.pop();

                double r = x / 100.0;
                st.push(r);
            }

            else
            {
                if(st.size() < 2)
                    throw std::runtime_error("Invalid expression: missing operand for binary operator.");

                double y = st.top();
                st.pop();
                double x = st.top();
                st.pop();

                double r = applyOperation(x, y, tok.op);
                st.push(r);
            }
        }
    }

    if(st.size() != 1)
        throw std::runtime_error("Invalid expression: malformed expression or missing operators.");

    return st.top();
}

void Calculator::evaluateExpr()
{
    auto tokens = tokenize();
    auto postfix = toPostfix(tokens);

    result = evaluatePostfix(postfix);
}

void Calculator::displayResult() const
{
    std::cout << "Answer: " << result << "\n";
}

struct Application
{
    Calculator calc;

    void run()
    {
        greet();

        while(true)
        {
            std::cout << "\nEnter your expression: ";
            calc.inputExpr();

            if(calc.getExpr() == "exit")
            {
                std::cout << "Program finished with exit code 0.\n\n";
                break;
            }

            if(calc.getExpr() == "help")
            {
                std::cout << "\nEnter any mathematical expression using numbers and any of the following operations: (), %, ^, *, /, +, -.";
                std::cout << "\nType 'exit' to close program.\n";
                continue;
            }

            try
            {
                calc.evaluateExpr();
                calc.displayResult();
            }
            catch (const std::runtime_error& exc) { std::cerr << "Error: " << exc.what() << "\n"; }
        }
    }

    void greet()
    {
        std::cout << "\n------ Welcome to Calculator 2.0 ------\n";
        std::cout << "Available operations (PEMDAS): (), %, ^, *, /, +, -. Negative numbers supported!\n";
        std::cout << "Type 'exit' to close program. Type 'help' for hints.\n";
    }
};

int main()
{
    Application app;
    app.run();
    return 0;
}
