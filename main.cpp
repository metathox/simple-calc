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
        void debug(const std::vector<Token>& tokens, const std::string& stage);

        double evaluateExpr();
        void displayResult() const;
};

// ----------------------------
// Input expression from user
// ----------------------------
void Calculator::inputExpr()
{
    std::getline(std::cin, expr);
}

// ----------------------------
// Operator precedence
// 'u' is unary minus
// ----------------------------
int Calculator::precedence(char op)
{
    switch(op)
    {
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        case 'u': return 3;  // unary minus
        case '^': return 4;
        case '%': return 5;
        default: return 0;
    }
}

// ----------------------------
// Apply binary operations
// ----------------------------
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

// ----------------------------
// Tokenize the input expression
// Handles numbers, operators, parentheses, unary minus
// ----------------------------
std::vector<Calculator::Token> Calculator::tokenize()
{
    std::vector<Token> tokens;
    size_t i = 0;

    while(i < expr.size())
    {
        char c = expr[i];

        // Skip whitespace
        if(isspace(c)) { ++i; continue; }

        // ----------------------------
        // Handle unary minus as 'u' operator
        // Occurs at start, after operator, or after '('
        // ----------------------------
        if(c == '-' && (i == 0 || tokens.empty() || tokens.back().type == Token::OPERATOR || tokens.back().type == Token::PAREN_LEFT))
        {
            tokens.push_back({Token::OPERATOR, 0, 'u'});
            ++i;
            continue;
        }

        // ----------------------------
        // Parse numbers (integers or decimals)
        // ----------------------------
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

        // ----------------------------
        // Parse binary operators
        // ----------------------------
        if(c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%')
        {
            tokens.push_back({Token::OPERATOR, 0, c});
            ++i;
            continue;
        }

        // ----------------------------
        // Parse parentheses
        // ----------------------------
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
        // ++i REMOVED 166 166 166
    }

    return tokens;
}

// ----------------------------
// Convert tokens to postfix (RPN) using Shunting Yard
// Handles operator precedence and right-associativity
// ----------------------------
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

// ----------------------------
// Evaluate postfix expression
// Handles unary minus 'u' and binary operators
// ----------------------------
double Calculator::evaluatePostfix(const std::vector<Token>& postfix)
{
    std::stack<double> st;

    for(const auto &tok : postfix)
    {
        if(tok.type == Token::NUMBER)
        {
            st.push(tok.value);
            std::cout << "\nPush " << tok.value << " onto stack\n";
        }

        else if(tok.type == Token::OPERATOR)
        {
            if(tok.op == 'u')
            {
                if(st.empty())
                    throw std::runtime_error("Invalid expression: missing operand for unary minus.");

                double x = st.top();
                st.pop();
                st.push(-x);
                std::cout << "Unary minus applied: -" << x << " -> pushed " << -x << "\n";
            }

            else if(tok.op == '%')
            {
                if(st.empty())
                    throw std::runtime_error("Invalid expression: missing operand for '%'.");
                
                double x = st.top();
                st.pop();

                double r = x / 100.0;
                st.push(r);
                std::cout << "Percent applied: " << x << "% -> pushed " << r << "\n";
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
                std::cout << "Applying " << tok.op << " to " << x << " and " << y << " -> " << r << "\n";
            }
        }
    }

    if(st.size() != 1)
        throw std::runtime_error("Invalid expression: malformed expression or missing operators.");

    return st.top();
}

// ----------------------------
// Evaluate the expression: tokenize -> postfix -> evaluate
// ----------------------------
double Calculator::evaluateExpr()
{
    auto tokens = tokenize();
    debug(tokens, "After Tokenization");

    auto postfix = toPostfix(tokens);
    debug(postfix, "Postfix Conversion");

    result = evaluatePostfix(postfix);
    return result;
}

void Calculator::debug(const std::vector<Token>& tokens, const std::string& stage)
{
    std::cout << "\n--- Debug: " << stage << " ---\n";

    for(const auto &tok : tokens)
    {
        switch(tok.type)
        {
            case Token::NUMBER:
                std::cout << "Number: " << tok.value << "\n";
                break;
            case Token::OPERATOR:
                std::cout << "Operator: " << tok.op << "\n";
                break;
            case Token::PAREN_LEFT:
                std::cout << "Paren: (\n";
                break;
            case Token::PAREN_RIGHT:
                std::cout << "Paren: )\n";
                break;
        }
    }

    std::cout << "-----------------------------------\n";
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
