#include <functional>
#include <iostream>
using namespace std;
using namespace std::placeholders;

class Foo
{
public:
    Foo(int x):num_(new int(x))
    {
        cout << "构造函数调用\n";
    };

    ~Foo()
    {
        cout << "析构调用\n";
        delete num_;
        num_ = nullptr;
    }

    // Foo(const Foo& f)
    // {
    //     cout << "拷贝调用\n";
    //     this->num_ = new int(*f.num_);
    // }

    void print(int x) const
    {
        cout << __FUNCTION__ << "Foo: " << x << "\n";
    }
private:
    int* num_;
};

void print(int a, float b, char c)
{
    cout << a << " " << b << " " << c << " \n";
}

void fprint(const Foo& f, int x)
{
    f.print(x);
}

int main()
{
    Foo f(2);
    function<void(Foo&, int)> func = &Foo::print;
    func(f, 1);

    function<void()> func2 = bind(&Foo::print, &f, 6);
    func2();

    auto func3 = bind(print, _2, _3, _1);
    function<void(char, int , float)> func4 = bind(print, _2, _3, _1);
    func3('A', 10, 1.45);
    // func4(10,'A', 1.45);
    cout << "************************************\n";
    {
        Foo f2(4);
        auto func5 = bind(&Foo::print, &f2, 10);
        func5();
    }
    cout << "end1\n";
    cout << "************************************\n";
    {
        Foo f3(4);
        auto func5 = bind(&Foo::print, f3, 6);
        func5();
    }
    cout << "end2\n";
    cout << "************************************\n";
    Foo f4(6);
    {
        auto func5 = bind(&Foo::print, f4, 7);
    }
    cout << "end3\n";
}