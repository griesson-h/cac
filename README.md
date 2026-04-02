# cac

Interpreter for my new pseudo-language cac (/ˈkeɪk/)

## state

- [x] simple expressions
- [x] simple statements (e.g. print, scan, etc.)
- [x] blocks
- [x] control flow (if, while, for, goto)
- [x] functions
- [x] lambdas
- [x] resolver
- [x] classes
- [x] inheritence
- [x] finish the book (that is the second charapter)
- [x] usage of multiple files
- [x] filesystem basic I/O
- [ ] at least some optimazations (dont even wait lol)

## language
### WARNING: the language is disgustingly slow (because it uses tree-walk interpreter)
### variables
Variables are created using keyword ``var``, since cac is a dynamicly typed language, type specification is not needed. Variables could also be dynamicly cast.
```javascript
var a = 2;
zaprintit str(a) + "\n"; // output: 2
a = "string\n";
zaprintit a; // output: string
```
cac also allows shadowing, so this is valid:
```javascript
var a = 2;
{
    var a = "shadowed\n";
    zaprintit a; // output: shadowed
}
```
### control flow
Cac has if, while, and for loops respectively. They work mostly like in other C-style languages, except like in Go, parentheses aren't necessery (though allowed)
```javascript
var bool = true;
if bool
    zaprintit "if statement\n";
bool = (2 <= 3 && 5 >= 2);
var i = 0;
while bool {
    i = i + 1;
    if i > 20 break;
}

for ;i < 100; i = i + 1 {
    zaprintit str(i) + "\n";
}
```
there are also labels and goto statements, just 'cause why not
```javascript
var a = 0;
label a:
  a = a + 1;
  goto b;
label b:
  zaprintit str(a) + "\n";
  if a < 20
    goto a;

```
### types
Cac's literals have 7 types; ``null``, boolean (``true`` or ``false``), int, float (i.e. double), function, class, instance (object if you will).
First four are clear, functions are created using ``func`` keyword and they are variables as well. Anonymous functions (lambdas) are created the same way, but without identifier
```javascript
func a_func(val) {
    var b_func = func () {
        zaprintit str(val) + "\n";
    };
    return b_func;
}
a_func(2)(); // output: 2
```
Classes use keyword ``class``, to inherit one use `` : `` and the base name. Constructor is method with the same name as the name of the class. To get current object like in many other languages you can use ``this``. In, say, Java, to execute a method as a superclass instance you use ``super``, but for me at least, it isn't super enough, so in cac you use ``superduper``.
```javascript
class A {
    func a_method() {zaprintit "A instance, a_method\n";}
};
class B : A {
    func B(val) {this.field = val;}
    func a_method() {zaprintit "B instance, a_method, field 'field': " + str(this.field) + "\n";}
    func b_method() {superduper.a_method();}
};
var a = B(23);
a.a_method(); // output: B instance, a_method, field 'field': 23
a.b_method(); // output: A instance, a_method
```
### basic I/O
Like any other language, cac has functions for printing and scanning user input. The first one as you may have already seen is ``zaprintit``. Why ``zaprintit``? It's just a funny english spelling of word 'print' in russian. The same way I called scanning ``zascanit``.
```javascript
var input;
zaprintit "Type stuff: ";
zascanit input; // input: good morning
zaprintit input + "\n"; // output: good morning
```
for filesystem I/O you can use class ``FILE``, which has constructor that takes a path to the file, method ``read`` which takes two arguement, where to start reading from, and how many characters to read, and method ``write``, which takes where to start writing from, and what to write.
```javascript
// file: test.txt:
// -----------------
// Hello World!
//
var file = FILE("test.txt");
var text = file.read(0,0); // if both arguements are zero, it reads the whole file, just for convenience
zaprintit text; // output: Hello World!

file.write(len(text), " Hello from cac!");
var text = file.read(0,0);
zaprintit text; // output: Hello World! Hello from cac!
```
### misc
- include "*path*": include another cac file
- str(value) -> string: cast to string
- int(value) -> int: cast to integer
- time() -> int: current unix time
- len(value) -> int: length of a string
