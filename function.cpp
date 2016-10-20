#include <functional>
#include <iostream>
 
struct Foo_Object {
    Foo_Object(int num) : num_(num) {}
    void print_add(int i) const { std::cout << num_+i << '\n'; }
    int num_;
};
 
void print_num_function(int i)
{
    std::cout << i << '\n';
}
 
struct PrintNum_Object {
    void operator()(int i) const
    {
        std::cout << i << '\n';
    }
};
 
int main()
{
    // store a free function
    std::function<void(int)> f_display = print_num_function;
    f_display(-9);
 
    // store a lambda
    std::function<void()> f_display_42 = []() { print_num_function(42); };
    f_display_42();
 
    // store the result of a call to std::bind
    std::function<void()> f_display_31337 = std::bind(print_num_function, 31337);
    f_display_31337();
 
    // store a call to a member function
    std::function<void(const Foo_Object&, int)> f_add_display = &Foo_Object::print_add;
    const Foo_Object foo(314159);
    f_add_display(foo, 1);
 
    // store a call to a data member accessor
    std::function<int(Foo_Object const&)> f_num = &Foo_Object::num_;
    std::cout << "num_: " << f_num(foo) << '\n';
 
    // store a call to a member function and object
    using std::placeholders::_1;
    std::function<void(int)> f_add_display2 = std::bind( &Foo_Object::print_add, foo, _1 );
    f_add_display2(2);
 
    // store a call to a member function and object ptr
    std::function<void(int)> f_add_display3 = std::bind( &Foo_Object::print_add, &foo, _1 );
    f_add_display3(3);
 
    // store a call to a function object
    std::function<void(int)> f_display_obj = PrintNum_Object();
    f_display_obj(18);
}