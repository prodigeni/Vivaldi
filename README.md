# Vivaldi #

Vivaldi is a simple interpreted language inspired by Ruby, Python, Lisp, etc.

## Overview ##

### Basics ###

Vivaldi can be run either from a file or from the REPL:

<pre><code>
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
</code></pre>

Vivaldi expressions are separated by newlines or semicolons.
Comments in Vivaldi are C-style `// till end of line` comments--- multiline
comments aren't supported yet.

### Builtins ###

Vivaldi has a fairly limited set of builtin types:

#### Nil ####
`nil` - like `nil` in Ruby and Lisp, or `None` in Python.

#### Bools ####
`true` and `false`. What'd you expect?

#### Floats ####
64-bit floating-point values.

#### Integers ####
32-bit signed integers. Integer literals can be written in decimal, hexadecimal,
octal, or binary:

<pre><code>
    let eighteen = 18
    eighteen == 0x12
    eighteen == 022
    eighteen == 0b10010
</code></pre>

#### Strings ####
Simple, mutable string class.

#### Symbols ####
`'symbol_name` - as in Ruby or Lisp.

#### Arrays ####
Simple mutable array type:

<pre><code>
    let array = [1, 2, 3]
    let three = array.size()
    array.append('foo)
    let four = array.size()
    let two = array.at(1)
</code></pre>

#### Ranges ####
Provides a range over any pair of objects that can be
* Compared with `==`
* Incremented by adding 1

A range covers [start, end):

<pre><code>
    >>> for i in 1 to 5: puts(i) // <x> to <y> is sugar for new Range(x, y)
    1
    2
    3
    4
    => nil
</code></pre>

#### Functions ####
Functions! Syntactically, a function is very simple:

<pre><code>
    fn <name>(<arguments>): <body>
</code></pre>

A function body is any valid Vivaldi expression (which is to say any valid
Vivaldi code):

<pre><code>
    fn is_three(x): x == 3
</code></pre>

A lambda is identical to a function, just without the name--- or, more
accurately, a function is just a lambda with a name. The following:

<pre><code>
    let five_returner = fn(x): 5
</code></pre>

is completely identical to

<pre><code>
    fn five_returner(): 5
</code></pre>

Once defined, functions work more or less like in Python:

<pre><code>
    fn id(x): x

    let function = id
    function(1) // 1
</code></pre>

#### Types ####
Everything in Vivaldi is an object, and has

* Members:

<pre><code>
    // a is an object
    a.foo = 5
    let five = a.foo
</code></pre>

* Methods (actually just members that happen to be functions):

<pre><code>
    // a is an object
    a.foo = 5
    a.bar = fn(): self.foo
    let five = a.bar()
</code></pre>

* A type

<pre><code>
    let int_type = int.type()
    int_type == Integer
</code></pre>

Defining custom types is simple:

<pre><code>
    class MyType
      fn init(x): self.x = x
      fn x_is_equal_to(y): self.x = y
    end

    let my_obj = new MyType(5)
    let yes = my_obj.x_is_equal_to(5)
    let no = my_obj.x_is_equal_to(47)
</code></pre>

#### Iterators and Ranges ####

Iterators are the basic way to, well, iterate over something. A basic iterator
type supports three methods:

* `get()` --- returns the item currently pointed to
* `increment` --- moves the iterator to the next item in its range, and returns
  itself.
* `at_end()` --- returns whether or not the iterator is at the end of its range.
  If it is, the iterator doesn't point to anything valid--- conceptually, it
  works like this:

<pre><code>
    { item 0, item 2 ... item n - 1, item n }
      ^                                     ^
      start                                 end
</code></pre>

A range is even simpler; it only needs to support one method, `start()`, that
returns an iterator pointing to its first element. In the standard library, both
`Array` and `String` are ranges, and `ArrayIterator` and `String` are their
corresponding iterators. `Range` is both a range (natch!) and an iterator---
calling `start()` on a range just returns itself.

Iterators are used to implement for loops:

<pre><code>
    for i in range: puts(i)
</code></pre>

is equivalent to something like:

<pre><code>
    let <implicit_var> = range.start()
    while !<implicit_var>.at_end(): do
      let i = <implicit_var>.get()
      puts(i)
      <implicit_var>.increment()
    end
</code></pre>

### Structures ###

Vivaldi expressions comprise:

#### Literals ####
`5`, `5.0`, `true`, `nil`, `"string"`, `'symbol`, `['array]`, and so forth.

#### Variables, Declarations and Assignments ####

All variables must be declared before use:

<pre><code>
    i = 5     // wrong --- i hasn't been declared yet
    let i = 5 // right
    i = 3     // right --- i has been declared, so assigning to it is OK
</code></pre>

#### Control Flow ####

Vivaldi's basic control flow structures are cond statements, while loops, and
for loops:

<pre><code>
    let i = cond false: "not me!",
                 true:  "me!"
    while true: puts("looping endlessly...")
</code></pre>

Cond statements work as in Lisp: the first member of each pair is evaluated
until one is truthy (i.e. not `false` or `nil`). When the truthy member is
found, the second half of that pair is evaluated and returned. If no truthy
tests are found, the statement returns `nil`.

The keyword `if` is provided as a synonym to `cond`, since it reads more
naturally for one-body cond statements:

<pre><code>
    // i is a variable
    let a = if i: "i is truthy"
    if a == nil: puts("i isn't truthy")

    // equivalent:
    let a = cond i: "i is truthy"
    cond a == nil: puts("i isn't truthy")
</code></pre>

while loops are very straightforward:

<pre><code>
    while <condition>: <expression>
</code></pre>

For loops are familiar to anyone who's ever used Python or Ruby:

<pre><code>
    for <i> in <range>: <expression>
</code></pre>

For more on for loops, see iterators.

Note that while and loops always evaluate to `nil`.

#### Blocks ####

Functions, cond/if statements, and while loops are all limited to a single
expression for their bodies. This kind of sucks if you want to do anything
actually interesting with them. Fortunately, it's possible, using blocks, to
mash a bunch of expressions together in a sequence:

<pre><code>
    let i = do
      puts("I'm in a block!")
      5; 4; 3; 2; 1
    end
    i == 1
</code></pre>

Using this, it's possible to build actually useful constructs:

<pre><code>
    fn filter(array, predicate): do
      let filtered = []
      for i in array: cond
        pred(i): filtered.append(i)
      filtered
    end
</code></pre>

Blocks have nested scope:

<pre><code>
    do let j = 5 end
    puts(j) // wrong --- j is out of scope

    let j = 5
    do puts(j) end // fine
</code></pre>

#### Exceptions ####

Like in C++, exceptions in Vivaldi don't have any special type (in fact there is
no builtin exception type; strings are used instead). Otherwise they work pretty
much as you'd expect:

<pre><code>
    let i = try: except 5
    catch e: e + 1
    i == 6
</code></pre>

As everywhere else in Vivaldi, the pieces of code following `try` and `catch`
are expressions.

### Example ###

#### FizzBuzz ####

<pre><code>
    for i in 1 to 100: cond
      i % 15 == 0: puts("FizzBuzz"),
      i % 5 == 0:  puts("Buzz"),
      i % 3 == 0:  puts("Fizz"),
      true:        puts(i)
</code></pre>

See examples folder for more.

### TODO ###

* Expand the standard library

* Add C API

* Improve performance, especially concerning the call stack
