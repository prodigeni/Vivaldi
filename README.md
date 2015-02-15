# Vivaldi #

Vivaldi is a simple interpreted language inspired by Ruby, Python, Lisp, etc,
supporting duck-typing, object orientation, and some functional constructs.

## Overview ##

### Basics ###

Vivaldi can be run either from a file or from the REPL:

    $ cat test.vv
    puts("Hello, world!")
    $ vivaldi ./test.vv
    Hello, world!
    $ ./vivaldi
    >>> puts("Hello, REPL!")
    Hello, REPL!
    => nil
    >>> quit()
    $

Vivaldi expressions are separated by newlines or semicolons.
Comments in Vivaldi are C-style `// till end of line` comments&mdash; multiline
comments aren't supported yet. For a full description of the grammar in
Backus-Naur form, see grammar.txt.

### Compiling

* If you don't already have it, install boost (`brew install boost` on OS X;
  Linux might be trickier, for reasons explained below)
* If you don't have CMake 3.0 or above, install it (`brew install cmake`,
  `pacman -S cmake`, `sudo apt-get install cmake` for OS X, Arch Linux, and
  Ubuntu respectively)
* Ensure that clang++ and libc++ >= 3.5 are installed.

        $ git clone git@github.com:jeorgun/Vivaldi.git
        $ cd Vivaldi
        $ mkdir build
        $ cd build
        $ cmake .. && make

Vivaldi's been tested on 64-bit OS X 10.10.2, and 32-bit Arch Linux with Linux
3.18, both with Clang/libc++ 3.5 and Boost 1.57.0. libc++ is required, and,
unfortunately, since Boost binaries are used, so is a Boost compiled with
libc++. The codebase is (or should be!) is fully conforming C++14--- it's quite
easy to add support for libstdc++, since it would basically consist of ripping
out a bunch of C++14 features, but I'm not really inclined to do that unless
there's a particularly pressing need.

### Builtins ###

Vivaldi has a fairly limited set of builtin types:

#### Objects ####

The root of the inheritance tree; every type in Vivaldi inherits from Object
(which in turn inherits from... itself; it really is turtles all the way down!)
Objects support a few universal methods:

* `type()` &mdash; Returns the type of `self`.
* `not()` &mdash;  Returns whether or not `self` is truthy (i.e. not `false` or
  `nil`).
* `equals(x)`, `unequal(x)` &mdash; Returns true if `self` has the same object
ID as `x`, and `false` otherwise (vice versa for `unequal`). These methods
should be overridden by any classes with value-based concepts of semantics (for
instance, in the standard library, `String`, `Symbol`, `Integer`, and so forth
all override them).

#### Nil ####
`nil` &mdash; like `nil` in Ruby and Lisp, or `None` in Python.

#### Bools ####
`true` and `false`. What'd you expect?

#### Floats ####
64-bit floating-point values.

#### Integers ####
32-bit signed integers. Integer literals can be written in decimal, hexadecimal,
octal, or binary:

    let eighteen = 18
    eighteen == 0x12
    eighteen == 022
    eighteen == 0b10010

#### Strings ####
Simple, mutable string class. Currently supports:

* `init(x)`&mdash; if `x` is a String or a Symbol, copies its string value;
  otherwise, creates a String with `x`'s display value. For instance, `new
  String(12)` will return `"12"`.
* `size()`&mdash; returns the size of the string.
* `append(x)`&mdash; Appends the String `x` to `self`.
* `add(x)`&mdash; Returns a string formed form concatenating `self` and the String
  `x`, leaving `self` unchanged.
* `times(x)`&mdash; Returns a string formed by concatenating `x` copies of `self`,
  leaving `self` unchanged.
* `start()`&mdash; Returns an iterator pointing to the beginning of `self`.
* `end()`&mdash; Returns an iterator pointing to the end of `self`.
* `equals(x)`, `unequal(x)`&mdash; Returns `true` if `x` is a String equal in value
  to `self`, and `false` otherwise (vice versa for `unequal`).

#### Symbols ####
`'symbol_name` - as in Ruby or Lisp:

* `init(x)`&mdash; Creates a new Symbol with the string value of `x`, where `x` is a
  String or a Symbol.
* `equals(x)`, `unequal(x)`&mdash; Returns `true` if `x` is a Symbol equal in value
  to `self`, and `false` otherwise (vice versa for `unequal`).

#### Arrays ####
Simple mutable array type:

    let array = [1, 2, 3]
    let three = array.size()
    array.append('foo)
    let four = array.size()
    let two = array[1]
    array[2] = "foo"
    array // [1, 2, "foo"]

* `init(x)`&mdash; Returns a copy of the Array `x`.
* `size()`&mdash; Returns the size of `self`.
* `append(x)`&mdash; Pushes the value `x` onto the end of `self`.
* `at(x)`&mdash; Returns the value at index `x`.
* `set_at(x, y)`&mdash; Sets the value at index `x` to `y`, returning `y`.
* `start()`&mdash; Returns an iterator pointing to the beginning of `self`.
* `end()`&mdash; Returns an iterator pointing to the end of `self`.
* `add(x)`&mdash; Returns the concatenation of `self` and Array `x`, leaving `self`
  unchanged.

#### Dictionaries ####
Mutable hash-map type. At the moment, there's no way to override a type's
equality or hash methods, so you're stuck with whatever you're inheriting
from&mdash; which is based on object ID if you derive from Object:

    let dict = { 'foo: 5, "bar": 6 }
    let two = dict.size()
    dict[0.5] = 'baz
    let three = dict.size()
    let five = dict['foo]
    dict // { 0.500000: 'baz, 'foo: 5, "bar": 6 }

* `init(x)`&mdash; Returns a copy of the Dictionary `x`.
* `size()`&mdash; Returns the size of `self`.
* `at(x)`&mdash; Returns the value at key `x`; if no such value exists, inserts (and
  returns) `nil` at `x`.
* `set_at(x, y)`&mdash; Sets the value at key `x` to `y`, returning `y`.

#### Ranges ####
Provides a range over any pair of objects that can be

* Compared with `>` (`greater`)
* Incremented by adding 1 (via `add`)

A range covers [start, end):

    >>> for i in 1 to 5: puts(i) // "<x> to <y>" is sugar for "new Range(x, y)"
    1
    2
    3
    4
    => nil

* `init(x, y)`&mdash; Returns a Range from `x` to `y`. If they're not comparable or
  incrementable, this won't blow up *immediately*&mdash; only when you first try to
  use it.
* `start()`&mdash; Just returns `self`; see the section on Iterators to understand
  why.
* `size()`&mdash; Returns `y - x`. Don't call this if subtraction won't work!
* `at_end()`&mdash; Returns if `x == y` (well, actually, if `!(y > x)`, so a Range
  from `1.3` to `5.0` doesn't go on infinitely).
* `increment`&mdash; Add 1 to `x`
* `to_arr`&mdash; Creates an Array from all values from `x` to `y`.

#### Functions ####
Functions! Syntactically, a function is very simple:

    fn <name>(<arguments>): <body>

A function body is any valid Vivaldi expression (which is to say any valid
Vivaldi code):

    fn is_three(x): x == 3

A lambda is identical to a function, just without the name&mdash; or, more
accurately, a function is just a lambda with a name. The following:

    let five_returner = fn(): 5

is completely identical to

    fn five_returner(): 5

Once defined, functions work more or less like in Python:

    fn id(x): x

    let function = id
    function(1) // 1

In functions and methods, `self` works a little differently than in Python or
Ruby. It's not explicitly passed as an argument, but rather is implicitly passed
whenever called as a member of an object. The following is perfectly valid:

    fn not_method(): return self.a
    not_method() // throws exception

    let myobj = new Object()
    myobj.a = 5
    myobj.yes_method = not_method
    myobj.yes_method() // returns 5

`return` can be used to exit out of a function early, but it's unnecessary for
the last expression of a function body (or *only* expression, unless it's
wrapped in a block):

    // contrived; this could just be 'fn is_even(x): x % 2 == 0'
    fn is_even(x): do
      if x % 2 == 0: return true
      false
    end

#### Types ####
Everything in Vivaldi is an object, and has

* Members:

        // a is an object
        a.foo = 5
        let five = a.foo

* Methods (actually just members that happen to be functions):

        // a is an object
        a.foo = 5
        a.bar = fn(): self.foo
        let five = a.bar()

* A type:

        let int_type = int.type()
        int_type == Integer

Defining custom types is simple:

    class MyType
      fn init(x): self.x = x
      fn x_is_equal_to(y): self.x = y
    end

    let my_obj = new MyType(5)
    let yes = my_obj.x_is_equal_to(5)
    let no = my_obj.x_is_equal_to(47)

* `parent()`&mdash; Returns the parent type of `self`.

#### Iterators and Ranges ####

Iterators are the basic way to, well, iterate over something. A basic iterator
type supports three methods:

* `get()` &mdash; returns the item currently pointed to
* `increment` &mdash; moves the iterator to the next item in its range, and returns
  itself.
* `at_end()` &mdash; returns whether or not the iterator is at the end of its range.
  If it is, the iterator doesn't point to anything valid&mdash; conceptually, it
  works like this:

        { item 0, item 2 ... item n - 1, item n }
          ^                                     ^
          start                                 end

A range is even simpler; it only needs to support one method, `start()`, that
returns an iterator pointing to its first element. In the standard library, both
`Array` and `String` are ranges, and `ArrayIterator` and `String` are their
corresponding iterators. `Range` is both a range (natch!) and an iterator&mdash;
calling `start()` on a range just returns itself.

Iterators are used to implement for loops:

    for i in range: puts(i)

is equivalent to something like:

    let <implicit_var> = range.start()
    while !<implicit_var>.at_end(): do
      let i = <implicit_var>.get()
      puts(i)
      <implicit_var>.increment()
    end

### Structures ###

Vivaldi expressions comprise:

#### Literals ####
`5`, `5.0`, `true`, `nil`, `"string"`, `'symbol`, `['array]`, and so forth.

#### Variables, Declarations and Assignments ####

All variables must be declared before use:

    i = 5     // wrong &mdash; i hasn't been declared yet
    let i = 5 // right
    i = 3     // right &mdash; i has been declared, so assigning to it is OK

#### Control Flow ####

Vivaldi's basic control flow structures are cond statements, while loops, and
for loops:

    let i = cond false: "not me!",
                 true:  "me!"
    while true: puts("looping endlessly...")

Cond statements work as in Lisp: the first member of each pair is evaluated
until one is truthy (i.e. not `false` or `nil`). When the truthy member is
found, the second half of that pair is evaluated and returned. If no truthy
tests are found, the statement returns `nil`.

The keyword `if` is provided as a synonym to `cond`, since it reads more
naturally for one-body cond statements:

    // i is a variable
    let a = if i: "i is truthy"
    if a == nil: puts("i isn't truthy")

    // equivalent:
    let a = cond i: "i is truthy"
    cond a == nil: puts("i isn't truthy")

while loops are very straightforward:

    while <condition>: <expression>

For loops are familiar to anyone who's ever used Python or Ruby:

    for <i> in <range>: <expression>

For more on for loops, see iterators.

Note that while and loops always evaluate to `nil`.

#### Blocks ####

Functions, cond/if statements, and while loops are all limited to a single
expression for their bodies. This kind of sucks if you want to do anything
actually interesting with them. Fortunately, it's possible, using blocks, to
mash a bunch of expressions together in a sequence, and return the last computed
result:

    let i = do
      puts("I'm in a block!")
      5; 4; 3; 2; 1
    end
    i == 1

Using this, it's possible to build actually useful constructs:

    fn filter(array, predicate): do
      let filtered = []
      for i in array: if pred(i): filtered.append(i)
      filtered
    end

Blocks have nested scope:

    do let j = 5 end
    puts(j) // wrong &mdash; j is out of scope

    let j = 5
    do puts(j) end // fine

#### Exceptions ####

Like in C++, exceptions in Vivaldi don't have any special type (in fact at the
moment there is no builtin exception type; strings are used instead). Otherwise
they work pretty much as you'd expect:

    let i = try: except 5
    catch e: e + 1
    i == 6

As everywhere else in Vivaldi, the pieces of code following `try` and `catch`
are expressions.

#### Operators ####

Vivaldi operators, aside from `&&`, `||`, `to` (which is syntax sugar for
Range), and `=` (which isn't actually an operator at all&mdash; it just looks like
one), are all just syntax sugar for method calls. Here they are in order of
precedence (basically C precedence, with the bitwise mistake fixed and `**` and
`to` inserted where appropriate):

    a[b]     // a.at(b)
    a[b] = c // a.set_at(b, c)
    !a       // a.not()
    -a       // a.negative()
    ~a       // a.negate()
    a ** b   // a.pow(b)
    a * b    // a.times(b)
    a / b    // a.divides(b)
    a % b    // a.modulo(b)
    a + b    // a.add(b)
    a - b    // a.subtract(b)
    a << b   // a.rshift(b)
    a >> b   // a.lshift(b)
    a & b    // a.bitand(b)
    a ^ b    // a.xor(b)
    a | b    // a.bitor(b)
    a to b   // Range(a, b)
    a < b    // a.less(b)
    a > b    // a.greater(b)
    a <= b   // a.less_equals(b)
    a >= b   // a.greater_equals(b)
    a == b   // a.equals(b)
    a != b   // a.unequal(b)

### Builtins ###

In addition to the above types, Vivaldi has a select few builtin functions:

* `puts(x)`&mdash; as in Ruby, write the passed value plus a newline. Takes only one
  argument.

* `print(x)`&mdash; identical to `puts`, sans newline.

* `gets()`&mdash; returns a String containing a single line of user input.

* `quit()`&mdash; exits the program unconditionally.

More to be added eventually.

### Example ###

#### FizzBuzz ####

    for i in 1 to 100: cond
      i % 15 == 0: puts("FizzBuzz"),
      i % 5 == 0:  puts("Buzz"),
      i % 3 == 0:  puts("Fizz"),
      true:        puts(i)

See examples folder for more.

### TODO ###

* Expand the standard library

* Add C API

* Improve performance, especially concerning the call stack
