/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Gianni Angelone
 */

#include "std_lib_facilities.h"

struct Token {
    char kind;
    double value;
    string name;
    Token(char ch) :kind(ch), value(0) { } //These are our operators
    Token(char ch, double val) :kind(ch), value(val) { }
    Token(char ch, string n) :kind(ch), name(n) { }
};

class Token_stream {
    bool full;
    Token buffer;
public:
    Token_stream() :full(0), buffer(0) { }
    Token get();
    void unget(Token t) { buffer = t; full = true; }
    void ignore(char); //Not accounting unused characters
};

const char let = 'L'; //User variables
const char quit = 'Q'; //To quit the program
const char print = ';'; //Prints to screen
const char number = '8';
const char name = 'a'; //Indicator
const char con = 'C'; //Constant

Token Token_stream::get() //Grab from user
{
    if (full) { full = false; return buffer; }
    char ch;
    cin >> ch;
    switch (ch) {
        case '(':
        case ')':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case ';':
        case '=':
            return Token(ch);
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {	cin.unget();
            double val;
            cin >> val;
            return Token(number, val);
        }
        default:
            if (isalpha(ch) || ch == '_') {
                string s;
                s += ch;
                while(cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) { //Account in the underscore
                    s+=ch;
                }
                cin.unget();
                if (s == "let") return Token(let); //Return variable defined by user
                if (s == "const") return Token(con); //Return constant defined by user
                if (s == "quit") return Token(quit); //End the program
                return Token(name, s);
            }
            error("Bad token");
    }
    return 0;
}

void Token_stream::ignore(char c)
{
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;
    char ch;
    while (cin >> ch)
        if (ch == c) return;
}

struct Variable { //Struct for names of variables
    string name;
    double value;
    bool is_const;
    Variable(string n, double v, bool b) :name(n), value(v), is_const(b) { }
};

vector<Variable> names;

double get_value(string s) //Gets the value
{
    for (int i = 0; i < names.size(); ++i)
        if (names[i].name == s) return names[i].value;
    error("get: undefined name ", s);
}

void set_value(string s, double d) //Sets the value
{
    for (int i = 0; i <= names.size(); ++i) {
        if (names[i].name == s) {
            names[i].value = d;
            return;
        }
    }
    error("set: undefined name ", s);
}

bool is_declared(string s) //Checks if name has already been declared
{
    for (int i = 0; i < names.size(); ++i)
    {
        if (names[i].name == s && names[i].is_const == true)
            error("Cannot reassign const variable");
        else if (names[i].name == s && names[i].is_const == false)
            return true;
    }

    return false;
}

Token_stream ts;

double expression();

double primary()
{
    Token t = ts.get();
    switch (t.kind) {
        case '(':
        {	double d = expression();
            t = ts.get();
            if (t.kind != ')') error("'(' expected");
            return d;
        }
        case '-':
            return -primary();
        case '+':
            return primary();
        case number:
            return t.value;
        case name:
        {
            Token t2 = ts.get();
            if (t2.kind == '=')
            {	// handle name = expression
                double d = expression();
                set_value(t2.name,d);
                return d;
            }
            else {
                ts.unget(t2);
                return get_value(t2.name);
            }
        }
        default:
            error("primary expected");
    }
}

double term()
{
    double left = primary();
    while (true) {
        Token t = ts.get();
        switch (t.kind) {
            case '*':
                left *= primary();
                break;
            case '/':
            {	double d = primary();
                if (d == 0) error("divide by zero");
                left /= d;
                break;
            }
            default:
                ts.unget(t);
                return left;
        }
    }
}

double expression()
{
    double left = term();
    Token t = ts.get();
    while (true) {
        switch (t.kind) {
            case '+':
                left += term();
                t = ts.get();
                break;
            case '-':
                left -= term();
                t = ts.get();
                break;
            case '=':
                error("use of '=' outside of a declaration");
            default:
                ts.unget(t);
                return left;
        }
    }
}

double declaration()
{
    Token t = ts.get();
    bool isC;
    if (t.kind == 'C')
    {
        isC = true;
        t = ts.get();  //Gets next word
    }
    else
        isC = false;

    if (t.kind != 'a')
        error("name expected in declaration;"); //Name error

    string name = t.name;
    if (is_declared(name))
    {
        cout << name + ", has been declared twice. Would you like to reenter your value with another variable? y/n > ";
        cin.clear();
        cin.ignore(10000, '\n');
        string response;
        getline(cin, response);
        if (response == "n")
            error(name, ", will not be reassigned; ");
        if (response == "y")
        {
            cout << "(Please reenter a new value: ";
            int val;
            cin >> val;
            set_value(name, val);
            double d = val;
            return d; //Calculator is reset
        }
    }

    Token t2 = ts.get();
    if (t2.kind != '=')
        error("= missing in declaration of ", name); //Equal sign error

    double d = expression();
    names.push_back(Variable(name, d, isC));

    return d;
}

double statement()
{
    Token t = ts.get();
    switch (t.kind) {
        case let:
            return declaration();
        case con:
            return declaration();
        default:
            ts.unget(t);
            return expression();
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
    while (true) try {
            cout << prompt;
            Token t = ts.get();
            while (t.kind == print) t = ts.get();
            if (t.kind == quit) return;
            ts.unget(t);
            cout << result << statement() << endl;
        }
        catch (runtime_error& e) {
            cerr << e.what() << endl;
            clean_up_mess();
        }
}

int main()

try {
    calculate(); //Does the calculations and returns proper error message
    return 0;
}
catch (exception& e) {
    cerr << "exception: " << e.what() << endl;
    char c;
    while (cin >> c && c != ';');
    return 1;
}
catch (...) {
    cerr << "exception\n";
    char c;
    while (cin >> c && c != ';');
    return 2;
}